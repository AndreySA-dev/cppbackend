#pragma once
#include <optional>
#include <string>
#include <vector>

namespace domain {

namespace repo {

struct BookResponse {
	std::string title;
	std::string author_name;
	int year;
};

struct BookResponseExtra {
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
	virtual std::vector<BookResponse> GetBooks(
		const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) = 0;
	virtual std::vector<BookResponseExtra> GetBooksExtraInfo(
		const std::optional<std::string> book_id, const std::optional<std::string> book_name = std::nullopt) = 0;
	virtual void DeleteBook(const std::string& book_id) = 0;

	virtual void DeleteAuthor(const std::string& id) = 0;
	virtual void EditAuthor(const std::string& id, const std::string& new_name) = 0;

  protected:
	~Repository() = default;
};

} // namespace repo
} // namespace domain