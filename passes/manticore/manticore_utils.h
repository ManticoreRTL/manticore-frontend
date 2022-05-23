#ifndef MANTICORE_UTIL_H
#define MANTICORE_UTIL_H
#include <string>
#include <vector>
#include "kernel/yosys.h"
#include "kernel/modtools.h"
namespace manticore
{
USING_YOSYS_NAMESPACE
struct RepeatPattern {
	const SigBit bit;
	const int width;
	const int offset;
	RepeatPattern(const SigBit &bit, int width, int pos) : bit(bit), width(width), offset(pos) {}
};

struct BitRepeatBuilder {
	SigBit current_bit;
	int width;
	int pos;
	std::vector<RepeatPattern> res;
	BitRepeatBuilder(const SigBit &first) : current_bit(first), width(0), pos(0) {}
	void consume(const SigBit &b)
	{
		if (current_bit == b) {
			width++;
		} else {
			res.emplace_back(current_bit, width, pos);
			pos += width;
			width = 1;
			current_bit = b;
		}
	}
	std::vector<RepeatPattern> build()
	{
		if (width) {
			res.emplace_back(current_bit, width, pos);
			return res;
		} else {
			std::vector<RepeatPattern> empty;
			return empty;
		}
	}
};
std::vector<RepeatPattern> getRepeats(const SigSpec &sig, const SigMap &sigmap);


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