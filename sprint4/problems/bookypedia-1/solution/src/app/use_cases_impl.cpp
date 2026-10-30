#include "use_cases_impl.h"

#include "../domain/author.h"
#include <stdexcept>

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
	if (name.empty()) {
		throw std::invalid_argument("Error. Author namr can't be empty.");
	}
	authors_.Save({AuthorId::New(), name});
}

std::vector<Author> UseCasesImpl::GetAuthors() {
	return authors_.Load();
}

void UseCasesImpl::AddBook(const std::string& author_id, const std::string& title, int year) {
	books_.Save({BookId::New(), AuthorId::FromString(author_id), title, year});
}

std::vector<Book> UseCasesImpl::GetBooks() {
	return books_.Load();
}

std::vector<domain::Book> UseCasesImpl::GetAuthorBooks(const std::string& author_id) {
	return books_.GetAuthorBooks(domain::AuthorId::FromString(author_id));
}

} // namespace app
