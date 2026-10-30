/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "logger/ComponentConfig.h"
#include "logger/EntrySerializer.h"
#include "logger/IBufferedLoggerOutputClient.h"
#include "logger/IComponentConfig.h"
#include "logger/IEntryFormatter.h"
#include "logger/IEntryOutput.h"
#include "logger/ILoggerListener.h"
#include "logger/ILoggerTime.h"
#include "logger/IPersistenceManager.h"

#include <gtest/gtest.h>

#include <etl/type_traits.h>

namespace
{
using namespace ::logger;

struct SerializerCallback final : IEntrySerializerCallback<>
{
    using Interface = IEntrySerializerCallback<>;

    void onEntry(
        uint32_t,
        uint8_t,
        ::util::logger::Level,
        char const*,
        ::util::format::IPrintfArgumentReader&) override
    {}
};

struct BufferedOutputClient final : IBufferedLoggerOutputClient<>
{
    using Interface = IBufferedLoggerOutputClient<>;

    void addListener(ILoggerListener&) override {}

    void removeListener(ILoggerListener&) override {}

    bool outputEntry(IEntryOutput<>&) override { return false; }
};

struct ComponentConfig final : IComponentConfig
{
    using Interface = IComponentConfig;

    uint8_t getMappingSize() const override { return 0U; }

    ::util::logger::Level getLevel(uint8_t) const override { return ::util::logger::LEVEL_NONE; }

    void setLevel(uint8_t, ::util::logger::Level) override {}

    ::util::logger::LevelInfo getLevelInfo(::util::logger::Level) const override
    {
        return ::util::logger::LevelInfo();
    }

    ::util::logger::LevelInfo getLevelInfoByName(::util::string::ConstString const&) const override
    {
        return ::util::logger::LevelInfo();
    }

    ::util::logger::ComponentInfo getComponentInfo(uint8_t) const override
    {
        return ::util::logger::ComponentInfo();
    }

    ::util::logger::ComponentInfo
    getComponentInfoByName(::util::string::ConstString const&) const override
    {
        return ::util::logger::ComponentInfo();
    }

    void readLevels() override {}

    void writeLevels() override {}
};

struct EntryFormatter final : IEntryFormatter<>
{
    using Interface = IEntryFormatter<>;

    void formatEntry(
        ::util::stream::IOutputStream&,
        uint32_t,
        uint32_t,
        ::util::logger::ComponentInfo const&,
        ::util::logger::LevelInfo const&,
        char const*,
        ::util::format::IPrintfArgumentReader&) const override
    {}
};

struct EntryOutput final : IEntryOutput<>
{
    using Interface = IEntryOutput<>;

    void outputEntry(
        uint32_t,
        uint32_t,
        ::util::logger::ComponentInfo const&,
        ::util::logger::LevelInfo const&,
        char const*,
        ::util::format::IPrintfArgumentReader&) override
    {}
};

struct Listener final : ILoggerListener
{
    using Interface = ILoggerListener;

    void logAvailable() override {}
};

struct LoggerTime final : ILoggerTime<>
{
    using Interface = ILoggerTime<>;

    uint32_t getTimestamp() const override { return 0U; }

    void formatTimestamp(::util::stream::IOutputStream&, uint32_t const&) const override {}
};

struct PersistenceManager final : IPersistenceManager
{
    using Interface = IPersistenceManager;

    bool writeMapping(::etl::span<uint8_t const> const&) const override { return false; }

    ::etl::span<uint8_t const> readMapping(::etl::span<uint8_t>) const override { return {}; }
};

template<class T>
class LoggerInterfaceLifetimeTest : public ::testing::Test
{};

using Implementations = ::testing::Types<
    SerializerCallback,
    BufferedOutputClient,
    ComponentConfig,
    EntryFormatter,
    EntryOutput,
    Listener,
    LoggerTime,
    PersistenceManager>;

TYPED_TEST_SUITE(LoggerInterfaceLifetimeTest, Implementations);

/**
 * Verify non-owning interfaces prohibit base destruction and keep concrete
 * implementations destructible, noncopyable and nonmovable.
 */
TYPED_TEST(LoggerInterfaceLifetimeTest, PreservesNonOwningLifetimeAndCopyPolicy)
{
    using Interface = typename TypeParam::Interface;
    static_assert(!::etl::has_virtual_destructor<Interface>::value);
    static_assert(!::etl::is_destructible<Interface>::value);
    static_assert(!::etl::is_abstract<TypeParam>::value);
    static_assert(::etl::is_default_constructible<TypeParam>::value);
    static_assert(::etl::is_destructible<TypeParam>::value);
    static_assert(!::etl::has_virtual_destructor<TypeParam>::value);
    static_assert(!::etl::is_copy_constructible<TypeParam>::value);
    static_assert(!::etl::is_copy_assignable<TypeParam>::value);
    static_assert(!::etl::is_move_constructible<TypeParam>::value);
    static_assert(!::etl::is_move_assignable<TypeParam>::value);
}

/**
 * Verify the concrete configuration base supports direct destruction and subclassing
 * without making the non-owning configuration interface publicly destructible.
 */
TEST(ComponentConfigLifetimeTest, PreservesConcreteConfigurationBaseDestruction)
{
    using Config = ::logger::ComponentConfig<1U>;
    static_assert(::etl::is_destructible<Config>::value);
    static_assert(::etl::has_virtual_destructor<Config>::value);
    static_assert(!::etl::is_copy_constructible<Config>::value);
    static_assert(!::etl::is_copy_assignable<Config>::value);
    static_assert(!::etl::is_move_constructible<Config>::value);
    static_assert(!::etl::is_move_assignable<Config>::value);
}
} // namespace
