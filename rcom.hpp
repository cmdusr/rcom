#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <type_traits>

// Common typedefs
typedef uint8_t byte;

// Remove params
#define EAT(...)

// Seperate params
#define EXPAND(...) __VA_ARGS__

// Combine params
#define CONCAT_IMP(a, b) a ## b
#define CONCAT(a, b) CONCAT_IMP(a, b)

// Make into string
#define STRINGIFY(x) #x
#define XSTRINGIFY(x) STRINGIFY(x)

// Assert implementation
namespace
{
	inline void assert_imp(bool x, const char* msg, const char* file, int line)
	{
		if(!x)
		{
			// Log the failed assertion
			fprintf(stderr, "Assert failed: %s\n file: %s\n line: %i\n", msg, file, line);
	
			// Crash
			abort();
		}
	}
}

#if defined(RCOM_ASSERTS_ENABLED)
	// Make sure this condition is true
	#define ASSERT_TRUE(x, msg) assert_imp((x), (msg), __FILE__, __LINE__)

	// Make sure this condition is false
	#define ASSERT_FALSE(x, msg) assert_imp(!(x), (msg), __FILE__, __LINE__)
#else
	#define ASSERT_TRUE(x, msg)
	#define ASSERT_FALSE(x, msg)
#endif

// Forward declaration for void specialisation
template<typename T> struct ArrayPtr;

// Void specialisation since array access and iteration is not allowed
template<> struct ArrayPtr<void>
{
	void*  data;
	size_t size;

	// Use default constructors and destructors
	ArrayPtr()                           = default;
	~ArrayPtr()                          = default;
	ArrayPtr(ArrayPtr&&)                 = default;
	ArrayPtr& operator=(ArrayPtr&&)      = default;
	ArrayPtr(const ArrayPtr&)            = default;
	ArrayPtr& operator=(const ArrayPtr&) = default;

	// No array access or iteration
};

// Holds a pointer to an array and its size
template<typename T> struct ArrayPtr
{
	T*     data;
	size_t size;

	// Use default constructors and destructors
	ArrayPtr()                           = default;
	~ArrayPtr()                          = default;
	ArrayPtr(ArrayPtr&&)                 = default;
	ArrayPtr& operator=(ArrayPtr&&)      = default;
	ArrayPtr(const ArrayPtr&)            = default;
	ArrayPtr& operator=(const ArrayPtr&) = default;

	// Array access
	      T& operator[](size_t i)       {return data[i];};
	const T& operator[](size_t i) const {return data[i];}

	// Used by range for loop
	      T* begin()       {return data;}
	const T* begin() const {return data;}
	      T* end()         {return data + size;}
	const T* end()   const {return data + size;}

	// Implicit conversion to const
	operator ArrayPtr<const T>() const {return {data, size};}

	// Implicit conversion to void with correct size
	operator ArrayPtr<void>() {return {data, sizeof(T) * size};}
};

inline void zero(ArrayPtr<void> dst)
{
	ASSERT_TRUE(dst.data, "Destination array is null");
	ASSERT_TRUE(dst.size > 0, "Destination array has zero size");
	memset(dst.data, 0, dst.size);
}

inline void copy(ArrayPtr<void> dst, ArrayPtr<void> src)
{
	ASSERT_TRUE(dst.data, "Destination array is null");
	ASSERT_TRUE(src.data, "Source array is null");
	ASSERT_TRUE(dst.size >= src.size, "Destination array is too small");
	memcpy(dst.data, src.data, src.size);
}

// Implementation of array size macro
namespace
{
	template<typename T> inline constexpr size_t array_size_imp()
	{
		static_assert(std::is_array<T>(), "Non array passed as argument to array size macro");
		return std::extent<T>::value;
	}
}

// NOTE: These are for use with raw arrays only (Not pointers to arrays or ArrayPtrs)

// Gets the size of the array. Static assert for array types
#define ARRAY_SIZE(x) array_size_imp<decltype(x)>()

// Get underlying type of array
#define ARRAY_TYPE(x) std::remove_all_extents<decltype(x)>::type

// Size of array in bytes
#define ARRAY_SIZE_BYTES(x) ARRAY_SIZE(x) * sizeof(ARRAY_TYPE(x))

// Creates ArrayPtr from raw array
#define ARRAY_PTR(x) ArrayPtr<ARRAY_TYPE(x)>{x, ARRAY_SIZE(x)}

// Memsets array to zero
#define ARRAY_ZERO(x) static_assert(std::is_array<decltype(x)>(), "Non array passed as argument to array zero macro"); \
	memset(x, 0, ARRAY_BYTE_SIZE(x))

// Memcopies from src to dst
#define ARRAY_COPY(dst, src) static_assert(std::is_array<decltype(dst)>(), "Non array passed as argument to array copy macro"); \
	static_assert(std::is_array<decltype(src)>(), "Non array passed as argument to array copy macro"); \
	ASSERT_TRUE(ARRAY_SIZE(dst) >= ARRAY_SIZE(src), "Destination array smaller than source array"); \
	memcpy(dst, src, ARRAY_BYTE_SIZE(src))

// String comparison
inline bool strequal(const char* s1, const char* s2)
{
	return strcmp(s1, s2) == 0;
}

// Implementation of the defer macro
namespace
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
}
// We generate a uniquly named DeferImp object, concatinating the line number
// Then deduce the DeferImp template parameters using operator <<
// Finally the exposed lamda captures the outside scope and is used as the block of code to be deferred

// Call block of code at the end of scope
#define DEFER_TO_SCOPE auto CONCAT(zz_defer_imp, __LINE__) = DeferTemp{} << [&]()


