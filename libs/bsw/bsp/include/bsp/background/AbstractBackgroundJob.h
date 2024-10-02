// Copyright 2024 Accenture.

/**
 * Contains abstract base class every class which wants to get jobs executed in
 * a background task has to implement.
 * \ingroup bsp
 *
 */
#ifndef GUARD_146DE4AF_8CD4_403A_9707_74FF5FF80739
#define GUARD_146DE4AF_8CD4_403A_9707_74FF5FF80739

#include <estd/forward_list.h>

namespace bsp
{
/**
 * Interface every BackgroundJob has to implement.
 *
 *
 */
class AbstractBackgroundJob : public ::estd::forward_list_node<AbstractBackgroundJob>
{
public:
    /**
     * Constructor
     */
    AbstractBackgroundJob()
    : ::estd::forward_list_node<AbstractBackgroundJob>(), fRemoveRequested(false)
    {}

    /**
     * Contains the code which should be executed during the background task.
     * \return *  - true if execute shall be called again
     *  - false if execution is done and should not be called again
     */
    virtual bool execute() = 0;

    /**
     * Will be called once the background task has been killed.
     * \note * This function will also be called if false is returned from execute
     * \see execute()
     */
    virtual void removed();

    /**
     * \return if remove has been requested
     */
    bool isRemoveRequested() const;

    /**
     * Clients should not use this method. Its to be used by the
     * IBackgroundJobHandler.
     * \param value    new status of remove requested
     *
     * \see IBackgroundJobHandler::addJob()
     * \see IBackgroundJobHandler::removeJob()
     */
    void setRemoveRequested(bool value);

private:
    bool fRemoveRequested;
};

/*
 * inline implementation
 */

// virtual
inline void AbstractBackgroundJob::removed() {}

inline bool AbstractBackgroundJob::isRemoveRequested() const { return fRemoveRequested; }

inline void AbstractBackgroundJob::setRemoveRequested(bool const value)
{
    fRemoveRequested = value;
}

} // namespace bsp

#endif /*GUARD_146DE4AF_8CD4_403A_9707_74FF5FF80739*/
