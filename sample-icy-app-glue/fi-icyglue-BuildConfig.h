// Contents released under the The MIT License (MIT)

#pragma once

// Default BuildConfig Template for Icy Libraries and Apps.
//
// This file is an alternative to baking options into makefiles and msbuild files.
// It's easier to edit and shows up more clearly in things like include-hierarchies
// and preprocessor dumps.
//
// The main caveat is order of inclusion with other forced-includes. But for most projects,
// a wee bit of architectural planning and subvert that risk.

#if PLATFORM_MSW
#   if !defined(_SECURE_SCL_THROWS)
#       define _SECURE_SCL_THROWS        0
#   endif
#   if !defined(_HAS_EXCEPTIONS)
#       define _HAS_EXCEPTIONS           0
#   endif
#   if !defined(_ITERATOR_DEBUG_LEVEL)
#       define _ITERATOR_DEBUG_LEVEL     0
#   endif
#   if !defined(_CRT_SECURE_NO_WARNINGS)
#       define _CRT_SECURE_NO_WARNINGS   1
#   endif
#   if !defined(_SCL_SECURE_NO_WARNINGS)
#       define _SCL_SECURE_NO_WARNINGS   1
#   endif
#endif


#include "fi-printf-redirect.h"
