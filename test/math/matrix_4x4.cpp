#include <boost/test/unit_test.hpp>

#include <math/matrix_4x4.hpp>

namespace math
{
  BOOST_AUTO_TEST_CASE (translation)
  {
    vector_3d const trans (10.0f, 20.0f, -2.0f);

    BOOST_REQUIRE_EQUAL (matrix_4x4 (matrix_4x4::translation, trans) * vector_3d(), trans);
    BOOST_REQUIRE_EQUAL (matrix_4x4 (matrix_4x4::translation, trans) * trans, 2 * trans);
  }

  BOOST_AUTO_TEST_CASE (scale)
  {
    vector_3d const scal (10.0f, 20.0f, -2.0f);

    BOOST_REQUIRE_EQUAL (matrix_4x4 (matrix_4x4::scale, scal) * vector_3d(), vector_3d());
    BOOST_REQUIRE_EQUAL (matrix_4x4 (matrix_4x4::scale, scal) * vector_3d (1.0f, 1.0f, 1.0f), scal);
  }

  //! \todo CHECK_CLOSE to not fail
  BOOST_AUTO_TEST_CASE (rotation)
  {
    BOOST_CHECK_EQUAL (matrix_4x4 (matrix_4x4::rotation, {0.f, 0.f, 0.f, 1.f}) * vector_3d(), vector_3d());
    BOOST_CHECK_EQUAL (matrix_4x4 (matrix_4x4::rotation, {0.f, 0.f, 0.f, 1.f}) * vector_3d (1.f, 0.f, 0.f), vector_3d (1.f, 0.f, 0.f));
    BOOST_CHECK_EQUAL (matrix_4x4 (matrix_4x4::rotation_xyz, {degrees (90.f), degrees (0.f), degrees (0.f)}) * vector_3d (1.f, 0.f, 0.f), vector_3d (1.f, 0.f, 0.f));
    BOOST_CHECK_EQUAL (matrix_4x4 (matrix_4x4::rotation_xyz, {degrees (0.f), degrees (90.f), degrees (0.f)}) * vector_3d (1.f, 0.f, 0.f), vector_3d (0.f, 0.f, -1.f));
    BOOST_CHECK_EQUAL (matrix_4x4 (matrix_4x4::rotation_xyz, {degrees (0.f), degrees (0.f), degrees (90.f)}) * vector_3d (1.f, 0.f, 0.f), vector_3d (0.f, -1.f, 0.f));
  }
}
