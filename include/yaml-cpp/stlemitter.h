#ifndef STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) ||       \
						  (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <list>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "emitter.h"

namespace YAML
{

namespace detail
{

template <typename T, size_t N>
struct tuple_emitter
{
	inline static void emit(Emitter& emitter, const T& value)
	{
		const auto i = N - 1;

		tuple_emitter<T, i>::emit(emitter, value);
		emitter << std::get<i>(value);
	}
};

template <typename T>
struct tuple_emitter<T, 1>
{
	inline static void emit(Emitter& emitter, const T& value)
	{
		emitter << std::get<0>(value);
	}
};

} // namespace detail

template <typename K, typename V>
inline Emitter& operator<<(Emitter& emitter, const std::pair<K, V>& value)
{
	return emitter << Flow << BeginSeq << value.first << value.second << EndSeq;
}

template <typename... Args>
inline Emitter& operator<<(Emitter& emitter, const std::tuple<Args...>& value)
{
	emitter << Flow << BeginSeq;
	detail::tuple_emitter<std::tuple<Args...>, sizeof...(Args)>::emit(emitter, value);
	emitter << EndSeq;
	return emitter;
}

template <typename Seq>
inline Emitter& EmitSeq(Emitter& emitter, const Seq& seq)
{
	emitter << BeginSeq;
	for (typename Seq::const_iterator it = seq.begin(); it != seq.end(); ++it) emitter << *it;
	emitter << EndSeq;
	return emitter;
}

template <typename T>
inline Emitter& operator<<(Emitter& emitter, const std::vector<T>& v)
{
	return EmitSeq(emitter, v);
}

template <typename T>
inline Emitter& operator<<(Emitter& emitter, const std::list<T>& v)
{
	return EmitSeq(emitter, v);
}

template <typename T>
inline Emitter& operator<<(Emitter& emitter, const std::set<T>& v)
{
	return EmitSeq(emitter, v);
}

template <typename K, typename V>
inline Emitter& operator<<(Emitter& emitter, const std::map<K, V>& m)
{
	typedef typename std::map<K, V> map;
	emitter << BeginMap;
	for (typename map::const_iterator it = m.begin(); it != m.end(); ++it)
		emitter << Key << it->first << Value << it->second;
	emitter << EndMap;
	return emitter;
}
} // namespace YAML

#endif // STLEMITTER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
