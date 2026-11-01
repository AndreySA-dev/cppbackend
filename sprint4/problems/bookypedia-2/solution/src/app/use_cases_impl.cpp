#include "use_cases_impl.h"
#include "../domain/author.h"
#include "../domain/book.h"

#include <stdexcept>

namespace app {
using namespace domain;

UseCasesImpl::UseCasesImpl(std::unique_ptr<UnitOfWorkFactory> uow_factory) : uow_factory_(std::move(uow_factory)) {}

void UseCasesImpl::AddAuthor(const std::string& name) {
	if (name.empty()) {
		throw std::invalid_argument("Error. Author namr can't be empty.");
	}
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	// authors_.Save({AuthorId::New(), name});

	uow_->AddAuthor({AuthorId::New(), name});
}

std::vector<Author> UseCasesImpl::GetAuthors() {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	// return authors_.Load();
	return uow_->GetAuthors();
}


void UseCasesImpl::AddBook(const std::string& author_id, const std::string& title, int year) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	// books_.Save({BookId::New(), AuthorId::FromString(author_id), title, year});
	uow_->AddBook({BookId::New(), AuthorId::FromString(author_id), title, year});
}

std::vector<Book> UseCasesImpl::GetBooks() {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	// return books_.Load();
	return uow_->GetBooks();
}

std::vector<domain::Book> UseCasesImpl::GetAuthorBooks(const std::string& author_id) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	// return books_.GetAuthorBooks(domain::AuthorId::FromString(author_id));
	return uow_->GetAuthorBooks(domain::AuthorId::FromString(author_id));
}

void UseCasesImpl::Start() {
	if (uow_) {
		throw std::logic_error("Error. Work already started.");
	}
	uow_ = uow_factory_->GetUoW();
}

void UseCasesImpl::Stop() {
	if (!uow_) {
		throw std::logic_error("Error. Work not started.");
	}
	uow_->Commit();
	uow_.reset();
}

} // namespace app
