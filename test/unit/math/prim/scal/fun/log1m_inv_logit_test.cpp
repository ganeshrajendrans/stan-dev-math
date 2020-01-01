#include <stan/math/prim/scal.hpp>
#include <gtest/gtest.h>
#include <cmath>
#include <limits>

TEST(MathFunctions, log1m_inv_logit) {
  using stan::math::inv_logit;
  using stan::math::log1m_inv_logit;
  using std::log;

  EXPECT_FLOAT_EQ(log(1 - inv_logit(-7.2)), log1m_inv_logit(-7.2));
  EXPECT_FLOAT_EQ(log(1 - inv_logit(0.0)), log1m_inv_logit(0.0));
  EXPECT_FLOAT_EQ(log(1 - inv_logit(1.9)), log1m_inv_logit(1.9));
}

TEST(MathFunctions, log1m_inv_logit_nan) {
  double nan = std::numeric_limits<double>::quiet_NaN();

  EXPECT_TRUE(std::isnan(stan::math::log1m_inv_logit(nan)));
}
