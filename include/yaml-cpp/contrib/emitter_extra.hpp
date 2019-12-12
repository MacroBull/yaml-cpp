/*

Copyright (c) 2019 Macrobull

*/

#pragma once

#include <sstream>
#include <tuple>
#include <type_traits>

#if defined(YAML_EMITTER_ENABLE_GENERAL_DEMANGLED_TAG) && defined(__GNUG__)

#include <cxxabi.h>

#endif

#include "yaml-cpp/emitter.h"

namespace YAML
{

namespace detail
{

//// traits

template <bool P, typename T = void>
using enable_if_t = typename std::enable_if<P, T>::type;

template <typename... Args>
struct make_void
{
	using type = void;
};

template <typename... Args>
using void_t = typename make_void<Args...>::type;

struct char8
{
	char x[8];
};

template <typename T>
char8 is_complete_helper(char (*)[sizeof(T)]);

template <typename>
char is_complete_helper(...);

template <typename T>
struct is_complete
{
	static const bool value = sizeof(is_complete_helper<T>(nullptr)) != 1;
};

template <class S, class T, typename Test = void>
struct is_streamable : std::false_type
{};

template <class S, class T>
struct is_streamable<S, T, void_t<decltype(std::declval<S&>() << std::declval<T>())>>
	: std::true_type
{};

//// helpers

template <typename T>
inline T as_numeric(T&& value)
{
	return value;
}

template <typename T>
inline enable_if_t<std::is_same<T, char>::value || std::is_same<T, unsigned char>::value, int>
as_numeric(T&& value)
{
	return static_cast<int>(value);
}

template <typename T, size_t N>
struct sequential_printer
{
	template <typename S>
	inline static void print(S& stream, const T& value)
	{
		const auto i = N - 1;

		sequential_printer<T, i>::print(stream, value);
		stream << std::get<i>(value);
	}
};

template <typename T>
struct sequential_printer<T, 0>
{
	template <typename S>
	inline static void print(S& /*stream*/, const T& /*value*/)
	{}
};

template <typename E, typename T> // E can be any Emitter-like
inline E& emit_sequence(E& emitter, T&& value)
{
	emitter << BeginSeq;
	for (const auto& item : value)
	{
		emitter << item;
	}
	return emitter << EndSeq;
}

template <typename E, typename T> // E can be any Emitter-like
inline E& emit_mapping(E& emitter, T&& value)
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
emit_streamable(Emitter& emitter, T&& value, std::stringstream* stream = nullptr)
{
	std::stringstream stream_;
	if (stream == nullptr)
	{
#ifdef YSL_NAMESPACE // extension

		emitter.SetStreamablePrecision<T>(stream_);

#endif

		stream = &stream_;
	}
	else
	{
		stream->str("");
	}

	*stream << std::forward<T>(value);
	return emitter << stream->str();
}

template <typename T>
inline Emitter& emit_complex(Emitter& emitter, T&& real, T&& imag)
{
#ifndef YAML_EMITTER_NO_COMPLEX

	std::stringstream ss;

#ifdef YSL_NAMESPACE // extension

	emitter.SetStreamablePrecision<T>(ss);

#endif

	ss << as_numeric(std::forward<T>(real)) << '+' << as_numeric(std::forward<T>(imag)) << 'j';
	emitter << LocalTag("complex") << ss.str();
	return emitter;

#else

	return emitter << Flow << BeginSeq << as_numeric(real) << as_numeric(imag) << EndSeq;

#endif
}

template <typename T>
inline std::string typeid_name()
{

#if defined(YAML_EMITTER_ENABLE_GENERAL_DEMANGLED_TAG) && defined(__GNUG__)

	int status = 0;

	const auto realname = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
	const std::string name{realname};
	free(realname);

#else

	const std::string name{typeid(T).name()};

#endif

	return name;
}

} // namespace detail

//// extra enums

template <typename T>
inline detail::enable_if_t<std::is_enum<T>::value, Emitter&> operator<<(Emitter& emitter, T v)
{
	emitter << LocalTag(detail::typeid_name<T>());
	return emitter.WriteIntegralType(v);
}

//// extra pointers

inline Emitter& operator<<(Emitter& emitter, std::nullptr_t)
{
	return emitter << _Null{};
}

//// general imcomplete pointers

template <typename T>
inline detail::enable_if_t<!detail::is_complete<T>::value, Emitter&>
operator<<(Emitter& emitter, const T* v)
{
	emitter << LocalTag(detail::typeid_name<T*>()); // tag ptr typeid
	if (v == nullptr)
	{
		return emitter << _Null{};
	}

	return emitter.WriteIntegralType(v);
}

//// general complete pointers

template <typename T>
inline detail::enable_if_t<detail::is_complete<T>::value, Emitter&>
operator<<(Emitter& emitter, const T* v)
{
	// emitter << LocalTag(detail::typeid_name<T>()); // tag typeid
	if (v == nullptr)
	{
		return emitter << _Null{};
	}

	return emitter << *v;
}

//// sequential

template <typename... Args>
struct Sequential
{
	/*const*/ std::tuple<Args...> values;

	template <typename S>
	inline void print(S& stream) const
	{
		detail::sequential_printer<std::tuple<Args...>, sizeof...(Args)>::print(stream, values);
	}
};

template <typename... CArgs>
inline Sequential<CArgs...> make_sequential(CArgs... args)
{
	return Sequential<CArgs...>{std::move(std::make_tuple(std::forward<CArgs>(args)...))};
}

template <typename... Args>
inline Emitter& operator<<(Emitter& emitter, const Sequential<Args...>& value)
{
	value.print(emitter);
	return emitter;
}

} // namespace YAML
