/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/bindings/someip/SomeIpCodec.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"
#include "someip/ISomeIpSerializable.h"

namespace middleware::bindings::someip
{

namespace internal
{

/**
 * Type-erased accessor for one payload's wire encoding.
 * Lets the dispatch table hold entries for differently typed and differently
 * tagged payloads uniformly. Each MemberBinding owns one per direction.
 */
class IPayloadSlot
{
public:
    virtual ~IPayloadSlot() = default;

    /** Returns the underlying ISomeIpSerializable, for handing to the stack. */
    virtual ::someip::ISomeIpSerializable& serializable() = 0;

    /**
     * Serializes the object at \p src, rather than this slot's own value, until
     * called again with nullptr. \p src must point at a live object of the
     * slot's own T; there is no runtime type check.
     */
    virtual void serializeFrom(void const* src) = 0;

    /**
     * Attaches this slot's decoded value to \p msg as the middleware payload.
     * Uses the typed path, which copy-constructs rather than copying bytes, so a
     * type holding pointers into itself is rebased onto the new storage instead of
     * being left pointing at this slot.
     */
    virtual ::middleware::core::HRESULT allocateInto(::middleware::core::Message& msg) const = 0;
};

template<typename T, typename Tag = internal::ScalarTag<T>>
class PayloadSlot final : public IPayloadSlot
{
public:
    ::someip::ISomeIpSerializable& serializable() override { return _value; }

    void serializeFrom(void const* const src) override
    {
        _value.serializeFrom(static_cast<T const*>(src));
    }

    ::middleware::core::HRESULT allocateInto(::middleware::core::Message& msg) const override
    {
        return ::middleware::core::MessagePayloadBuilder::getInstance().allocate(_value.get(), msg);
    }

private:
    SomeIpSerializableValue<T, Tag> _value;
};

} // namespace internal

} // namespace middleware::bindings::someip
