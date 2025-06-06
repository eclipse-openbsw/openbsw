// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup common
 */
#pragma once

#include <etl/unaligned_type.h>

#include <platform/estdint.h>

namespace common
{
using EcuId = uint8_t;

/**
 * Class for managing a list of EcuIds
 *
 * \note the internal data-layout of this implementation DOES matter!!
 *          do not change! pData points to the data buffer with this layout:
 *          |len_1|len_2|ecu1|ecu2|ecu3|...|
 */
class EcuIdList
{
public:
    enum ErrorCode
    {
        ECU_LIST_OK,
        ECU_LIST_ERROR
    };

    explicit EcuIdList(uint8_t pData[] = nullptr, uint16_t length = 0U);

    EcuIdList(EcuIdList const&) = delete;

    /**
     * initializes the EcuIdList
     *
     * @parameter   pData   valid pointer to a chunk of data
     * @parameter   length  length of the data chunk
     * @parameter   initializeWithData  if the passed in buffer already contains valid data
     *                                  we shall use those (assumes that size is set correctly)
     * \note * this function has to be called before the list can be used since
     * the list uses the space provided in this function
     *
     * \post will set size to 0 if initializeWithData is false
     */
    ErrorCode init(uint8_t* pData, uint16_t length, bool initializeWithData = false);

    uint16_t getMaxSize() const { return static_cast<uint16_t>(fBufferLength - 2U); }

    uint16_t getSize() const
    {
        return (fBufferLength == 0U) ? 0U : static_cast<uint16_t>(etl::be_uint16_t(fpData));
    }

    bool empty() const { return (getSize() == 0U); }

    bool hasMore() const { return (getSize() > 0U); }

    /**
     * peek at last element
     *
     * \return the last ecu in the list
     *
     * \pre size N of list has to be N >= 1
     * \post size before = N, size after = N
     */
    EcuId peek() const;

    /**
     * removes last element
     *
     * \return the last ecu in the list
     *
     * \pre size N of list has to be N >= 1
     * \post size before = N, size after = N-1
     */
    EcuId pop_back();

    /**
     * removes last element
     *
     * \return the last ecu in the list
     *
     * \pre size N of list has to be N >= 1
     * \post size before = N, size after = N-1
     */
    ErrorCode getNextEcuId(EcuId& ecuId);

    /**
     * add ecu to end of list, increase size
     */
    ErrorCode push_back(EcuId ecuId);

    /**
     * sets size to 0 and clears all data
     */
    void clearList();

    EcuIdList& operator=(EcuIdList const& other);

    /**
     * sorts the list of ecus so that the ecu with the smallest ecuId comes first
     * and the one with the largest comes last
     */
    void sortAscending();

    /**
     * sorts the list of ecus in descending order
     */
    void sortDescending();

    uint8_t* toStream();

    bool contains(EcuId ecuId) const;

    void remove(EcuId ecuId);

    ErrorCode resetSize(uint16_t size);

private:
    friend bool operator==(EcuIdList const& x, EcuIdList const& y);
    friend bool operator!=(EcuIdList const& x, EcuIdList const& y);
    uint8_t* fpData;
    uint16_t fBufferLength;
};

inline uint8_t* EcuIdList::toStream() { return fpData; }

} // namespace common
