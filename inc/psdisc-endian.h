// Contents released under the The MIT License (MIT)

#pragma once

#ifdef _MSC_VER
#   include <cstdlib>
#   define __bswap16(in)    _byteswap_ushort(in)
#   define __bswap32(in)    _byteswap_ulong(in)
#   define __bswap64(in)    _byteswap_uint64(in)
#else
#   include <cstdlib>
#   define __bswap16(in)    __builtin_bswap16(in)
#   define __bswap32(in)    __builtin_bswap32(in)
#   define __bswap64(in)    __builtin_bswap64(in)
#endif

#ifndef _BIG_ENDIAN
#	define _BIG_ENDIAN 0
#endif

#if _BIG_ENDIAN
template<typename T>
T LoadFromBE(const T& src) {
    return src;
}

template<typename T>
T LoadFromLE(const T& src)
{
    static_assert(
            sizeof(T) == 1
        ||  sizeof(T) == 2
        ||  sizeof(T) == 4
        ||  sizeof(T) == 8,
        "Invalid input data for LoadFromLE"
    );

    if constexpr (sizeof(T)) return src;
    if constexpr (sizeof(T)) return __bswap16((u16)src);
    if constexpr (sizeof(T)) return __bswap32((u32)src);
    if constexpr (sizeof(T)) return __bswap64((u64)src);

    return src;
}

#else

template<typename T>
T LoadFromLE(const T& src) {
    return src;
}

template<typename T>
T LoadFromBE(const T& src)
{
    static_assert(
            sizeof(T) == 1
        ||  sizeof(T) == 2
        ||  sizeof(T) == 4
        ||  sizeof(T) == 8,
        "Invalid input data for LoadFromBE"
    );

    if constexpr (sizeof(T) == 1) return src;
    if constexpr (sizeof(T) == 2) return __bswap16((uint16_t)src);
    if constexpr (sizeof(T) == 4) return __bswap32((uint32_t)src);
    if constexpr (sizeof(T) == 8) return __bswap64((uint64_t)src);

    return src;
}
#endif

template<typename T, typename T2>
void StoreToLE(T& dest, const T2& src) {
    static_assert(std::is_fundamental<T>::value && std::is_fundamental<T2>::value);
    static_assert(sizeof(T) == sizeof(T2));
    dest = LoadFromLE((T)src);
}

template<typename T, typename T2>
void StoreToBE(T& dest, const T& src) {
    static_assert(std::is_fundamental<T>::value && std::is_fundamental<T2>::value);
    static_assert(sizeof(T) == sizeof(T2));
    dest = LoadFromBE((T)src);
}
