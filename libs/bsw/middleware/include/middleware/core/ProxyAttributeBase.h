// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Future.h"
#include "middleware/core/FutureDispatcher.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/type_traits.h>

#include <cstdint>

namespace middleware::core
{

/**
 * Base class for proxy attribute getter functionality.
 * Provides async getter dispatch with future-based request tracking.
 *
 * \tparam REQUEST_LIMIT Maximum concurrent getter requests
 * \tparam DispatcherTraits Traits describing the getter argument type, member ID, etc.
 */
template<uint8_t REQUEST_LIMIT = 1U, typename DispatcherTraits = void>
class ProxyAttributeBase
{
public:
    using AttributeType  = typename DispatcherTraits::ArgumentType;
    using GetterCallback = etl::delegate<void(etl::expected<AttributeType, Future::State> const&)>;

    explicit ProxyAttributeBase(ProxyBase& proxy) : _proxy(&proxy) {}

    /**
     * Initiates an asynchronous attribute retrieval request.
     *
     * \param attrCb Callback invoked with either the attribute value or an error state
     * \return RequestId on success, or HRESULT on failure
     */
    etl::expected<uint16_t, HRESULT> get(GetterCallback const& attrCb) noexcept
    {
        etl::expected<uint16_t, HRESULT> ret{etl::unexpected<HRESULT>{HRESULT::NotRegistered}};

        if (_proxy->isInitialized())
        {
            etl::optional<uint16_t> const reqId = _getDispatcher.obtainRequestId(attrCb);
            if (reqId.has_value())
            {
                Message msg = _proxy->generateMessageHeader(
                    DispatcherTraits::METHOD_MEMBER_ID, reqId.value());
                HRESULT const sendResult = _proxy->sendMessage(msg);
                if (sendResult == HRESULT::Ok)
                {
                    ret = reqId.value();
                }
                else
                {
                    static_cast<void>(_getDispatcher.cancelRequest(reqId.value()));
                    ret = etl::unexpected<HRESULT>{sendResult};
                    logger::logMessageSendingFailure(
                        logger::LogLevel::Error, logger::Error::SendMessage, ret.error(), msg);
                }
            }
            else
            {
                ret = etl::unexpected<HRESULT>{HRESULT::RequestPoolDepleted};
            }
        }
        return ret;
    }

    /**
     * Invalidates all futures for pending attribute getter requests.
     */
    void freeAll() { _getDispatcher.freeAll(); }

    /**
     * Scans all active futures and triggers timeout callbacks for expired requests.
     * Only enabled when TIMEOUT_VALUE > 0.
     */
    template<typename T = DispatcherTraits>
    typename etl::enable_if<(T::TIMEOUT_VALUE > 0U), void>::type updateTimeouts()
    {
        _getDispatcher.updateTimeouts();
    }

    /**
     * Cancels a pending attribute getter request.
     *
     * \param requestId Request identifier returned from get()
     * \return HRESULT Result code
     */
    HRESULT cancelGetterRequest(uint16_t requestId)
    {
        return _getDispatcher.cancelRequest(requestId);
    }

    /**
     * Matches attribute getter responses to pending futures and invokes callbacks.
     *
     * \param msg Middleware message containing the attribute response
     */
    void answerGetterRequest(Message const& msg) { _getDispatcher.answerRequest(msg); }

protected:
    ProxyBase* _proxy = nullptr;

private:
    FutureDispatcher<DispatcherTraits, REQUEST_LIMIT> _getDispatcher;
};

/**
 * Partial specialization for proxy attributes with no getter dispatcher (void traits).
 */
template<uint8_t REQUEST_LIMIT>
class ProxyAttributeBase<REQUEST_LIMIT, void>
{
public:
    explicit ProxyAttributeBase(ProxyBase& proxy) : _proxy(&proxy) {}

protected:
    ProxyBase* _proxy = nullptr;
};

} // namespace middleware::core
