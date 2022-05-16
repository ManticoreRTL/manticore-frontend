#ifndef MANTICORE_UTIL_H
#define MANTICORE_UTIL_H
#include <string>
#include <vector>
namespace manticore
{

std::vector<std::string> split(const std::string &original, const char delim);
template <typename T> struct Optional {
	T value;
	bool valid = false;
	Optional(const T &value) : value(value), valid(true) {}
	Optional() : valid(false) {}
	Optional(const Optional<T> &other)
	{
		if (other.valid) {
			valid = true;
			value = other.value;
		} else {
			valid = false;
		}
	}
};

}; // namespace manticore

#endif