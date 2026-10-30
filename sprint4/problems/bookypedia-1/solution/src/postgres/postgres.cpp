#include "postgres.h"

#include <pqxx/pqxx>
#include <pqxx/zview.hxx>

#include <string_view>

namespace postgres {

using namespace std;
using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
	// Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
	// В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
	// запросов выполнить в рамках одной транзакции.
	// Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
	pqxx::work work{connection_};
	work.exec_params(
		R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
		author.GetId().ToString(), author.GetName());
	work.commit();
}

std::vector<domain::Author> AuthorRepositoryImpl::Load() {
	std::vector<domain::Author> result;
	pqxx::read_transaction read(connection_);
	for (auto [id, name] : read.query<string_view, string_view>("SELECT id, name FROM authors ORDER BY name"_zv)) {
		result.emplace_back(domain::AuthorId::FromString(std::string(id)), std::string(name));
	}

	return result;
}


void BookRepositoryImpl::Save(const domain::Book& book) {
	// Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
	// В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
	// запросов выполнить в рамках одной транзакции.
	// Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
	pqxx::work work{connection_};
	work.exec_params(
		R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
)"_zv,
		book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetYear());

	work.commit();
}

std::vector<domain::Book> BookRepositoryImpl::Load() {
	std::vector<domain::Book> result;
	pqxx::read_transaction read(connection_);
	for (auto [id, author_id, title, year] : read.query<string_view, string_view, string_view, int>(
			 "SELECT id, author_id, title, publication_year FROM books ORDER BY title"_zv)) {
		result.emplace_back(domain::BookId::FromString(std::string(id)),
			domain::AuthorId::FromString(std::string(author_id)), std::string(title), year);
	}

	return result;
}

std::vector<domain::Book> BookRepositoryImpl::GetAuthorBooks(const domain::AuthorId& author_id) {
	std::vector<domain::Book> result;
	pqxx::read_transaction read(connection_);
	for (auto [id, author_id, title, year] : read.query<string_view, string_view, string_view, int>(
			 "SELECT id, author_id, title, publication_year FROM books WHERE author_id = '" + read.esc(author_id.ToString()) + "' ORDER BY publication_year, title;")) {
		result.emplace_back(domain::BookId::FromString(std::string(id)),
			domain::AuthorId::FromString(std::string(author_id)), std::string(title), year);
	}

	return result;
}


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

	// коммитим изменения
	work.commit();
}


} // namespace postgres