#pragma once

// yes the fflush() bookends are needed, otherwise stdout and stderr outputs will issue out-of-order
// to the users' console TTY.

#if !defined(log_host)
#   define log_host(fmt, ...)        (                 fprintf(stdout, fmt "\n", __VA_ARGS__), fflush(nullptr))
#   define log_error(fmt, ...)       (fflush(nullptr), fprintf(stderr, fmt "\n", __VA_ARGS__), fflush(nullptr))
#endif

