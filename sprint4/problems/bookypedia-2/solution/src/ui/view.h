#pragma once
#include "../domain/repository.h"
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
	std::string title;
	std::string author_id;
	int publication_year = 0;
	std::vector<std::string> tags;
};

struct AuthorInfo {
	std::string id;
	std::string name;
};

struct BookInfo {
	std::string title;
	int publication_year;
};

struct BookExtraInfo {
	std::string title;
	std::string author_name;
	int publication_year;

	explicit BookExtraInfo(std::string title, std::string author_name, int year)
		: title(std::move(title)), author_name(std::move(author_name)), publication_year(year) {}

	explicit BookExtraInfo(domain::repo::BookExtraInfo response)
		: title(response.title), author_name(response.author_name), publication_year(response.year) {}
};

struct BookExtraInfoWTags {
	std::string title;
	std::string author_name;
	int publication_year;
	std::string tags;
};

} // namespace detail

class View {
  public:
	View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

  private:
	bool AddAuthor(std::istream& cmd_input) const;
	bool ShowAuthors() const;
	bool ShowAuthorBooks() const;
	bool DeleteAuthor(std::istream& cmd_input) const;
	bool EditAuthor(std::istream& cmd_input) const;

	bool AddBook(std::istream& cmd_input) const;
	bool ShowBooks() const;
	bool ShowBook(std::istream& cmd_input) const;
	bool DeleteBook(std::istream& cmd_input) const;
	bool EditBook(std::istream& cmd_input) const;

	std::vector<detail::AuthorInfo> GetAuthors() const;
	std::optional<std::string> SelectAuthor() const;
	std::optional<std::string> EnterOrSelectAuthor(std::istream& cmd_input) const;
	std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;
	std::optional<detail::AuthorInfo> ReadOrAddAuthor() const;

	std::vector<detail::BookExtraInfo> GetBooks() const;
	std::optional<detail::AddBookParams> ReadBookParams(std::istream& cmd_input) const;
	std::vector<std::string> ReadBookTags() const;
	std::optional<std::string> SelectBook(std::istream& cmd_input) const;

	menu::Menu& menu_;
	app::UseCases& use_cases_;
	std::istream& input_;
	std::ostream& output_;
};

} // namespace ui