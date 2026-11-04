#include "use_cases_impl.h"
#include "../domain/author.h"
#include "../domain/book.h"

#include <stdexcept>

namespace app {
using namespace domain;

UseCasesImpl::UseCasesImpl(std::unique_ptr<UnitOfWorkFactory> uow_factory) : uow_factory_(std::move(uow_factory)) {}

domain::Author UseCasesImpl::AddAuthor(const std::string& name) {
	if (name.empty()) {
		throw std::invalid_argument("Error. Author namr can't be empty.");
	}
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	domain::Author new_author{AuthorId::New(), name};
	uow_->AddAuthor(new_author);
	return new_author;
}

std::vector<Author> UseCasesImpl::GetAuthors() {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	return uow_->GetAuthors();
}

std::vector<domain::Book> UseCasesImpl::GetAuthorBooks(const std::string& author_id) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	return uow_->GetAuthorBooks(domain::AuthorId::FromString(author_id));
}

std::optional<domain::Author> UseCasesImpl::GetAuthorByName(const std::string& author_name) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	return uow_->GetAuthorByName(author_name);
}

void UseCasesImpl::DeleteAuthor(const std::string& id) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	uow_->DeleteAuthor(id);
}

void UseCasesImpl::EditAuthor(const std::string& id, const std::string& new_name) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	uow_->EditAuthor(id, new_name);
}

void UseCasesImpl::DeleteBook(const std::string& book_id) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	uow_->DeleteBook(book_id);
}

void UseCasesImpl::EditBook(
	const std::string& book_id, const std::string& title, int year, const std::optional<std::vector<std::string>>& tags) {
		if (!uow_) {
			throw std::logic_error("Error. Transaction not started.");
		}
		uow_->EditBook(book_id, title, year, tags);
}

void UseCasesImpl::AddBook(
	const std::string& author_id, const std::string& title, int year, std::vector<std::string> tags) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	uow_->AddBook({BookId::New(), AuthorId::FromString(author_id), title, year, std::move(tags)});
}

std::vector<domain::repo::BookInfo> UseCasesImpl::GetBooks(
	const std::optional<std::string> book_id, const std::optional<std::string> book_name) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	return uow_->GetBooks(book_id, book_name);
}

// std::vector<domain::Book> UseCasesImpl::GetBookByTitle(const std::string& title) {
// 	if (!uow_) {
// 		throw std::logic_error("Error. Transaction not started.");
// 	}
// 	return uow_->GetBookByTitle(title);
// }

std::vector<domain::repo::BookExtraInfo> UseCasesImpl::GetBooksExtraInfo(
	const std::optional<std::string> book_id, const std::optional<std::string> book_name) {
	if (!uow_) {
		throw std::logic_error("Error. Transaction not started.");
	}
	return uow_->GetBooksExtraInfo(book_id, book_name);
}

void UseCasesImpl::Start() {
	if (uow_) {
		throw std::logic_error("Error. Work already started.");
	}
	uow_ = uow_factory_->GetUoW();
}

void UseCasesImpl::End() {
	if (!uow_) {
		throw std::logic_error("Error. Work not started.");
	}
	uow_->Commit();
	uow_.reset();
}

void UseCasesImpl::Interrupt() {
	uow_.reset();
}

} // namespace app
