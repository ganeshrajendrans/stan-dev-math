#include <stan/math/prim.hpp>
#include <test/unit/util.hpp>
#include <gtest/gtest.h>
#include <limits>

TEST(prob_transform, lb) {
  EXPECT_FLOAT_EQ(exp(-1.0) + 2.0, stan::math::lb_constrain(-1.0, 2.0));
  EXPECT_THROW(
      stan::math::lb_constrain(7.9, -std::numeric_limits<double>::infinity()),
      std::domain_error);
}

TEST(prob_transform, lb_vec) {
  Eigen::VectorXd input(2);
  input << -1.0, 1.1;
  Eigen::VectorXd lbv(2);
  lbv << 2.0, 3.0;
  double lb = 2.0;

  Eigen::VectorXd resv(2);
  resv << exp(-1.0) + 2.0, exp(1.1) + 3.0;
  Eigen::VectorXd res(2);
  res << exp(-1.0) + 2.0, exp(1.1) + 2.0;
  
  EXPECT_MATRIX_EQ(resv, stan::math::lb_constrain(input, lbv));
  EXPECT_MATRIX_EQ(res, stan::math::lb_constrain(input, lb));

  double lp = 0.0;
  EXPECT_MATRIX_EQ(resv, stan::math::lb_constrain(input, lbv, lp));
  EXPECT_EQ(input.sum(), lp);
  lp = 0.0;
  EXPECT_MATRIX_EQ(res, stan::math::lb_constrain(input, lb, lp));
  EXPECT_EQ(input.sum(), lp);
}

TEST(prob_transform, lb_j) {
  double lp = 15.0;
  EXPECT_FLOAT_EQ(exp(-1.0) + 2.0, stan::math::lb_constrain(-1.0, 2.0, lp));
  EXPECT_FLOAT_EQ(15.0 - 1.0, lp);

  double lp2 = 8.6;
  EXPECT_THROW(stan::math::lb_constrain(
                   7.9, -std::numeric_limits<double>::infinity(), lp2),
               std::domain_error);
  EXPECT_FLOAT_EQ(8.6, lp2);
}
TEST(prob_transform, lb_f) {
  EXPECT_FLOAT_EQ(log(3.0 - 2.0), stan::math::lb_free(3.0, 2.0));
  EXPECT_THROW(
      stan::math::lb_free(1.7, -std::numeric_limits<double>::infinity()),
      std::domain_error);
}
TEST(prob_transform, lb_f_exception) {
  double lb = 2.0;
  EXPECT_THROW(stan::math::lb_free(lb - 0.01, lb), std::domain_error);
}
TEST(prob_transform, lb_rt) {
  double x = -1.0;
  double xc = stan::math::lb_constrain(x, 2.0);
  double xcf = stan::math::lb_free(xc, 2.0);
  EXPECT_FLOAT_EQ(x, xcf);
  double xcfc = stan::math::lb_constrain(xcf, 2.0);
  EXPECT_FLOAT_EQ(xc, xcfc);
}
