#pragma once
#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/repository.h"
#include "../domain/domain_fwd.h"

#include <memory>
#include <optional>
#include <vector>

namespace app {

class UnitOfWork {
  public:
	virtual void AddAuthor(domain::Author author) = 0;
	virtual std::vector<domain::Author> GetAuthors() = 0;
	virtual std::vector<domain::Book> GetAuthorBooks(const domain::AuthorId author_id) = 0;
	virtual std::optional<domain::Author> GetAuthorByName(const std::string& author_name) = 0;

	virtual void AddBook(domain::Book book) = 0;
	virtual std::vector<domain::repo::BookResponse> GetBooks() = 0;

	virtual void Commit() = 0;

	virtual ~UnitOfWork() = default;

  protected:
};

class UnitOfWorkFactory {
  public:
	virtual std::unique_ptr<UnitOfWork> GetUoW() = 0;

	virtual ~UnitOfWorkFactory() = default;

  protected:
};


} // namespace app