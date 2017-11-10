#pragma once

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define FC_UNUSED __attribute__ ((unused))
#elif defined(_MSC_VER)
#define FC_UNUSED
#endif
