#pragma once
#include "../domain/domain_fwd.h"
#include "use_cases.h"
#include <memory>


namespace app {

class UseCasesImpl : public UseCases {
  public:
	// explicit UseCasesImpl(domain::AuthorRepository& authors, domain::BookRepository& books)
	// 	: authors_{authors}, books_(books) {}
	explicit UseCasesImpl(std::unique_ptr<UnitOfWorkFactory> uow_factory);

	void AddAuthor(const std::string& name) override;
	std::vector<domain::Author> GetAuthors() override;

	void AddBook(const std::string& author_id, const std::string& title, int year) override;
	std::vector<domain::Book> GetBooks() override;

	std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) override;

	void Start() override;
	void Stop() override;


  private:
	std::unique_ptr<UnitOfWorkFactory> uow_factory_ = nullptr;
	std::unique_ptr<UnitOfWork> uow_ = nullptr;
	// domain::AuthorRepository& authors_;
	// domain::BookRepository& books_;
};




} // namespace app
