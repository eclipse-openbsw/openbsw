// Copyright 2024 Accenture.

#ifndef GUARD_85BBDD33_B784_4F2B_968E_240820E76807
#define GUARD_85BBDD33_B784_4F2B_968E_240820E76807

namespace bios
{
struct CommandSpec
{ // lint --e{970}    CommandInterpreter uses C strings, so usage of char is OK
    char const* fName;
    char const* fHelpText;
};
} // namespace bios

#endif /* GUARD_85BBDD33_B784_4F2B_968E_240820E76807 */
