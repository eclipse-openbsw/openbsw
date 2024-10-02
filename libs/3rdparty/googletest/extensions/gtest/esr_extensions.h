#ifndef GOOGLETEST_ESR_EXTENSIONS_H
#define GOOGLETEST_ESR_EXTENSIONS_H

#include "gmock/gmock.h"

namespace testing
{
ACTION_TEMPLATE(SaveRef, HAS_1_TEMPLATE_PARAMS(int, k), AND_1_VALUE_PARAMS(dest))
{
    *dest = &::testing::get<k>(args);
}

#if ESR_DEPRECATE < 202310

// SaveArgAddress* are deprecated in favor of SaveRef<*>, which
// serves as a drop in replacement.
ACTION_P(SaveArgAddress, pointer) { *pointer = &arg0; }
ACTION_P(SaveArgAddress0, pointer) { *pointer = &arg0; }
ACTION_P(SaveArgAddress1, pointer) { *pointer = &arg1; }

#endif

} // namespace testing

#endif // GOOGLETEST_ESR_EXTENSIONS_H
