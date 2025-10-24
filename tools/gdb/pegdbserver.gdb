target extended-remote :7224

set print asm-demangle on
break etl_assert_function
load

stepi
