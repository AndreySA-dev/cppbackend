#pragma once
#include <optional>
#include <string>
#include <vector>

namespace domain {

namespace repo {

struct BookInfo {
	std::string title;
	std::string author_name;
	int year;
};

struct BookExtraInfo {
	std::string book_id;
	std::string title;
	std::string author_name;
	int year;
	// std::vector<std::string> tags;
	std::string tags;
};

// struct BookResponseExtra : public BookResponse {
// 	std::string tags;
// 	BookResponseExtra(std::string title, std::string author_name, int year, std::string tags);
// };

class Repository {
  public:
	virtual std::vector<BookInfo> GetBooks(
		const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) = 0;
	virtual std::vector<BookExtraInfo> GetBooksExtraInfo(
		const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) = 0;
	virtual void DeleteBook(const std::string& book_id) = 0;
	virtual void EditBook(const std::string& book_id, const std::string& title, int year, const std::optional<std::vector<std::string>>& tags) = 0;

	virtual void DeleteAuthor(const std::string& id) = 0;
	virtual void EditAuthor(const std::string& id, const std::string& new_name) = 0;

  protected:
	~Repository() = default;
};

} // namespace repo
} // namespace domain