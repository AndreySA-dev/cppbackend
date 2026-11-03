#include "view.h"

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <cassert>
#include <iostream>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std;
using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
	out << author.name;
	return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
	out << book.title << ", " << book.publication_year;
	return out;
}

std::ostream& operator<<(std::ostream& out, const BookExtraInfo& book_info) {
	out << book_info.title << " by " << book_info.author_name << ", " << book_info.publication_year;
	return out;
}

// std::ostream& operator<<(std::ostream& out, const BookExtraInfoWTags& info) {
// 	out << "Title: " << info.title << '\n';
// 	out << "Author : " << info.author_name << '\n';
// 	out << "Publication year: " << info.publication_year << '\n';
// 	if (!info.tags.empty()) {
// 		out << "Tags: " << info.tags << '\n';
// 	}
// 	return out;
// }

} // namespace detail

std::ostream& operator<<(std::ostream& out, const domain::repo::BookResponseExtra& info) {
	out << "Title: " << info.title << '\n'
		<< "Author: " << info.author_name << '\n'
		<< "Publication year: " << info.year << '\n';
	if (!info.tags.empty()) {
		out << "Tags: " << info.tags << std::endl;
	}
	return out;
}

template <typename T, typename CastTo = T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
	int i = 1;
	for (auto& value : vector) {
		out << i++ << " " << static_cast<CastTo>(value) << std::endl;
	}
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
	: menu_{menu}, use_cases_{use_cases}, input_{input}, output_{output} {
	menu_.AddAction( //
		"AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
		// либо
		// [this](auto& cmd_input) { return AddAuthor(cmd_input); }
	);
	menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s, std::bind(&View::AddBook, this, ph::_1));
	menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
	menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
	menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s, std::bind(&View::ShowAuthorBooks, this));
	menu_.AddAction("DeleteAuthor"s, {}, "Delete author and his books"s, std::bind(&View::DeleteAuthor, this, ph::_1));
	menu_.AddAction("EditAuthor"s, {}, "Edit author name"s, std::bind(&View::EditAuthor, this, ph::_1));
	menu_.AddAction("ShowBook"s, {}, "Show book detail info"s, std::bind(&View::ShowBook, this, ph::_1));
	menu_.AddAction("DeleteBook"s, {}, "Delete one book"s, std::bind(&View::DeleteBook, this, ph::_1));
}

bool View::AddAuthor(std::istream& cmd_input) const { // <------- MENU ACTION
	try {
		std::string name;
		std::getline(cmd_input, name);
		boost::algorithm::trim(name);
		app::UseCasesGuard guard(use_cases_);
		use_cases_.AddAuthor(std::move(name));
		use_cases_.End();
	} catch (const std::exception&) { output_ << "Failed to add author"sv << std::endl; }
	return true;
}

bool View::ShowAuthors() const { // <------- MENU ACTION
	app::UseCasesGuard guard(use_cases_);
	PrintVector(output_, GetAuthors());
	// use_cases_.End();
	return true;
}

