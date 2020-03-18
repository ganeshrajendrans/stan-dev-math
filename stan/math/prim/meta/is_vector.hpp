#ifndef STAN_MATH_PRIM_META_IS_VECTOR_HPP
#define STAN_MATH_PRIM_META_IS_VECTOR_HPP

#include <stan/math/prim/meta/bool_constant.hpp>
#include <stan/math/prim/meta/is_eigen.hpp>
#include <stan/math/prim/meta/scalar_type.hpp>
#include <stan/math/prim/meta/value_type.hpp>
#include <stan/math/prim/meta/require_helpers.hpp>
#include <type_traits>
#include <vector>

namespace stan {

/** \ingroup type_trait
 * Base implementation for checking if type is std vector
 */
template <typename T, typename = void>
struct is_std_vector : std::false_type {};

namespace internal {
/** \ingroup type_trait
 * Underlying implementation for detecting if an Eigen Matrix is a column
 * vector.
 */
template <typename T, bool = is_eigen<T>::value>
struct is_eigen_col_vector_impl
    : bool_constant<std::decay_t<T>::ColsAtCompileTime == 1> {};

/** \ingroup type_trait
 * Specialization for when type is not an eigen vector.
 */
template <typename T>
struct is_eigen_col_vector_impl<T, false> : std::false_type {};

/** \ingroup type_trait
 * Underlying implementation for detecting if an Eigen Matrix is a row vector.
 */
template <typename T, bool = is_eigen<T>::value>
struct is_eigen_row_vector_impl
    : std::integral_constant<bool, std::decay_t<T>::RowsAtCompileTime == 1> {};

/** \ingroup type_trait
 * Specialization for when type is not an eigen vector.
 */
template <typename T>
struct is_eigen_row_vector_impl<T, false> : std::false_type {};
}  // namespace internal

/** \ingroup type_trait
 * If the input type T is an eigen matrix with 1 row at compile time this
 * has a static member with a value of true. Else this has a static
 * member with a value of false.
 */
template <typename T>
struct is_eigen_col_vector : internal::is_eigen_col_vector_impl<T> {};

/** \addtogroup require_container_types
*  @{
*/
/**
 * Require a container is an eigen column vector
 */
STAN_ADD_REQUIRE_UNARY(eigen_col_vector, is_eigen_col_vector);
/** @}*/

/** \ingroup type_trait
 * If the input type T is an eigen matrix with 1 column at compile time this
 * has a static member with a value of true. Else this has a static
 * member with a value of false.
 */
template <typename T>
struct is_eigen_row_vector : internal::is_eigen_row_vector_impl<T> {};

/** \addtogroup require_container_types
*  @{
*/
/**
 * Require a container is an eigen row vector.
 */
STAN_ADD_REQUIRE_UNARY(eigen_row_vector, is_eigen_row_vector);
/** @}*/

/** \ingroup type_trait
 * If the input type T is an eigen matrix with 1 column or 1 row at compile time
 * this has a static member with a value of true. Else this has a static
 * member with a value of false.
 */
template <typename T>
struct is_eigen_vector : bool_constant<is_eigen_col_vector<T>::value
                                       || is_eigen_row_vector<T>::value> {};

/** \addtogroup require_container_types
*  @{
*/
/**
 * Require a container is either an eigen row or column vector
 */
STAN_ADD_REQUIRE_UNARY(eigen_vector, is_eigen_vector);
/** @}*/

/** \ingroup type_trait
 * If the input type T is either an eigen matrix with 1 column or 1 row at
 * compile time or a standard vector, this has a static member with a value
 * of true. Else this has a static member with a value of false.
 */
template <typename T>
struct is_vector
    : bool_constant<is_eigen_vector<T>::value || is_std_vector<T>::value> {};

/** \addtogroup require_container_types
*  @{
*/
/**
 * Require a container is either an eigen vector or standard vector.
 */
STAN_ADD_REQUIRE_UNARY(vector, is_vector);
/** @}*/
namespace internal {

/** \ingroup type_trait
 * This underlying implementation is used when the type is not an std vector.
 */
template <typename T>
struct is_std_vector_impl : std::false_type {};

/** \ingroup type_trait
 * This specialization implementation has a static member named value when the
 * template type is an std vector.
 */
template <typename... Args>
struct is_std_vector_impl<std::vector<Args...>> : std::true_type {};

}  // namespace internal

/** \ingroup type_trait
 * Checks if the decayed type of T is a standard vector.
 */
template <typename T>
struct is_std_vector<
    T, std::enable_if_t<internal::is_std_vector_impl<std::decay_t<T>>::value>>
    : std::true_type {};

/** \ingroup type_trait
 * Specialization of scalar_type for vector to recursively return the inner
 * scalar type.
 *
 * @tparam T type of standard vector
 */
template <typename T>
struct scalar_type<T, std::enable_if_t<is_std_vector<T>::value>> {
  using type = scalar_type_t<typename std::decay_t<T>::value_type>;
};

/** \ingroup type_trait
 * Template metaprogram class to compute the type of values stored
 * in a standard vector.
 *
 * @tparam T type of elements in standard vector.
 */
template <typename T>
struct value_type<T, std::enable_if_t<is_std_vector<T>::value>> {
  using type = typename std::decay_t<T>::value_type;
};

/** \addtogroup require_container_types
*  @{
*/
/**
 * Require a container is an standard vector
 */
STAN_ADD_REQUIRE_UNARY(std_vector, is_std_vector);
/** @}*/

}  // namespace stan
#endif
