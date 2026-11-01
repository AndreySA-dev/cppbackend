#include "postgres.h"

#include <pqxx/pqxx>
#include <pqxx/zview.hxx>

#include <string_view>

namespace postgres {

using namespace std;
using namespace std::literals;
using pqxx::operator"" _zv;


Database::Database(pqxx::connection connection) : connection_{std::move(connection)} {
	pqxx::work work{connection_};
	work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);

	work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT book_key PRIMARY KEY,
    author_id uuid NOT NULL,
	title varchar(100) NOT NULL,
	publication_year integer  NOT NULL
);
)"_zv);

	work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
	book_id UUID NOT NULL,
	tag varchar(30) NOT NULL
);
)"_zv);

	// коммитим изменения
	work.commit();
}

pqxx::connection& Database::GetConection() {
	return connection_;
}


// AUTHOR SECTION =====================================================================================
AuthorRepositoryImpl::AuthorRepositoryImpl(pqxx::work& work) : work_{work} {}

void AuthorRepositoryImpl::Save(const domain::Author& author) {
	// pqxx::work work{connection_};
	work_.exec_params(
		R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
		author.GetId().ToString(), author.GetName());
	// work.commit();
}

std::vector<domain::Author> AuthorRepositoryImpl::Load() {
	std::vector<domain::Author> result;
	for (auto [id, name] : work_.query<string_view, string_view>("SELECT id, name FROM authors ORDER BY name"_zv)) {
		result.emplace_back(domain::AuthorId::FromString(std::string(id)), std::string(name));
	}

	return result;
}

std::optional<domain::Author> AuthorRepositoryImpl::GetAuthorByName(const std::string& name) {
	try {
		auto [id, author_name] = work_.query1<std::string, std::string>(
			"SELECT id, name FROM authors WHERE name = '" + work_.esc(name) + "'");
		return domain::Author{domain::AuthorId::FromString(id), author_name};
	} catch (const pqxx::unexpected_rows&) {
		//
	}
	return std::nullopt;
}


// AUTHOR SECTION =====================================================================================
BookRepositoryImpl::BookRepositoryImpl(pqxx::work& work) : work_{work} {}

void BookRepositoryImpl::Save(const domain::Book& book) {

	work_.exec_params(
		R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
)"_zv,
		book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetYear());

	for (auto& tag : book.GetTags()) {
		work_.exec_params(
			R"(
INSERT INTO book_tags (book_id, tag) VALUES ($1, $2)
)"_zv,
			book.GetId().ToString(), tag);
	}
	// work.commit();
}

std::vector<domain::Book> BookRepositoryImpl::Load() {
	std::vector<domain::Book> result;
	for (auto [id, author_id, title, year] : work_.query<string_view, string_view, string_view, int>(
			 "SELECT id, author_id, title, publication_year FROM books ORDER BY title"_zv)) {
		result.emplace_back(domain::BookId::FromString(std::string(id)),
			domain::AuthorId::FromString(std::string(author_id)), std::string(title), year, std::vector<std::string>{});
	}

	return result;
}

std::vector<domain::Book> BookRepositoryImpl::GetAuthorBooks(const domain::AuthorId& author_id) {
	std::vector<domain::Book> result;
	auto s = author_id.ToString();
	for (auto [id, author_id, title, year] : work_.query<string_view, string_view, string_view, int>(
			 "SELECT id, author_id, title, publication_year FROM books WHERE author_id = '" +
			 work_.esc(author_id.ToString()) + "' ORDER BY publication_year, title;")) {
		result.emplace_back(domain::BookId::FromString(std::string(id)),
			domain::AuthorId::FromString(std::string(author_id)), std::string(title), year, std::vector<std::string>{});
	}

	return result;
}

// UoW SECTION ==========================================================================
UnitOfWorkImpl::UnitOfWorkImpl(pqxx::connection& connection)
	: work_(connection), authors_repo_(work_), books_repo_(work_), repo_(work_) {}

void UnitOfWorkImpl::AddAuthor(domain::Author author) {
	authors_repo_.Save(std::move(author));
}

std::vector<domain::Author> UnitOfWorkImpl::GetAuthors() {
	return authors_repo_.Load();
}


void UnitOfWorkImpl::AddBook(domain::Book book) {
	books_repo_.Save(std::move(book));
}

std::vector<domain::repo::BookResponse> UnitOfWorkImpl::GetBooks() {
	return repo_.GetBooks();
}

std::vector<domain::Book> UnitOfWorkImpl::GetAuthorBooks(const domain::AuthorId author_id) {
	return books_repo_.GetAuthorBooks(author_id);
}

std::optional<domain::Author> UnitOfWorkImpl::GetAuthorByName(const std::string& name) {
	return authors_repo_.GetAuthorByName(name);
}

void UnitOfWorkImpl::Commit() {
	work_.commit();
}


UnitOfWorkFactoryImpl::UnitOfWorkFactoryImpl(pqxx::connection& connection) : connection_(connection) {}

std::unique_ptr<app::UnitOfWork> UnitOfWorkFactoryImpl::GetUoW() {
	return std::make_unique<UnitOfWorkImpl>(connection_);
}

RepositoryImpl::RepositoryImpl(pqxx::work& work) : work_{work} {}

std::vector<domain::repo::BookResponse> RepositoryImpl::GetBooks() {
	std::vector<domain::repo::BookResponse> result;
	for (auto [title, year, author_name] : work_.query<string_view, int, string_view>(
			 "SELECT B.title, B.publication_year, A.name FROM books as B "
			 "LEFT JOIN authors as A ON B.author_id = A.id ORDER BY B.title, A.name, B.year"_zv)) {
		result.emplace_back(std::string(title), std::string(author_name), year);
	}

	return result;
}

} // namespace postgres