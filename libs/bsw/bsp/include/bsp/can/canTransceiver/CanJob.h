// Copyright 2024 Accenture.

/**
 * \ingroup can
 */
#ifndef GUARD_BA68CB3C_8BCC_46EC_B646_E223A142DFB4
#define GUARD_BA68CB3C_8BCC_46EC_B646_E223A142DFB4

#include <estd/array.h>
#include <estd/memory.h>
#include <estd/slice.h>
#include <platform/estdint.h>

#include <cstring>

namespace can
{
struct CanCommandJob
{
    ::estd::array<uint8_t, 8U> data;
    uint32_t canId;
    uint8_t length;
};

struct CanJob
{
    ::estd::array<uint8_t, 64U> data;
    uint32_t canId;
    uint8_t busId;
    uint8_t length;
};

struct CanJobWithCallback : CanJob
{
    void* callback;
    uint64_t timestamp;
};

inline ::estd::slice<uint8_t> payloadOf(CanJob& job)
{
    return ::estd::slice<uint8_t>::from_pointer(job.data.data(), job.length);
}

inline uint8_t setPayload(CanJob& job, ::estd::slice<uint8_t const> const& data)
{
    job.length = static_cast<uint8_t>(data.size());
    (void)::estd::memory::copy(job.data, data);
    return job.length;
}

inline uint8_t setPayload(CanJob& job, uint8_t const* const data, uint8_t const length)
{
    job.length = length;
    memcpy(job.data.data(), data, length);
    return job.length;
}

} // namespace can

#endif /* GUARD_BA68CB3C_8BCC_46EC_B646_E223A142DFB4 */
