// Contents released under the The MIT License (MIT)

#pragma once

#include <cstdint>
#include <type_traits>

// Image Offset Type, capable of addressing image sizes >4GB
// image offset type MUST be minimally 64 bits in order to support PS2 DVDs correctly.
using psdisc_off_t = int64_t;

// sector offset type, must be capable of handling sizes up to ~256k.
// This type may optionally be set up as an int32_t to reduce the size/overhead of some internal
// data structures.
using psdisc_sec_t = int64_t;
