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
	// auto s = author_id.ToString();
	for (auto [id, author_id, title, year] : work_.query<string_view, string_view, string_view, int>(
			 "SELECT id, author_id, title, publication_year FROM books WHERE author_id = '" +
			 work_.esc(author_id.ToString()) + "' ORDER BY publication_year, title;")) {
		result.emplace_back(domain::BookId::FromString(std::string(id)),
			domain::AuthorId::FromString(std::string(author_id)), std::string(title), year, std::vector<std::string>{});
	}

	return result;
}

// std::vector<domain::Book> BookRepositoryImpl::GetBookByTitle(const std::string& title) {
// 	std::vector<domain::Book> result;
// 	for (auto [id, author_id, title, year] : work_.query<string_view, string_view, string_view, int>(
// 			 "SELECT id, author_id, title, publication_year FROM books WHERE author_id = '" +
// 			 work_.esc(author_id.ToString()) + "' ORDER BY publication_year, title;")) {
// 		result.emplace_back(domain::BookId::FromString(std::string(id)),
// 			domain::AuthorId::FromString(std::string(author_id)), std::string(title), year, std::vector<std::string>{});
// 	}

// 	return result;
// }


// REPO SECTION ==========================================================================
RepositoryImpl::RepositoryImpl(pqxx::work& work) : work_{work} {}

std::vector<domain::repo::BookInfo> RepositoryImpl::GetBooks(
	const std::optional<std::string> book_id, const std::optional<std::string> book_name) {
	std::vector<domain::repo::BookInfo> result;
	for (auto [title, year, author_name] : work_.query<string_view, int, string_view>(
			 "SELECT B.title, B.publication_year, A.name FROM books as B "_zv
			 "LEFT JOIN authors as A ON B.author_id = A.id ORDER BY B.title, A.name, B.publication_year"_zv)) {
		result.emplace_back(std::string(title), std::string(author_name), year);
	}

	return result;
}

std::vector<domain::repo::BookExtraInfo> RepositoryImpl::GetBooksExtraInfo(
	const std::optional<std::string> book_id, const std::optional<std::string> book_name) {

	std::vector<domain::repo::BookExtraInfo> result;
	std::string query;

	// clang-format off
	if (book_id) {
		query = "SELECT b.id, b.title, b.publication_year, a.name, t.tags FROM books AS b "
				"LEFT JOIN authors AS a ON b.author_id = a.id "
				"LEFT JOIN (SELECT book_id, STRING_AGG(tag, ', ' ORDER BY tag) AS tags FROM book_tags GROUP BY "
				"book_id) AS t ON b.id = t.book_id "
				"WHERE b.id = '" + work_.esc(*book_id) + "' "
				"ORDER BY b.title, a.name, b.publication_year ";
	} else if (book_name) {
		query = "SELECT b.id, b.title, b.publication_year, a.name, t.tags FROM books AS b "
				"LEFT JOIN authors AS a ON b.author_id = a.id "
				"LEFT JOIN (SELECT book_id, STRING_AGG(tag, ', ' ORDER BY tag) AS tags FROM book_tags GROUP BY "
				"book_id) AS t ON b.id = t.book_id "
				"WHERE b.title = '" + work_.esc(*book_name) + "' "
				"ORDER BY b.title, a.name, b.publication_year ";
	} else {
		query = "SELECT b.id, b.title, b.publication_year, a.name, t.tags FROM books AS b "
				"LEFT JOIN authors AS a ON b.author_id = a.id "
				"LEFT JOIN (SELECT book_id, STRING_AGG(tag, ', ' ORDER BY tag) AS tags FROM book_tags GROUP BY "
				"book_id) AS t ON b.id = t.book_id "
				"ORDER BY b.title, a.name, b.publication_year ";
	}
	// clang-format on

	for (auto [book_id, title, year, author_name, tags] :
		work_.query<string_view, string_view, int, string_view, std::optional<string>>(query)) {
		result.emplace_back(
			std::string(book_id), std::string(title), std::string(author_name), year, std::string(tags.value_or("")));
	}
	return result;
}

void RepositoryImpl::DeleteBook(const std::string& book_id) {
	work_.exec("DELETE FROM book_tags WHERE book_id = '" + work_.esc(book_id) + "';");
	work_.exec("DELETE FROM books WHERE id = '" + work_.esc(book_id) + "';");
}

void RepositoryImpl::DeleteAuthor(const std::string& id) {
	work_.exec(
		"DELETE FROM book_tags WHERE book_id IN (SELECT id FROM books WHERE author_id = '" + work_.esc(id) + "');");
	work_.exec("DELETE FROM books WHERE author_id = '" + work_.esc(id) + "';");
	work_.exec("DELETE FROM authors WHERE id = '" + work_.esc(id) + "';");
}

void RepositoryImpl::EditAuthor(const std::string& id, const std::string& new_name) {
	work_.exec("UPDATE authors SET name = '" + work_.esc(new_name) + "' WHERE id = '" + work_.esc(id) + "';");
}

void RepositoryImpl::EditBook(const std::string& book_id, const std::string& title, int year,
	const std::optional<std::vector<std::string>>& tags) {
	if (tags) {
		work_.exec_params("DELETE FROM book_tags WHERE book_id = $1;", book_id);
		for (auto& tag : *tags) {
			work_.exec_params("INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);"_zv, book_id, tag);
		}
	}
	work_.exec_params("UPDATE books SET title = $1, publication_year = $2 WHERE id = $3", title, year, book_id);
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

std::vector<domain::repo::BookInfo> UnitOfWorkImpl::GetBooks(
	const std::optional<std::string> book_id, const std::optional<std::string> book_name) {
	return repo_.GetBooks(book_id, book_name);
}

std::vector<domain::Book> UnitOfWorkImpl::GetAuthorBooks(const domain::AuthorId author_id) {
	return books_repo_.GetAuthorBooks(author_id);
}

// std::vector<domain::Book> UnitOfWorkImpl::GetBookByTitle(const std::string& title) {
// 	return books_repo_.GtBookByTitle(title);
// }

std::vector<domain::repo::BookExtraInfo> UnitOfWorkImpl::GetBooksExtraInfo(
	const std::optional<std::string> book_id, const std::optional<std::string> book_name) {
	return repo_.GetBooksExtraInfo(book_id, book_name);
}

std::optional<domain::Author> UnitOfWorkImpl::GetAuthorByName(const std::string& name) {
	return authors_repo_.GetAuthorByName(name);
}

void UnitOfWorkImpl::DeleteAuthor(const std::string& id) {
	repo_.DeleteAuthor(id);
}

void UnitOfWorkImpl::EditAuthor(const std::string& id, const std::string& new_name) {
	repo_.EditAuthor(id, new_name);
}

void UnitOfWorkImpl::DeleteBook(const std::string& book_id) {
	repo_.DeleteBook(book_id);
}

void UnitOfWorkImpl::EditBook(const std::string& book_id, const std::string& title, int year,
	const std::optional<std::vector<std::string>>& tags) {

	repo_.EditBook(book_id, title, year, tags);
}

void UnitOfWorkImpl::Commit() {
	work_.commit();
}


UnitOfWorkFactoryImpl::UnitOfWorkFactoryImpl(pqxx::connection& connection) : connection_(connection) {}

std::unique_ptr<app::UnitOfWork> UnitOfWorkFactoryImpl::GetUoW() {
	return std::make_unique<UnitOfWorkImpl>(connection_);
}


} // namespace postgres