// Copyright 2024 Accenture.

/**
 * \ingroup bsp
 */

#include "bsp/background/AbstractBackgroundJob.h"

#include <gtest/gtest.h>

namespace
{
class MyBackgroundJob : public ::bsp::AbstractBackgroundJob
{
public:
    bool execute() override { return true; }
};

/**
 * \desc
 * Verifies that a newly constructed job has not remove requested.
 */
TEST(AbstractBackgroundJobTest, Constructor)
{
    MyBackgroundJob job;
    EXPECT_FALSE(job.isRemoveRequested());
}

/**
 * \desc
 * Verifies that a setRemoveRequested will request removal.
 */
TEST(AbstractBackgroundJobTest, SetRemoveRequested)
{
    MyBackgroundJob job;
    EXPECT_FALSE(job.isRemoveRequested());
    job.setRemoveRequested(true);
    EXPECT_TRUE(job.isRemoveRequested());
    job.removed();
}

} // anonymous namespace
