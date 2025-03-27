// Copyright 2024 Accenture.

/**
 * Contains estd::none_t
 * \file
 * \ingroup estl_containers
 */
#pragma once

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
