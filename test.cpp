#include <iostream>
#include "ollama.hpp"

using namespace std;

int one()
{
	std::cout << ollama::generate("deepseek-r1", "What is 1+1?") << std::endl;
	return 0;
}

int main()
{
	std::string model = "deepseek-r1";
	ollama::setReadTimeout(60 * 10);
	std::string query;
	query = "Why is the sky blue?";
	cout << "query: " << query << endl;
	ollama::response context = ollama::generate(model, query);
	cout << "response: " << context.as_simple_string() << endl;
	query = "Tell me more about this.";
	cout << "query: " << query << endl;
	ollama::response response = ollama::generate(model, query, context);
	cout << "response: " << response.as_simple_string() << endl;
}
