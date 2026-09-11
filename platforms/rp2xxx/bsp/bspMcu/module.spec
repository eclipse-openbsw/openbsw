maturity: raw
unit_test: false
format_check_exclude:
  - "include/3rdparty/nxp/*.h"
  - "include/3rdparty/cmsis/**/*.h"
sca_exclude:
  '*':
    - 'include/3rdparty/nxp/*.h'
    - 'include/3rdparty/cmsis/**/*.h'
oss: true
