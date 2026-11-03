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

	domain::Author AddAuthor(const std::string& name) override;
	std::vector<domain::Author> GetAuthors() override;
	std::vector<domain::Book> GetAuthorBooks(const std::string& author_id) override;
	std::optional<domain::Author> GetAuthorByName(const std::string& author_name) override;
	void DeleteAuthor(const std::string& id) override;
	void EditAuthor(const std::string& id, const std::string& new_name) override;
	void DeleteBook(const std::string& book_id) override;

	void AddBook(const std::string& author_id, const std::string& title, int year, std::vector<std::string> tags) override;
	std::vector<domain::repo::BookResponse> GetBooks(const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) override;
	// std::vector<domain::Book> GetBookByTitle(const std::string& title) override;
	std::vector<domain::repo::BookResponseExtra> GetBooksExtraInfo(const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) override;


	void Start() override;
	void End() override;
	void Interrupt() override;
	
  private:
	std::unique_ptr<UnitOfWorkFactory> uow_factory_ = nullptr;
	std::unique_ptr<UnitOfWork> uow_ = nullptr;
	// domain::AuthorRepository& authors_;
	// domain::BookRepository& books_;
};




} // namespace app
