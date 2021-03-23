#include <boost/test/unit_test.hpp>

#include <math/vector_2d.hpp>

namespace math
{
  BOOST_AUTO_TEST_CASE (defaults_to_zero)
  {
    {
      vector_2d const v;
      BOOST_REQUIRE_EQUAL (v.x, 0.0f);
      BOOST_REQUIRE_EQUAL (v.y, 0.0f);
    }
    {
      vector_2d const v (1.0f, 0.0f);
      BOOST_REQUIRE_EQUAL (v.x, 1.0f);
      BOOST_REQUIRE_EQUAL (v.y, 0.0f);
    }
  }

  BOOST_AUTO_TEST_CASE (data_and_xy_are_same)
  {
    vector_2d const v (1.0f, 1.5f);
    BOOST_REQUIRE_EQUAL (v.x, v._data[0]);
    BOOST_REQUIRE_EQUAL (v.y, v._data[1]);
  }

  BOOST_AUTO_TEST_CASE (equality)
  {
    BOOST_REQUIRE_EQUAL ((vector_2d{0.f, 0.f}), vector_2d{});
    BOOST_REQUIRE_NE ((vector_2d {1.0f, 0.0f}), vector_2d{});
    BOOST_REQUIRE_NE ((vector_2d {0.0f, 1.0f}), vector_2d{});
    BOOST_REQUIRE_EQUAL ((vector_2d {0.0f, 1.0f}), (vector_2d {0.0f, 1.0f}));
  }

  BOOST_AUTO_TEST_CASE (implicitly_converts_to_float_ptr)
  {
    vector_2d const v (1.0f, 1.5f);
    float const* data (v);
    BOOST_REQUIRE_EQUAL (data[0], 1.0f);
    BOOST_REQUIRE_EQUAL (data[1], 1.5f);
  }

  BOOST_AUTO_TEST_CASE (multiply_by_factor_multiplies_both_dimensions)
  {
    BOOST_REQUIRE_EQUAL (vector_2d (1.0f, 1.5f) * 2.0f, vector_2d (2.0f, 3.0f));
    BOOST_REQUIRE_EQUAL (vector_2d (1.0f, 1.5f) * 0.0f, vector_2d (0.0f, 0.0f));
  }

  BOOST_AUTO_TEST_CASE (addition)
  {
    BOOST_REQUIRE_EQUAL (vector_2d (1.0f, 2.0f) + vector_2d (3.0f, 5.0f), vector_2d (4.0f, 7.0f));
  }

  BOOST_AUTO_TEST_CASE (rotate_rotates_counter_clockwise)
  {
    //! \note no CHECK_CLOSE for vector_2d but results are rarely == 0.0f
    BOOST_CHECK_SMALL (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (0)).x, 0.0001f);
    BOOST_CHECK_CLOSE (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (0)).y, 1.0f, 0.0001f);

    BOOST_CHECK_CLOSE (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (90)).x, -1.0f, 0.0001f);
    BOOST_CHECK_SMALL (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (90)).y, 0.0001f);

    BOOST_CHECK_SMALL (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (180)).x, 0.0001f);
    BOOST_CHECK_CLOSE (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (180)).y, -1.0f, 0.0001f);

    BOOST_CHECK_CLOSE (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (270)).x, 1.0f, 0.0001f);
    BOOST_CHECK_SMALL (rotate ({0.0f, 0.0f}, {0.0f, 1.0f}, degrees (270)).y, 0.0001f);
  }
}
