// Copyright 2024 Accenture.

/**
 * Contains estd::none_t
 * \file
 * \ingroup estl_containers
 */
#ifndef GUARD_3B852209_F91C_479C_B8CF_8C5B02FBF1A4
#define GUARD_3B852209_F91C_479C_B8CF_8C5B02FBF1A4

namespace estd
{
struct none_t
{
    /**
     * This type implicitly converts to a default constructed instance of anything.
     */
    template<class T>
    operator T() const
    {
        return T();
    }
};

static none_t const none;

} // namespace estd

#endif /* include_guard */
