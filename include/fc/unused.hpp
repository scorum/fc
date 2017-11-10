#pragma once

#if defined(_MSC_VER)
   #define FC_UNUSED
#else
   #define FC_UNUSED __attribute__ ((unused))
#endif
