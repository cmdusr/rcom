#pragma once

#include "misc.hpp"

namespace rcom { namespace hidden
{
	// Use destructor to Call captured function at end of scope (Scope guard)
	template<typename Func> struct DeferImp
	{
		Func f;
		~DeferImp(){f();}
	};

	// Dummy struct used for the next trick
	struct DeferTemp{};

	// Deduce correct template arguments for DeferImp. No brackets '()' needed
	template<typename Func> DeferImp<Func> operator<<(DeferTemp, Func f)
	{
		return {f};
	};
}}
// namespace rcom::hidden

// We generate a uniquly named DeferImp object, concatinating the line number
// Then deduce the DeferImp template parameters using operator <<
// Finally the exposed lamda captures the outside scope and is used as the block of code to be deferred

// Call block of code at the end of scope
#define RCOM_DEFER_TO_SCOPE auto RCOM_CONCAT(zz_defer_imp, __LINE__) = rcom::hidden::DeferTemp{} << [&]()
