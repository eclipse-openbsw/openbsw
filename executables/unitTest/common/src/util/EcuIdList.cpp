// Copyright 2024 Accenture.

#include "util/EcuIdList.h"

#include <estd/assert.h>

#include <cstdlib>
#include <cstring>

using common::EcuId;

extern "C"
{
static int ecuCompareSmallestFirst(void const* const arg1, void const* const arg2)
{
    return static_cast<int>(*static_cast<EcuId const*>(arg1))
           - static_cast<int>(*static_cast<EcuId const*>(arg2));
}

static int ecuCompareBiggestFirst(void const* const arg1, void const* const arg2)
{
    return static_cast<int>(*static_cast<EcuId const*>(arg2))
           - static_cast<int>(*static_cast<EcuId const*>(arg1));
}
}

namespace common
{
bool operator==(EcuIdList const& x, EcuIdList const& y)
{
    if (x.getSize() == y.getSize())
    {
        if (0 == memcmp(x.fpData, y.fpData, static_cast<size_t>(x.getSize()) + 2U))
        {
            return true;
        }
    }
    return false;
}

bool operator!=(EcuIdList const& x, EcuIdList const& y) { return !(x == y); }

EcuIdList::EcuIdList(uint8_t* const pData, uint16_t const length)
: fpData(pData), fBufferLength(length)
{}

EcuIdList::ErrorCode
EcuIdList::init(uint8_t* const pData, uint16_t const length, bool const initializeWithData)
{
    if ((length <= 2U) || (pData == nullptr)) // no space for ecus
    {
        return ECU_LIST_ERROR;
    }
    fpData        = pData;
    fBufferLength = length;
    if (!initializeWithData)
    {
        ::estd::write_be<uint16_t>(fpData, 0U); // set current length to 0
    }
    return ECU_LIST_OK;
}

EcuId EcuIdList::peek() const
{
    uint16_t const size = getSize();
    estd_assert(size > 0U);
    return fpData[static_cast<uint16_t>(2U + static_cast<uint16_t>(size - 1U))];
}

EcuId EcuIdList::pop_back()
{
    uint16_t const size = getSize();
    estd_assert(size > 0U);
    uint16_t const newSize = static_cast<uint16_t>(size - 1U);
    ::estd::write_be<uint16_t>(fpData, newSize);
    return fpData[static_cast<uint16_t>(2U + static_cast<uint16_t>(size - 1U))];
}

EcuIdList::ErrorCode EcuIdList::getNextEcuId(EcuId& ecuId)
{
    uint16_t size = getSize();
    if (size == 0U)
    {
        return ECU_LIST_ERROR;
    }
    ecuId = fpData[static_cast<uint16_t>(2U + static_cast<uint16_t>(size - 1U))];

    --size;

    uint16_t const newSize = size;
    ::estd::write_be<uint16_t>(fpData, newSize);
    return ECU_LIST_OK;
}

EcuIdList::ErrorCode EcuIdList::push_back(EcuId const ecuId)
{
    if (fpData == nullptr)
    {
        return ECU_LIST_ERROR;
    }
    uint16_t size = getSize();
    if (getMaxSize() == size)
    {
        return ECU_LIST_ERROR;
    }
    ++size;
    ::estd::write_be<uint16_t>(fpData, size);
    fpData[static_cast<uint16_t>(1U + size)] = ecuId;
    return ECU_LIST_OK;
}

void EcuIdList::clearList() { (void)memset(fpData, 0, static_cast<size_t>(fBufferLength)); }

EcuIdList& EcuIdList::operator=(EcuIdList const& other)
{
    if (this == &other)
    {
        return *this;
    }
    estd_assert(fpData != nullptr);
    estd_assert(fBufferLength >= other.fBufferLength);
    (void)memcpy(fpData, other.fpData, static_cast<size_t>(other.fBufferLength));
    return *this;
}

void EcuIdList::sortAscending()
{
    qsort(&fpData[2], static_cast<size_t>(getSize()), 1U, &ecuCompareSmallestFirst);
}

void EcuIdList::sortDescending()
{
    qsort(&fpData[2], static_cast<size_t>(getSize()), 1U, &ecuCompareBiggestFirst);
}

bool EcuIdList::contains(EcuId const ecuId) const
{
    if (fpData == nullptr)
    {
        return false;
    }
    uint16_t const size = ::estd::read_be<uint16_t>(fpData);
    for (uint16_t i = 0U; i < size; ++i)
    {
        if (fpData[static_cast<uint16_t>(2U + i)] == ecuId)
        {
            return true;
        }
    }
    return false;
}

void EcuIdList::remove(EcuId const ecuId)
{
    if (fpData == nullptr)
    {
        return;
    }
    uint16_t size = ::estd::read_be<uint16_t>(fpData);
    bool found    = false;
    uint8_t idx   = 0U;
    for (uint16_t i = 0U; i < size; ++i)
    {
        if (fpData[static_cast<uint16_t>(2U + i)] == ecuId)
        {
            found = true;
            idx   = static_cast<uint8_t>(i);
            break;
        }
    }
    if (found)
    {
        fpData[static_cast<uint8_t>(2U + idx)] = fpData[static_cast<uint16_t>(2U + (size - 1U))];
        --size;
        ::estd::write_be<uint16_t>(fpData, size);
    }
}

EcuIdList::ErrorCode EcuIdList::resetSize(uint16_t const size)
{
    if ((fBufferLength - 2U) >= size)
    {
        ::estd::write_be<uint16_t>(fpData, size);
        return ECU_LIST_OK;
    }
    return ECU_LIST_ERROR;
}

} // namespace common
