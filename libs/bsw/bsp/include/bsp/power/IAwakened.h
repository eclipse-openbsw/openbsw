// Copyright 2024 Accenture.

#ifndef GUARD_7D39DD0A_8598_4A5C_9BA8_00F697948F68
#define GUARD_7D39DD0A_8598_4A5C_9BA8_00F697948F68

namespace bios
{
class IAwakened
{
    IAwakened& operator=(IAwakened const&);

public:
    /**
     * for the check can be IAwakened
     * the parameter is the nominal cycle time
     */
    virtual bool wokenUp() = 0;
};

} // namespace bios

#endif /* GUARD_7D39DD0A_8598_4A5C_9BA8_00F697948F68 */
