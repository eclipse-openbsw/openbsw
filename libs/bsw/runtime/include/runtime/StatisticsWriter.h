// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

#include "util/format/StringWriter.h"

#include <etl/delegate.h>

#include <cstdint>

namespace runtime
{
class StatisticsWriter
{
public:
    struct Mode
    {
        enum class Type : uint8_t
        {
            HEADER,
            LINE,
            VALUE
        };
    };

    template<class Statistics>
    struct FormatStatistics
    {
        using Type = ::etl::delegate<void(StatisticsWriter&, Statistics const&)>;
    };

    StatisticsWriter(
        ::util::format::StringWriter& writer, uint32_t totalRuntime, uint32_t ticksPerUs);

    void setMode(Mode::Type mode);

    void writeEol();
    void writeText(char const* const title, uint32_t minWidth, char const* text);
    void writeNumber(char const* const title, uint32_t minWidth, uint32_t value);
    void writeRuntime(char const* const title, uint32_t minWidth, uint32_t runtime);
    void writeRuntimeMS(char const* const title, uint32_t minWidth, uint32_t runtime);
    void writeRuntimePercentage(char const* const title, uint32_t runtime);
    void writePercentage(char const* const title, uint32_t const value, uint32_t const total);

    template<class Statistics>
    void formatStatisticsLine(
        typename FormatStatistics<Statistics>::Type formatStatistics,
        char const* const title,
        uint32_t minWidth,
        char const* name,
        Statistics const& statistics);
    template<class StatisticsIterator>
    void formatStatisticsGroup(
        typename FormatStatistics<typename StatisticsIterator::StatisticsType>::Type
            formatStatistics,
        char const* const title,
        uint32_t minWidth,
        StatisticsIterator iterator);

private:
    static char const LINE_CHAR = '-';

    bool handleDefaultMode(
        char const* const title,
        uint32_t minWidth,
        uint32_t padding,
        bool isLeftAligned,
        uint32_t& columnWidth);

    ::util::format::StringWriter& _writer;
    uint32_t _totalRuntime;
    uint32_t _ticksPerUs;
    Mode::Type _mode;
    bool _isLineStart;
};

/**
 * Inline implementation.
 */
template<class Statistics>
void StatisticsWriter::formatStatisticsLine(
    typename FormatStatistics<Statistics>::Type const formatStatistics,
    char const* const title,
    uint32_t const minWidth,
    char const* const name,
    Statistics const& statistics)
{
    writeText(title, minWidth, name);
    formatStatistics(*this, statistics);
    writeEol();
}

template<class StatisticsIterator>
void StatisticsWriter::formatStatisticsGroup(
    typename FormatStatistics<typename StatisticsIterator::StatisticsType>::Type const
        formatStatistics,
    char const* const title,
    uint32_t const minWidth,
    StatisticsIterator iterator)
{
    typename StatisticsIterator::StatisticsType const dummy;
    setMode(Mode::Type::HEADER);
    formatStatisticsLine(formatStatistics, title, minWidth, "", dummy);
    setMode(Mode::Type::LINE);
    formatStatisticsLine(formatStatistics, title, minWidth, "", dummy);
    setMode(Mode::Type::VALUE);
    while (iterator.hasValue())
    {
        formatStatisticsLine(
            formatStatistics, title, minWidth, iterator.getName(), iterator.getStatistics());
        iterator.next();
    }
}

} // namespace runtime
