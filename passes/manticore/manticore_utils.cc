#include "passes/manticore/manticore_utils.h"
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
std::vector<RepeatPattern> getRepeats(const SigSpec &sig, const SigMap &sigmap)
{

	auto bits = sig.to_sigbit_vector();
	if (bits.empty()) {
		log_err("Can not get the repeat pattern of an empty signal %s\n", log_signal(sig));
	}

	auto bit0 = bits.front();
	auto builder = BitRepeatBuilder(sigmap(bit0));
	for (const auto &b : bits) {
		builder.consume(sigmap(b));
	}
	return builder.build();
}
}; // namespace manticore
