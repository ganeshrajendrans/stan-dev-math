#ifndef STAN_MATH_OPENCL_KERNEL_GENERATOR_CHECK_CL_HPP
#define STAN_MATH_OPENCL_KERNEL_GENERATOR_CHECK_CL_HPP
#ifdef STAN_OPENCL

#include <stan/math/prim/meta.hpp>
#include <stan/math/opencl/value_type.hpp>
#include <stan/math/opencl/kernel_generator/operation_cl_lhs.hpp>

namespace stan {
namespace math {

/** \addtogroup opencl_kernel_generator
 *  @{
 */

/**
 * Represents a check in kernel generator expressions. When a check is assigned
 * an expression it will throw if any value in the result of the expression is
 * false.
 * @tparam T type of matrix_cl or expression that is checked
 */
template <typename T>
class check_cl_ : public operation_cl_lhs<check_cl_<T>, bool> {
 public:
  using Scalar = bool;
  using base = operation_cl_lhs<check_cl_<T>, bool>;
  using base::var_name_;
  using base::operator=;

 protected:
  // buffer[0] is indicator whether exception needs to be raised
  // buffer[1,2] are problematic indices
  matrix_cl<int> buffer_;
  matrix_cl<value_type_t<T>> value_;
  T arg_;
  const char* function_;
  const char* err_variable_;
  const char* must_be_;

 public:
  /**
   * Constructor.
   * @param function function name (for error messages)
   * @param err_variabhle variable name (for error messages)
   * @param y variable to check (for error messages)
   * @param must_be description of what the value must be (for error messages)
   */
  check_cl_(const char* function, const char* err_variabhle, T&& y,
            const char* must_be)
      : buffer_(3, 1),
        value_(1, 1),
        arg_(std::forward<T>(y)),
        function_(function),
        err_variable_(err_variabhle),
        must_be_(must_be) {
    buffer_.zeros();
  }

  // this operation can not be used on the right hand side of assignment
  inline kernel_parts get_kernel_parts(...) = delete;

  /**
   * Generates kernel code for this and nested expressions.
   * @param[in,out] generated set of (pointer to) already generated operations
   * @param name_gen name generator for this kernel
   * @param row_index_name row index variable name
   * @param col_index_name column index variable name
   * @return part of kernel with code for this and nested expressions
   */
  inline kernel_parts get_kernel_parts_lhs(
      std::set<const operation_cl_base*>& generated, name_generator& name_gen,
      const std::string& row_index_name,
      const std::string& col_index_name) const {
    kernel_parts res;
    if (generated.count(this) == 0) {
      this->var_name_ = name_gen.generate();
      generated.insert(this);
      res = arg_.get_kernel_parts(generated, name_gen, row_index_name,
                                  col_index_name, false);

      res.args += "__global int* " + var_name_ + "_buffer, __global "
                  + type_str<value_type_t<T>>() + "* " + var_name_ + "_value, ";
      res.body += "bool " + var_name_;
      res.reduction += "if(!" + var_name_ +
            " && atomic_xchg(" + var_name_ + "_buffer, 1) == 1){\n"
          + var_name_ + "_buffer[1] = " + row_index_name + ";\n"
          + var_name_ + "_buffer[2] = " + col_index_name + ";\n"
          + var_name_ + "_value[0] = " + arg_.var_name_ + ";\n"
          "}";
    }
    return res;
  }

  /**
   * Sets kernel arguments for this expression.
   * @param[in,out] generated set of expressions that already set their kernel
   * arguments
   * @param kernel kernel to set arguments on
   * @param[in,out] arg_num consecutive number of the first argument to set.
   * This is incremented for each argument set by this function.
   */
  inline void set_args(std::set<const operation_cl_base*>& generated,
                       cl::Kernel& kernel, int& arg_num) const {
    if (generated.count(this) == 0) {
      generated.insert(this);
      arg_.set_args(generated, kernel, arg_num);
      kernel.setArg(arg_num++, buffer_.buffer());
      kernel.setArg(arg_num++, value_.buffer());
    }
  }

  /**
   * Checks if desired dimensions match dimensions of the argument.
   * @param rows desired number of rows
   * @param cols desired number of columns
   * @throws std::invalid_argument desired dimensions do not match dimensions
   * of the argument.
   */
  inline void check_assign_dimensions(int rows, int cols) const {
    check_size_match("check_cl_.check_assign_dimensions", "Rows of ",
                     "argument", arg_.rows(), "rows of ", "expression", rows);
    check_size_match("check_cl_.check_assign_dimensions", "Columns of ",
                     "argument", arg_.cols(), "columns of ", "expression",
                     cols);
  }

  /**
   * Instead of adding event to matrices, waits on the event and throws if check
   * failed.
   * @param e the event to add
   * @throws std::domain_error check failed
   */
  inline void add_write_event(cl::Event& e) const {
    e.wait();
    Eigen::VectorXi res = from_matrix_cl<Eigen::Dynamic, 1>(buffer_);
    if (res.coeff(0)) {
      Eigen::VectorXd value = from_matrix_cl<Eigen::Dynamic, 1>(value_);
      std::stringstream s;
      s << function_ << ": " << err_variable_ << "[" << res[1] << ", " << res[2]
        << "] = " << value[0] << ", but it must be " << must_be_ << "!";
      throw std::domain_error(s.str());
    }
  }

  /**
   * Number of rows of a matrix that would be the result of evaluating this
   * expression.
   * @return number of rows
   */
  inline int rows() const { return arg_.rows(); }

  /**
   * Number of columns of a matrix that would be the result of evaluating this
   * expression.
   * @return number of columns
   */
  inline int cols() const { return arg_.cols(); }
};

/**
 * Constructs a check on opencl matrix or expression. When assigned a boolean
 * expression it throws a `std::domain_error` if any value of the expression
 * result is false.
 * @tparam T type of the expression to check
 * @param function function name (for error messages)
 * @param var_name variable name (for error messages)
 * @param y variable to check (for error messages)
 * @param must_be description of what the value must be (for error messages)
 */
template <typename T,
          typename = require_all_kernel_expressions_and_none_scalar_t<T>>
inline auto check_cl(const char* function, const char* var_name, T&& y,
                     const char* must_be) {
  return check_cl_<as_operation_cl_t<T>>(
      function, var_name, as_operation_cl(std::forward<T>(y)), must_be);
}

/** @}*/

}  // namespace math
}  // namespace stan

#endif
#endif  // CHECK_CL_HPP
