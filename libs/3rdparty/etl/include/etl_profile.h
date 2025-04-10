#ifndef ETL_PROFILE_H
#define ETL_PROFILE_H

//*****************************************************************************
// Generic C++14
//*****************************************************************************

#define ETL_TARGET_DEVICE_GENERIC
#define ETL_TARGET_OS_NONE
#define ETL_COMPILER_GCC
#define ETL_CPP11_SUPPORTED 1
#define ETL_CPP14_SUPPORTED 1

#if __cplusplus >= 201703L
#define ETL_CPP17_SUPPORTED 1
#else
#define ETL_CPP17_SUPPORTED 0
#endif

// workaround instead of 202002L because of old arm cross compiler version
#if __cplusplus > 201703L
#define ETL_CPP20_SUPPORTED 1
#else
#define ETL_CPP20_SUPPORTED 0
#endif

#if __cplusplus >= 202302L
#define ETL_CPP23_SUPPORTED 1
#else
#define ETL_CPP23_SUPPORTED 0
#endif

#define ETL_NO_NULLPTR_SUPPORT 0
#define ETL_NO_LARGE_CHAR_SUPPORT 0
#define ETL_CPP11_TYPE_TRAITS_IS_TRIVIAL_SUPPORTED 0
#define ETL_NO_STL
#define ETL_NO_INITIALIZER_LIST

#endif
