#pragma once

#include <Eigen/Core>

namespace trianglelite {

#ifdef TRIANGLELITE_SINGLE
using Scalar = float;
#else
using Scalar = double;
#endif

// Triangle requires index to be encoded as int.
using Index = int;

using Matrix1F  = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
using Matrix2Fr = Eigen::Matrix<Scalar, Eigen::Dynamic, 2, Eigen::RowMajor>;
using Matrix1I  = Eigen::Matrix<Index, Eigen::Dynamic, 1>;
using Matrix2Ir = Eigen::Matrix<Index, Eigen::Dynamic, 2, Eigen::RowMajor>;
using Matrix3Ir = Eigen::Matrix<Index, Eigen::Dynamic, 3, Eigen::RowMajor>;

using Matrix1FMap  = Eigen::Map<Matrix1F>;
using Matrix2FrMap = Eigen::Map<Matrix2Fr>;
using Matrix1IMap  = Eigen::Map<Matrix1I>;
using Matrix2IrMap = Eigen::Map<Matrix2Ir>;
using Matrix3IrMap = Eigen::Map<Matrix3Ir>;

} // namespace trianglelite
