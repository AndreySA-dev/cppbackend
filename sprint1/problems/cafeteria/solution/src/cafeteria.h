#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <memory>


#include "clock.h"
#include "hotdog.h"
#include "result.h"

using namespace std::literals;
namespace net = boost::asio;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

class HotDogOrder : public std::enable_shared_from_this<HotDogOrder> {
	constexpr static Clock::duration SAUSAGE_FRY_DURATION = Milliseconds{1500};
	constexpr static Clock::duration BREAD_FRY_DURATION = Milliseconds{1000};

  public:
	// clang-format off
	HotDogOrder(
		boost::asio::io_context& io, 
		HotDogHandler handler, 
		// int hotdog_id, 
		// std::shared_ptr<Sausage> sausage,
		// std::shared_ptr<Bread> bread, 
		std::shared_ptr<GasCooker> gas_cooker,
		Store& store,
		boost::asio::strand<boost::asio::io_context::executor_type>& id_strand
	)
		: 
		io_(io), 
		handler_(std::move(handler)), 
		// id_(hotdog_id), 
		// sausage_(std::move(sausage)), 
		// bread_(std::move(bread)),
		gas_cooker_(gas_cooker),
		store_(store),
		release_strand_(boost::asio::make_strand(io_)),
		id_strand_(id_strand)
	// clang-format on
	{}

	void MakeHotDog() {

		boost::asio::post(id_strand_, [order = shared_from_this()]() {
			order->sausage_ = order->store_.GetSausage();
			order->bread_ = order->store_.GetBread();
			order->id_ = order->store_.GetNextId();

			order->sausage_->StartFry(*order->gas_cooker_, [order = order->shared_from_this()] {
				order->sausage_timer_.expires_after(SAUSAGE_FRY_DURATION);
				order->sausage_timer_.async_wait(
					[order = order->shared_from_this()](const boost::system::error_code ec) {
						order->sausage_->StopFry();
						order->TryRelease();
					});
			});

			order->bread_->StartBake(*order->gas_cooker_, [order = order->shared_from_this()] {
				order->bread_timer_.expires_after(BREAD_FRY_DURATION);
				order->bread_timer_.async_wait([order = order->shared_from_this()](const boost::system::error_code ec) {
					order->bread_->StopBaking();
					order->TryRelease();
				});
			});
		});
	}

	[[nodiscard]] bool IsReady() const {
		return sausage_->IsCooked() && bread_->IsCooked();
	}

	void TryRelease() {
		boost::asio::post(release_strand_, [order = shared_from_this()]() {
			if (!order->is_released && order->IsReady()) {
				order->is_released = true;
				auto hotdog = HotDog(order->id_, std::move(order->sausage_), std::move(order->bread_));
				order->handler_(std::move(hotdog));
			}
		});
	}


  private:
	boost::asio::io_context& io_;
	HotDogHandler handler_;
	int id_;
	std::shared_ptr<Sausage> sausage_;
	std::shared_ptr<Bread> bread_;
	std::shared_ptr<GasCooker> gas_cooker_;
	Store& store_;

	bool is_released = false;
	boost::asio::strand<boost::asio::io_context::executor_type> release_strand_;
	boost::asio::strand<boost::asio::io_context::executor_type>& id_strand_;
	boost::asio::steady_timer sausage_timer_{io_};
	boost::asio::steady_timer bread_timer_{io_};
};

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
	constexpr static Clock::duration SAUSAGE_FRY_DURATION = Milliseconds{1500};
	constexpr static Clock::duration BREAD_FRY_DURATION = Milliseconds{1000};

  public:
	explicit Cafeteria(net::io_context& io) : io_{io}, id_strand_(boost::asio::make_strand(io_)) {}

	// Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
	// Этот метод может быть вызван из произвольного потока
	void OrderHotDog(HotDogHandler handler) {

		// auto sausage = store_.GetSausage();
		// auto bread = store_.GetBread();
		auto order = std::make_shared<HotDogOrder>(
			io_, std::move(handler), /* hotdog_next_id_++,  sausage, bread, */ gas_cooker_, store_, id_strand_);
		order->MakeHotDog();
	}


	// void OrderHotDog(HotDogHandler handler) {


	// 	auto TryPackHotDog = [sausage, bread, id = ++hotdog_next_id_, handler = handler] {
	// 		// std::cout << "Check IsCooked()" << std::endl;
	// 		if (sausage->IsCooked() && bread->IsCooked()) {
	// 			// std::cout << "HotDog is cooked - " << id << ". Start pack." << std::endl;
	// 			auto hotdog = HotDog(id, std::move(sausage), std::move(bread));
	// 			handler(std::move(hotdog));
	// 		} else {
	// 			// std::cout << "HotDog is not cooked - " << id << "." << std::endl;
	// 		}
	// 	};

	// 	// std::cout << "Start fry " << std::endl;

	// 	sausage->StartFry(*gas_cooker_, [&io = io_, sausage, TryPackHotDog]() {
	// 		// std::cout << "Sausage. Fry start - " << sausage->GetId() << std::endl;
	// 		auto timer = std::make_shared<boost::asio::steady_timer>(io, SAUSAGE_FRY_DURATION);
	// 		timer->async_wait([timer, sausage, TryPackHotDog](const boost::system::error_code& ec) {
	// 			if (ec) {
	// 				throw std::logic_error("Sausage fry error - " + ec.what());
	// 			}
	// 			// std::cout << "Sausage. Stop fry - " << sausage->GetId() << std::endl;
	// 			sausage->StopFry();
	// 			TryPackHotDog();
	// 		});
	// 	});

	// 	bread->StartBake(*gas_cooker_, [&io = io_, bread, TryPackHotDog]() {
	// 		// std::cout << "Bread. Bake start - " << bread->GetId() << std::endl;
	// 		auto timer = std::make_shared<boost::asio::steady_timer>(io, BREAD_FRY_DURATION);
	// 		timer->async_wait([timer = std::move(timer), bread, TryPackHotDog](const boost::system::error_code& ec) {
	// 			if (ec) {
	// 				throw std::logic_error("Bread baking error - " + ec.what());
	// 			}
	// 			// std::cout << "Bread. Fry start - " << bread->GetId() << std::endl;
	// 			bread->StopBaking();
	// 			TryPackHotDog();
	// 		});
	// 	});
	// }

  private:
	net::io_context& io_;
	boost::asio::strand<boost::asio::io_context::executor_type> id_strand_;
	// Используется для создания ингредиентов хот-дога
	Store store_;
	// Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
	// Используйте её для приготовления ингредиентов хот-дога.
	// Плита создаётся с помощью make_shared, так как GasCooker унаследован от
	// enable_shared_from_this.
	std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
	int hotdog_next_id_ = 0;
};
