
#pragma once

#include <type_traits>

#if defined(YAML_EMITTER_USE_EIGEN_FORMATTER)

#include <sstream>

#endif

#include <Eigen/Core>

#include "emitter.h"

namespace YAML
{

namespace _eigen
{

//// traits

template <bool P, typename T = void>
using enable_if_t = typename std::enable_if<P, T>::type;

template <typename T>
inline enable_if_t<!(std::is_same<T, char>::value || std::is_same<T, unsigned char>::value), T>
as_numeric(const T& value)
{
	return value;
}

template <typename T>
inline enable_if_t<std::is_same<T, char>::value || std::is_same<T, unsigned char>::value, int>
as_numeric(const T& value)
{
	return static_cast<int>(value);
}

} // namespace _eigen

#ifdef YAML_EMITTER_USE_EIGEN_FORMATTER

//// Eigen::WithFormat

template <typename T>
Emitter& operator<<(Emitter& emitter, const Eigen::WithFormat<T>& value)
{
	std::stringstream ss;
	ss << value;
	return emitter << Literal << ss.str();
}

#endif

//// Eigen::DenseBase

template <typename T>
Emitter& operator<<(Emitter& emitter, const Eigen::DenseBase<T>& value)
{
#ifdef YAML_EMITTER_USE_EIGEN_FORMATTER

	Eigen::IOFormat format{Eigen::StreamPrecision, 0, ", ", "\n", "[", "]"};
	return emitter << LocalTag("tensor") << value.format(format);

#else

	const typename T::Nested matrix = value.eval();

	if (matrix.cols() <= 1)
	{
		emitter << Flow;
	}

	emitter << BeginSeq;
	for (Eigen::Index i = 0; i < matrix.rows(); ++i)
	{
		emitter << Flow << BeginSeq;
		for (Eigen::Index j = 0; j < matrix.cols(); ++j)
		{
			emitter << _eigen::as_numeric(matrix.coeff(i, j));
		}
		emitter << EndSeq;
	}
	return emitter << EndSeq;

#endif
}

} // namespace YAML
