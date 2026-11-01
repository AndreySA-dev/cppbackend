#pragma once
#include <string>
#include <vector>

namespace domain {

namespace repo {

struct BookResponse {
	std::string title;
	std::string author_name;
	int year; 
};

class Repository {
  public:
	virtual std::vector<BookResponse> GetBooks() = 0;

  protected:
	~Repository() = default;
};

} // namespace repo
} // namespace domain