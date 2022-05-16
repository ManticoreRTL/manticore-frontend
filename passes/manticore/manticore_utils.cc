#ifndef MANTICORE_UTIL_H
#define MANTICORE_UTIL_H
#include <string>
#include <vector>
namespace manticore
{

std::vector<std::string> split(const std::string &original, const char delim)
{
	if (original.empty()) {
		return std::vector<std::string>();
	}
	size_t start = 0UL;
	size_t end = std::string::npos;
	std::vector<std::string> parts;
	do {
		end = original.find(delim, start);
		auto token = original.substr(start, end - start);
		parts.push_back(token);
		start = end + 1;
	} while (end != std::string::npos);
	return parts;
}

}; // namespace manticore

#endif