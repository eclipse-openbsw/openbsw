#ifndef ETL_PROFILE_H
#define ETL_PROFILE_H

#define ETL_TARGET_DEVICE_GENERIC
#define ETL_TARGET_OS_NONE

//*****************************************************************************
// C++23 support is still not deduced by ETL, since there is no full support
// by all compilers yet. So, if C++23 is to be used, we define here ETL's
// necessary macro to enable its features.
//*****************************************************************************

#if __cplusplus >= 202302L
#define ETL_CPP23_SUPPORTED 1
#else
#define ETL_CPP23_SUPPORTED 0
#endif

#define ETL_NO_STL
#define ETL_FORCE_STD_INITIALIZER_LIST

#endif
