#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../app/unit_of_work.h"
#include "../domain/author.h"
#include "../domain/book.h"


namespace postgres {
// DATABASE SECTION ===========================================================
class Database {
  public:
	explicit Database(pqxx::connection connection);

	pqxx::connection& GetConection();
	// AuthorRepositoryImpl& GetAuthors() & {
	// 	return authors_;
	// }
	// BookRepositoryImpl& GetBooks() & {
	// 	return books_;
	// }

  private:
	pqxx::connection connection_;
	// AuthorRepositoryImpl authors_{connection_};
	// BookRepositoryImpl books_{connection_};
};

// AUTHOR SECTION ===========================================================
class AuthorRepositoryImpl : public domain::AuthorRepository {
  public:
	explicit AuthorRepositoryImpl(pqxx::work& work);

	void Save(const domain::Author& author) override;
	std::vector<domain::Author> Load() override;
	std::optional<domain::Author> GetAuthorByName(const std::string& name) override;

  private:
	pqxx::work& work_;
};

// BOOK SECTION ===========================================================
class BookRepositoryImpl : public domain::BookRepository {
  public:
	explicit BookRepositoryImpl(pqxx::work& work);

	void Save(const domain::Book& book) override;
	std::vector<domain::Book> Load() override;
	std::vector<domain::Book> GetAuthorBooks(const domain::AuthorId& author_id) override;

  private:
	pqxx::work& work_;
};


// Repository SECTION ===========================================================
class RepositoryImpl : public domain::repo::Repository {
  public:
	explicit RepositoryImpl(pqxx::work& work);
	std::vector<domain::repo::BookResponse> GetBooks() override;

  private:
	pqxx::work& work_;
};


// UoW SECTION ===========================================================
class UnitOfWorkImpl : public app::UnitOfWork {
  public:
	UnitOfWorkImpl(pqxx::connection& connection);

	void AddAuthor(domain::Author author) override;
	std::vector<domain::Author> GetAuthors() override;
	std::vector<domain::Book> GetAuthorBooks(const domain::AuthorId author_id) override;
	std::optional<domain::Author> GetAuthorByName(const std::string& author_name) override;

	void AddBook(domain::Book book) override;
	std::vector<domain::repo::BookResponse> GetBooks() override;

	void Commit() override;

  private:
	pqxx::work work_;
	AuthorRepositoryImpl authors_repo_;
	BookRepositoryImpl books_repo_;
	RepositoryImpl repo_;	
};


class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
  public:
	UnitOfWorkFactoryImpl(pqxx::connection& connection);
	std::unique_ptr<app::UnitOfWork> GetUoW() override;

	
  private:
	pqxx::connection& connection_;
};


} // namespace postgres