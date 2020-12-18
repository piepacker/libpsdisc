// Contents released under the The MIT License (MIT)

#pragma once

#ifdef _MSC_VER
#   include <cstdlib>
#   define __bswap16(in)    _byteswap_ushort(in)
#   define __bswap32(in)    _byteswap_ulong(in)
#   define __bswap64(in)    _byteswap_uint64(in)
#endif


#if BIG_ENDIAN
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

    switch (sizeof(T)) {
        case 1: return src;
        case 2: return __bswap16((u16)src);
        case 4: return __bswap32((u32)src);
        case 8: return __bswap64((u64)src);
    }
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

    switch (sizeof(T)) {
        case 1: return src;
        case 2: return __bswap16((uint16_t)src);
        case 4: return __bswap32((uint32_t)src);
        case 8: return __bswap64((uint64_t)src);
    }
    return src;
}
#endif
