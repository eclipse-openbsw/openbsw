// Copyright 2024 Accenture.

/**
 * Contains interface of a class which runs jobs for other components during
 * a background task (idle task).
 * \ingroup bsp
 *
 * \note * In order to influence the system as little as possible the background task
 * must be preemptive and jobs executed should have as little critical sections
 * as possible if any.
 *
 */
#ifndef GUARD_F1BC5D6A_6E50_47B1_BEBA_D7709994FBAE
#define GUARD_F1BC5D6A_6E50_47B1_BEBA_D7709994FBAE

namespace bsp
{
class AbstractBackgroundJob;

/**
 * Interface for BackgroundJobHandler
 *
 * Interface of a class which runs jobs for other components during a background
 * task.
 */
class IBackgroundJobHandler
{
public:
    /**
     * adds a background job
     * \param job   AbstractBackgroundJob which should be added
     * \return true if call was successfull, false if not
     */
    virtual bool addJob(AbstractBackgroundJob& job) = 0;

    /**
     * removes a background job
     * \param job   AbstractBackgroundJob which should be deleted
     */
    virtual bool removeJob(AbstractBackgroundJob& job) = 0;

protected:
    IBackgroundJobHandler& operator=(IBackgroundJobHandler const&) = default;
};

} // namespace bsp

#endif /*GUARD_F1BC5D6A_6E50_47B1_BEBA_D7709994FBAE*/
