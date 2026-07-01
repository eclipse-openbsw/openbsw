/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/buffer/LinkedBuffer.h"
#include "util/command/CommandContext.h"
#include "util/command/GroupCommand.h"
#include "util/command/HelpCommand.h"
#include "util/command/ICommand.h"
#include "util/command/IParentCommand.h"
#include "util/command/ParentCommand.h"
#include "util/command/SimpleCommand.h"
#include "util/defer/Defer.h"
#include "util/format/AttributedString.h"
#include "util/format/IPrintfArgumentReader.h"
#include "util/format/Printf.h"
#include "util/format/PrintfArgumentReader.h"
#include "util/format/PrintfFormatScanner.h"
#include "util/format/PrintfFormatter.h"
#include "util/format/SharedStringWriter.h"
#include "util/format/StringWriter.h"
#include "util/format/Vt100AttributedStringFormatter.h"
#include "util/logger/ComponentInfo.h"
#include "util/logger/IComponentMapping.h"
#include "util/logger/ILoggerOutput.h"
#include "util/logger/LevelInfo.h"
#include "util/logger/Logger.h"
#include "util/memory/Bit.h"
#include "util/meta/BinaryValue.h"
#include "util/meta/Bitmask.h"
#include "util/stream/ByteBufferOutputStream.h"
#include "util/stream/INonBlockingInputStream.h"
#include "util/stream/IOutputStream.h"
#include "util/stream/ISharedOutputStream.h"
#include "util/stream/NormalizeLfOutputStream.h"
#include "util/stream/NullOutputStream.h"
#include "util/stream/SharedOutputStream.h"
#include "util/stream/SharedOutputStreamResource.h"
#include "util/stream/StdinStream.h"
#include "util/stream/StdoutStream.h"
#include "util/stream/StringBufferOutputStream.h"
#include "util/stream/TaggedOutputHelper.h"
#include "util/stream/TaggedOutputStream.h"
#include "util/stream/TaggedSharedOutputStream.h"
#include "util/string/ConstString.h"
#include "util/types/Enum.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

TEST(IncludeTest, TestIncludes) {}

} // anonymous namespace
