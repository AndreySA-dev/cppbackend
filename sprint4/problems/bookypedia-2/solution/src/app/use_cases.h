#pragma once

#include "../domain/domain_fwd.h"
#include "unit_of_work.h"
#include <optional>
#include <string>
#include <vector>

namespace app {

class UseCases {
  public:
	virtual domain::Author AddAuthor(const std::string& name) = 0;
	virtual std::vector<domain::Author> GetAuthors() = 0;
	virtual std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) = 0;
	virtual std::optional<domain::Author> GetAuthorByName(const std::string& author_name) = 0;
	virtual void DeleteAuthor(const std::string& id) = 0;
	virtual void EditAuthor(const std::string& id, const std::string& new_name) = 0;
	virtual void DeleteBook(const std::string& book_id) = 0;

	virtual void AddBook(
		const std::string& author_id, const std::string& title, int year, std::vector<std::string> tags) = 0;
	virtual std::vector<domain::repo::BookResponse> GetBooks(
		const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) = 0;
	// [[nodiscard]] virtual std::vector<domain::Book> GetBookByTitle(const std::string& title) = 0;
	[[nodiscard]] virtual std::vector<domain::repo::BookResponseExtra> GetBooksExtraInfo(
		const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) = 0;


	virtual void Start() = 0;
	virtual void End() = 0;
	virtual void Interrupt() = 0;

	virtual ~UseCases() = default;

  protected:
	// virtual UnitOfWork GetUoW() && = 0;
};

class UseCasesGuard {
  public:
	UseCasesGuard(UseCases& use_cases);
	~UseCasesGuard();

	UseCasesGuard(const UseCasesGuard&) = delete;
	UseCasesGuard& operator=(const UseCasesGuard&) = delete;
	UseCasesGuard(UseCasesGuard&&) = delete;
	UseCasesGuard& operator=(UseCasesGuard&&) = delete;

  private:
	UseCases& use_cases_;
};

} // namespace app
