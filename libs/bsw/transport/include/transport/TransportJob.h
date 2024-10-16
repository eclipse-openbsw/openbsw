// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_BF23E108_7BDB_42F4_AD94_F6924A07720A
#define GUARD_BF23E108_7BDB_42F4_AD94_F6924A07720A

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;

/**
 * Class for associating a TransportMessage with an ITransportMessageProcessedListener.
 *
 * \see TransportMessage
 * \see ITransportMessageProcessedListener
 */
class TransportJob
{
    TransportJob(TransportJob const&) = delete;

public:
    TransportJob() : fpTransportMessage(nullptr), fpSendListener(nullptr){};

    TransportJob(TransportMessage& message, ITransportMessageProcessedListener* const pListener)
    : fpTransportMessage(&message), fpSendListener(pListener){};

    TransportJob& operator=(TransportJob const& job)
    {
        if (this != &job)
        {
            fpTransportMessage = job.fpTransportMessage;
            fpSendListener     = job.fpSendListener;
        }
        return *this;
    }

    void setTransportMessage(TransportMessage& message) { fpTransportMessage = &message; }

    TransportMessage* getTransportMessage() const { return fpTransportMessage; }

    void setProcessedListener(ITransportMessageProcessedListener* const pListener)
    {
        fpSendListener = pListener;
    }

    ITransportMessageProcessedListener* getProcessedListener() const { return fpSendListener; }

private:
    TransportMessage* fpTransportMessage;
    ITransportMessageProcessedListener* fpSendListener;
};

} // namespace transport

#endif /* GUARD_BF23E108_7BDB_42F4_AD94_F6924A07720A */
