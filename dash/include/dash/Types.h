#ifndef DASH__TYPES_H_
#define DASH__TYPES_H_

#include <array>

namespace dash {

/**
 * Scalar type for a dimension value, with 0 indicating
 * the first dimension.
 */
typedef int dim_t;

/**
 * Signed integer type used as default for index values.
 */
typedef int default_index_t;

/**
 * Difference type for global pointers.
 */
typedef long long gptr_diff_t;

/**
 * Unsigned integer type used as default for size values.
 */
typedef typename std::make_unsigned<default_index_t>::type default_size_t;

template<
  dash::dim_t NumDimensions,
  typename IndexType = int>
struct Point {
  ::std::array<IndexType, NumDimensions> coords;
};

template<
  dash::dim_t NumDimensions,
  typename SizeType = unsigned int>
struct Extent {
  ::std::array<SizeType, NumDimensions> sizes;
};

} // namespace dash

#endif // DASH__TYPES_H_
