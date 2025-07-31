// Copyright 2024 Accenture.

#pragma once

#include "util/stream/IOutputStream.h"

namespace util
{
namespace stream
{
class ISharedOutputStream
{
public:
    ISharedOutputStream(ISharedOutputStream const&)            = delete;
    ISharedOutputStream& operator=(ISharedOutputStream const&) = delete;

    class IContinuousUser
    {
    public:
        IContinuousUser();
        IContinuousUser& operator=(IContinuousUser const&) = delete;

        /**
         * Called by the shared output stream in case the user
         * has ended its output with a call to endOutput with inLine flag
         * set to true and another channel gathers the output.
         * \param stream stream to use for outputs
         */
        virtual void endContinuousOutput(IOutputStream& stream) = 0;
    };

    ISharedOutputStream();

    /**
     * start the output on the stream.
     * \param user optional pointer to the user that wants to continue its output
     * \return reference to the output stream to use.
     * \note this stream is only guaranteed to be valid up to the corresponding call to endOutput().
     */
    virtual IOutputStream& startOutput(IContinuousUser* user) = 0;
    /**
     * end the output previously started with startOutput.
     * \param user optional pointer to the user that has ended its output and wants to continue
     *        the output on the next call to startOutput. If another user allocates the output in
     *        between the method endContinuousOutput on the user interface will be called.
     */
    virtual void endOutput(IContinuousUser* user)             = 0;
    /**
     * remove pending references to a continuous user. This method has to be called if a user object
     * gets invalid and there was a previous call to endOutput with the corresponding user. From
     * within this function call the user callback may be called to end the previously started
     * continuous output.
     *
     * \param user reference to the user interface for which all references should be removed
     */
    virtual void releaseContinuousUser(IContinuousUser& user) = 0;
};

/**
 * Implementation.
 */
inline ISharedOutputStream::ISharedOutputStream() = default;

inline ISharedOutputStream::IContinuousUser::IContinuousUser() = default;

} // namespace stream
} // namespace util
