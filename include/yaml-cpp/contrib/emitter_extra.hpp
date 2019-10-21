/*

Copyright (c) 2019 Macrobull

*/

#pragma once

#include <sstream>
#include <tuple>
#include <type_traits>

#include "yaml-cpp/emitter.h"

namespace YAML
{

namespace detail
{

//// traits

template <bool P, typename T = void>
using enable_if_t = typename std::enable_if<P, T>::type;

//// helpers

template <typename T>
inline enable_if_t<std::is_same<T, char>::value || std::is_same<T, unsigned char>::value, int>
as_numeric(const T& value)
{
	return static_cast<int>(value);
}

template <typename T>
inline enable_if_t<!(std::is_same<T, char>::value || std::is_same<T, unsigned char>::value), T>
as_numeric(const T& value)
{
	return value;
}

template <typename T, size_t N>
struct sequential_emitter
{
	inline static void emit(Emitter& emitter, const T& value)
	{
		const auto i = N - 1;

		sequential_emitter<T, i>::emit(emitter, value);
		emitter << std::get<i>(value);
	}
};

template <typename T>
struct sequential_emitter<T, 1>
{
	inline static void emit(Emitter& emitter, const T& value)
	{
		emitter << std::get<0>(value);
	}
};

template <typename T>
inline Emitter& emit_sequence(Emitter& emitter, const T& value)
{
	emitter << BeginSeq;
	for (const auto& item : value)
	{
		emitter << item;
	}
	return emitter << EndSeq;
}

template <typename T>
inline Emitter& emit_mapping(Emitter& emitter, const T& value)
{
	emitter << BeginMap;
	for (const auto& key_value : value)
	{
		emitter << Key << std::get<0>(key_value) << Value << std::get<1>(key_value);
	}
	return emitter << EndMap;
}

template <typename T>
inline Emitter&
emit_streamable(Emitter& emitter, const T& value, std::stringstream* stream = nullptr)
{
	std::stringstream stream_;
	if (stream == nullptr)
	{
		stream = &stream_;

#ifdef YSL_NAMESPACE // extension

		emitter.SetStreamablePrecision<T>(stream_);

#endif
	}
	else
	{
		stream->str("");
	}

	*stream << value;
	return emitter << stream->str();
}

template <typename T>
inline Emitter& emit_complex(Emitter& emitter, const T& real, const T& imag)
{
#ifndef YAML_EMITTER_NO_COMPLEX

	std::stringstream ss;

#ifdef YSL_NAMESPACE // extension

	emitter.SetStreamablePrecision<T>(ss);

#endif

	ss << as_numeric(real) << '+' << as_numeric(imag) << 'j';
	emitter << LocalTag("complex") << ss.str();
	return emitter;

#else

	return emitter << Flow << BeginSeq << real << imag << EndSeq;

#endif
}

} // namespace detail

//// extra pointers

inline Emitter& operator<<(Emitter& emitter, std::nullptr_t)
{
	return emitter << _Null{};
}

inline Emitter& operator<<(Emitter& emitter, const void* v)
{
	return emitter.WriteIntegralType(v);
}

template <typename... Args>
struct Sequential
{
	/*const*/ std::tuple<Args...> values;

	inline void emit(Emitter& emitter)
	{
		detail::sequential_emitter<std::tuple<Args...>, sizeof...(Args)>::emit(emitter, values);
	}
};

template <typename... CArgs>
inline Sequential<CArgs...> make_sequential(CArgs... args)
{
	return Sequential<CArgs...>{std::move(std::make_tuple(std::forward<CArgs>(args)...))};
}

//// sequential

template <typename... Args>
Emitter& operator<<(Emitter& emitter, const Sequential<Args...>& value)
{
	value.emit(emitter);
	return emitter;
}

} // namespace YAML
