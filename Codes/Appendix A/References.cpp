#include <iostream>
#include <string>
#include <assert.h>
#include <vector>

void print(std::string const& s);
void process_copy(std::vector<int> const& vec_);

int main()
{
	int var = 42;
	int& ref = var;
	ref = 99;

	assert(var == 99);

	// int& i = 42;
	// int const& i = 42;

	print("Hello");

	int&& i = 42;
	int j = 42;
	// int&& k = j;	// Won't compile

	return 0;
}

void process_copy(std::vector<int> const& vec_)
{
	std::vector<int> vec(vec_);
	vec.push_back(42);
}