// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once

#include <etl/uncopyable.h>

#include <platform/estdint.h>

namespace transport
{
class TransportMessage;

/**
 * This interface is for classes that need to implement a function like routing
 * on the fly.
 *
 * Routing on the fly does not require to wait until a complete
 * TransportMessage has been received but routing commences when the first
 * bytes are available.
 *
 * \see TransportMessage
 * \see TransportMessage::isComplete()
 */
class IDataProgressListener : public etl::uncopyable
{
protected:
    IDataProgressListener() = default;

public:
    /**
     * This function notifies an IDataProgressListener when new data has been
     * added to a given TransportMessage. \param transportMessage
     * TransportMessage that has had progress \param n Number of bytes that have
     * been added
     */
    virtual void dataProgressed(TransportMessage& transportMessage, uint32_t n) = 0;
};

} // namespace transport
