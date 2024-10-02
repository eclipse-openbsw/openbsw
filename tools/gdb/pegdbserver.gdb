target extended-remote :7224

set print asm-demangle on
break estd_assert
load

stepi
