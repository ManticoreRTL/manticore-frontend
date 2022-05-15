#ifndef MANTICORE_CLOCK_H
#define MANTICORE_CLOCK_H

#include "kernel/yosys.h"
#include "kernel/modtools.h"

USING_YOSYS_NAMESPACE

namespace manticore
{

struct OptionalClock {
	Wire *clk;
	bool polarity;
	OptionalClock() : clk(nullptr), polarity(false) {}
	OptionalClock(Wire *clk, bool p) : clk(clk), polarity(p) {}

	inline bool operator==(const OptionalClock &other) const { return polarity == other.polarity && clk == other.clk; }
	inline bool operator!=(const OptionalClock &other) const { return !operator==(other); }
	inline bool nonEmpty() const { return clk != nullptr; }
	inline bool empty() const { return clk == nullptr; }
};

OptionalClock checkClock(Design *design, Module *mod, const OptionalClock &assumed_clock);

}; // namespace manticore

#endif