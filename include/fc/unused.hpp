#pragma once

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define FC_UNUSED __attribute__ ((unused))
#else
#define FC_UNUSED
#endif
