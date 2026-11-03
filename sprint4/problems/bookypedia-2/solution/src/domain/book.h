#pragma once
#include "../util/tagged_uuid.h"
#include "author.h"

#include <string>
#include <vector>


namespace domain {

namespace detail {
struct BookTag {};
} // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
  public:
	Book(BookId id, AuthorId author_id, std::string title, int year, std::vector<std::string> tags)
		: id_(std::move(id)), author_id_(std::move(author_id)), title_(std::move(title)), year_(year),
		  tags_(std::move(tags)) {}

	const BookId& GetId() const noexcept {
		return id_;
	}

	const AuthorId& GetAuthorId() const noexcept {
		return author_id_;
	}

	const std::string& GetTitle() const noexcept {
		return title_;
	}

	int GetYear() const {
		return year_;
	}

	const std::vector<std::string> GetTags() const {
		return tags_;
	}

  private:
	BookId id_;
	AuthorId author_id_;
	std::string title_;
	int year_;
	std::vector<std::string> tags_;
};

class BookRepository {
  public:
	virtual void Save(const Book& author) = 0;
	virtual std::vector<Book> Load() = 0;
	// virtual std::vector<domain::Book> GetBookByTitle(const std::string& title) = 0;
	virtual std::vector<Book> GetAuthorBooks(const domain::AuthorId& author_id) = 0;

  protected:
	~BookRepository() = default;
};


} // namespace domain