bool View::ShowAuthorBooks() const { // <------- MENU ACTION
	// TODO: handle error
	try {
		app::UseCasesGuard guard(use_cases_);
		if (auto author_id = SelectAuthor()) {
			PrintVector(output_, GetAuthorBooks(*author_id));
		}
		// use_cases_.End();
	} catch (const std::exception& e) { throw std::runtime_error("Failed to Show Books"); }
	return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const { // <------- MENU ACTION
	try {
		app::UseCasesGuard guard(use_cases_);
		if (auto author_id = EnterOrSelectAuthor(cmd_input)) {
			use_cases_.DeleteAuthor(*author_id);
		} else {
			throw std::invalid_argument("Error: No author with this name.");
		}
		use_cases_.End();
	} catch (const std::exception& e) { throw std::runtime_error("Failed to delete author"); }
	return true;
}

bool View::EditAuthor(std::istream& cmd_input) const { // <------- MENU ACTION
	try {
		app::UseCasesGuard guard(use_cases_);
		auto author_id = EnterOrSelectAuthor(cmd_input);
		output_ << "Enter new name:" << endl;
		string new_name;
		if (!std::getline(input_, new_name)) {
			throw std::logic_error("Error. cant read new name");
		}
		if (author_id) {
			use_cases_.EditAuthor(*author_id, new_name);
		} else {
			throw std::invalid_argument("Error EditAuthor. Author with this name not found.");
		}
		use_cases_.End();
	} catch (const std::exception& e) { throw std::runtime_error("Failed to edit author"); }
	return true;
}

bool View::AddBook(std::istream& cmd_input) const { // <------- MENU ACTION
	try {
		app::UseCasesGuard guard(use_cases_);
		if (auto params = GetBookParams(cmd_input)) {
			use_cases_.AddBook(params->author_id, params->title, params->publication_year, std::move(params->tags));
		} else {
			output_ << "Failed to add book"sv << std::endl;
		}
		use_cases_.End();
	} catch (const std::exception& e) { output_ << "Failed to add book"sv << std::endl; }
	return true;
}

bool View::ShowBooks() const { // <------- MENU ACTION
	app::UseCasesGuard guard(use_cases_);
	PrintVector(output_, GetBooks());
	return true;
}

bool View::ShowBook(std::istream& cmd_input) const { // <------- MENU ACTION

	app::UseCasesGuard guard(use_cases_);
	if (auto book_id = SelectBook(cmd_input)) {
		auto books_info = use_cases_.GetBooksExtraInfo({book_id}, std::nullopt);
		output_ << books_info[0];
	}
	return true;
}

bool View::DeleteBook(std::istream& cmd_input) const {
	try {
		app::UseCasesGuard guard(use_cases_);
		if (auto book_id = SelectBook(cmd_input)) {
			use_cases_.DeleteBook(*book_id);
			// auto books_info = use_cases_.GetBooksExtraInfo({book_id}, std::nullopt);
			// output_ << books_info[0];
		}
		use_cases_.End();
	} catch (const std::exception& e) { output_ << "Failed to delete book"sv << std::endl; }
	return true;
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
	std::vector<detail::AuthorInfo> dst_autors;
	for (auto& author : use_cases_.GetAuthors()) {
		dst_autors.emplace_back(author.GetId().ToString(), author.GetName());
	}
	return dst_autors;
}

std::optional<std::string> View::SelectAuthor() const {
	output_ << "Select author:" << std::endl;
	auto authors = GetAuthors();
	PrintVector(output_, authors);
	output_ << "Enter author # or empty line to cancel" << std::endl;

	std::string str;
	if (!std::getline(input_, str) || str.empty()) {
		return std::nullopt;
	}

	int author_idx;
	try {
		author_idx = std::stoi(str);
	} catch (std::exception const&) { throw std::runtime_error("Invalid author num"); }

	--author_idx;
	if (author_idx < 0 or author_idx >= authors.size()) {
		throw std::runtime_error("Invalid author num");
	}

	return authors[author_idx].id;
}

std::optional<std::string> View::EnterOrSelectAuthor(std::istream& cmd_input) const {
	string cmd_line;
	std::getline(cmd_input, cmd_line);
	boost::algorithm::trim(cmd_line);
	if (!cmd_line.empty()) {
		if (auto author = use_cases_.GetAuthorByName(cmd_line)) {
			return author->GetId().ToString();
		} else {
			throw std::invalid_argument("Error: No author wth this name.");
		}
	} else if (auto author_id = SelectAuthor()) {
		return author_id;
	} else {
		throw std::invalid_argument("Error: Author not selected.");
	}
	return {""};
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
	std::vector<detail::BookInfo> books_info;
	auto author_books = use_cases_.GetAuthorBooks(author_id);
	std::transform(author_books.cbegin(), author_books.cend(), std::back_inserter(books_info),
		[](auto& book) { return detail::BookInfo{book.GetTitle(), book.GetYear()}; });
	return books_info;
}

std::optional<detail::AuthorInfo> View::GetOrAddAuthor() const {
	optional<string> author_id{nullopt};

	string name;
	output_ << "Enter author name or empty line to select from list:" << std::endl;
	getline(input_, name);
	boost::algorithm::trim(name);
	if (name.empty()) {
		author_id = SelectAuthor();
		if (author_id)
			return detail::AuthorInfo{*author_id, ""};
		else {
			return std::nullopt;
		}
	}
	auto author = use_cases_.GetAuthorByName(name);
	if (author) {
		return detail::AuthorInfo{author->GetId().ToString(), author->GetName()};
	}

	string answer;
	output_ << "No author found. Do you want to add " + name + " (y/n)?" << std::endl;
	getline(input_, answer);
	if (!(answer == "y" || answer == "Y")) {
		throw std::invalid_argument("Error. Cancel add Author.");
	}

	author = use_cases_.AddAuthor(name);
	return detail::AuthorInfo{author->GetId().ToString(), author->GetName()};
}


std::vector<detail::BookExtraInfo> View::GetBooks() const {
	std::vector<detail::BookExtraInfo> dst_books;
	for (auto& book : use_cases_.GetBooks(nullopt, nullopt)) {
		dst_books.emplace_back(book.title, book.author_name, book.year);
	}
	return dst_books;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
	detail::AddBookParams params;

	cmd_input >> params.publication_year;
	std::getline(cmd_input, params.title);
	boost::algorithm::trim(params.title);

	auto author = GetOrAddAuthor();
	params.tags = SelectBookTags();
	if (!author) {
		return nullopt;
	} else {
		params.author_id = author->id;
		return params;
	}
}

std::vector<std::string> View::SelectBookTags() const {
	std::vector<std::string> result;
	string line;
	output_ << "Enter tags (comma separated):" << std::endl;
	getline(input_, line);
	boost::algorithm::split(result, line, boost::is_any_of(","), boost::token_compress_on);
	for (auto& tag : result) {
		boost::algorithm::trim_all(tag);
	}
	std::erase_if(result, [](const std::string& s) { return s.empty(); });
	std::sort(result.begin(), result.end());
	auto last = std::unique(result.begin(), result.end());
	result.erase(last, result.end());
	return result;
}

// return selected book id
std::optional<std::string> View::SelectBook(std::istream& cmd_input) const {
	string book_name;
	// name entered with command on one line
	if (getline(cmd_input, book_name)) {
		boost::algorithm::trim(book_name);
	}

	vector<domain::repo::BookResponseExtra> books_info;

	if (book_name.empty()) {
		// no book name in command line
		// get all books
		books_info = use_cases_.GetBooksExtraInfo(nullopt, nullopt);
	} else {
		// book name in command line
		// get all books with this name
		books_info = use_cases_.GetBooksExtraInfo(nullopt, {book_name});
	}

	if (books_info.size() == 1) {

		return {books_info[0].book_id};

	} else if (books_info.size() > 1) {

		PrintVector<decltype(books_info)::value_type, detail::BookExtraInfo>(output_, books_info);
		output_ << "Enter the book # or empty line to cancel:" << endl;
		std::string str;
		if (!std::getline(input_, str) || str.empty()) {
			return std::nullopt;
		}

		int book_idx;
		try {
			book_idx = std::stoi(str);
		} catch (std::exception const&) {
			return std::nullopt;
			// throw std::runtime_error("Invalid book num");
		}

		--book_idx;
		if (book_idx < 0 or book_idx >= books_info.size()) {
			return std::nullopt;
			// throw std::runtime_error("Invalid book num");
		}

		return {books_info[book_idx].book_id};
	}

	return std::nullopt;
}

} // namespace ui
