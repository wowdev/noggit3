#include <boost/test/included/unit_test.hpp>

#include <math/trig.hpp>

namespace math
{
  BOOST_AUTO_TEST_CASE (radians_and_degrees_are_trivial_wrappers)
  {
    BOOST_CHECK_EQUAL (radians (10020)._, 10020);
    BOOST_CHECK_EQUAL (degrees (10020)._, 10020);
  }

  BOOST_AUTO_TEST_CASE (deg_to_rad)
  {
    BOOST_CHECK_CLOSE (radians (degrees (0.000f))._, 0.0f * M_PI, 0.0001f);
    BOOST_CHECK_CLOSE (radians (degrees (90.00f))._, 0.5f * M_PI, 0.0001f);
    BOOST_CHECK_CLOSE (radians (degrees (180.0f))._, 1.0f * M_PI, 0.0001f);
    BOOST_CHECK_CLOSE (radians (degrees (270.0f))._, 1.5f * M_PI, 0.0001f);
    BOOST_CHECK_CLOSE (radians (degrees (360.0f))._, 2.0f * M_PI, 0.0001f);
  }

  BOOST_AUTO_TEST_CASE (rad_to_deg)
  {
    BOOST_CHECK_CLOSE (degrees (radians (0.0f * M_PI))._, 0.000f, 0.0001f);
    BOOST_CHECK_CLOSE (degrees (radians (0.5f * M_PI))._, 90.00f, 0.0001f);
    BOOST_CHECK_CLOSE (degrees (radians (1.0f * M_PI))._, 180.0f, 0.0001f);
    BOOST_CHECK_CLOSE (degrees (radians (1.5f * M_PI))._, 270.0f, 0.0001f);
    BOOST_CHECK_CLOSE (degrees (radians (2.0f * M_PI))._, 360.0f, 0.0001f);
  }

  BOOST_AUTO_TEST_CASE (sine)
  {
    BOOST_CHECK_SMALL (sin (radians (0.0f * M_PI)), 0.0001f);
    BOOST_CHECK_CLOSE (1.0f, sin (radians (0.5f * M_PI)), 0.0001f);
    BOOST_CHECK_SMALL (sin (radians (1.0f * M_PI)), 0.0001f);
    BOOST_CHECK_CLOSE (-1.0f, sin (radians (1.5f * M_PI)), 0.0001f);
    BOOST_CHECK_SMALL (sin (radians (2.0f * M_PI)), 0.0001f);

    BOOST_CHECK_SMALL (sin (degrees (0.0f)), 0.0001f);
    BOOST_CHECK_CLOSE (1.0f, sin (degrees (90.0f)), 0.0001f);
    BOOST_CHECK_SMALL (sin (degrees (180.0f)), 0.0001f);
    BOOST_CHECK_CLOSE (-1.0f, sin (degrees (270.0f)), 0.0001f);
    BOOST_CHECK_SMALL (sin (degrees (360.0f)), 0.0001f);
  }

  BOOST_AUTO_TEST_CASE (cosine)
  {
    BOOST_CHECK_CLOSE (1.0f, cos (radians (0.0f * M_PI)), 0.0001f);
    BOOST_CHECK_SMALL (cos (radians (0.5f * M_PI)), 0.0001f);
    BOOST_CHECK_CLOSE (-1.0f, cos (radians (1.0f * M_PI)), 0.0001f);
    BOOST_CHECK_SMALL (cos (radians (1.5f * M_PI)), 0.0001f);
    BOOST_CHECK_CLOSE (1.0f, cos (radians (2.0f * M_PI)), 0.0001f);

    BOOST_CHECK_CLOSE (1.0f, cos (degrees (0.0f)), 0.0001f);
    BOOST_CHECK_SMALL (cos (degrees (90.0f)), 0.0001f);
    BOOST_CHECK_CLOSE (-1.0f, cos (degrees (180.0f)), 0.0001f);
    BOOST_CHECK_SMALL (cos (degrees (270.0f)), 0.0001f);
    BOOST_CHECK_CLOSE (1.0f, cos (degrees (360.0f)), 0.0001f);
  }
}
