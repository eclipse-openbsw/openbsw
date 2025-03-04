// Copyright 2024 Accenture.

#ifndef GUARD_C39212E3_9DB9_43AB_8E28_6EFFDA982A3B
#define GUARD_C39212E3_9DB9_43AB_8E28_6EFFDA982A3B

#include <util/logger/Logger.h>

// The purpose of the safety logger is to be able to log form the QM context, and for debugging
// during development. There should not be any logging from within the safety context.

DECLARE_LOGGER_COMPONENT(SAFETY)

#endif /* GUARD_C39212E3_9DB9_43AB_8E28_6EFFDA982A3B */
