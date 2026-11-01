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

	virtual void AddBook(const std::string& author_id, const std::string& title, int year,  std::vector<std::string> tags) = 0;
	virtual std::vector<domain::repo::BookResponse> GetBooks() = 0;

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
