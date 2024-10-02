// Copyright 2024 Accenture.

#include "runtime/StatisticsWriter.h"

#include <cstring>

namespace runtime
{
StatisticsWriter::StatisticsWriter(
    ::util::format::StringWriter& writer, uint32_t const totalRuntime, uint32_t const ticksPerUs)
: _writer(writer)
, _totalRuntime(totalRuntime)
, _ticksPerUs(ticksPerUs)
, _mode(Mode::Type::VALUE)
, _isLineStart(true)
{}

void StatisticsWriter::setMode(Mode::Type const mode) { _mode = mode; }

void StatisticsWriter::writeEol()
{
    (void)_writer.write("\n");
    _isLineStart = true;
}

void StatisticsWriter::writeText(
    char const* const title, uint32_t const minWidth, char const* const text)
{
    uint32_t columnWidth = 0U;
    if (!handleDefaultMode(title, minWidth, 0U, true, columnWidth))
    {
        (void)_writer.printf("%-*s", columnWidth, text);
    }
}

void StatisticsWriter::writeNumber(
    char const* const title, uint32_t const minWidth, uint32_t const value)
{
    uint32_t columnWidth = 0U;
    if (!handleDefaultMode(title, minWidth, 0U, false, columnWidth))
    {
        (void)_writer.printf("%*d", columnWidth, value);
    }
}

void StatisticsWriter::writeRuntime(
    char const* const title, uint32_t const minWidth, uint32_t const runtime)
{
    uint32_t columnWidth = 0U;
    if (!handleDefaultMode(title, minWidth, 3U, false, columnWidth))
    {
        (void)_writer.printf("%*d us", columnWidth, runtime / _ticksPerUs);
    }
}

void StatisticsWriter::writeRuntimeMS(
    char const* const title, uint32_t const minWidth, uint32_t const runtime)
{
    uint32_t columnWidth = 0U;
    if (!handleDefaultMode(title, minWidth, 3U, false, columnWidth))
    {
        (void)_writer.printf("%*d ms", columnWidth, runtime / (1000U * _ticksPerUs));
    }
}

void StatisticsWriter::writeRuntimePercentage(char const* const title, uint32_t const runtime)
{
    writePercentage(title, runtime, _totalRuntime);
}

void StatisticsWriter::writePercentage(
    char const* const title, uint32_t const value, uint32_t const total)
{
    uint32_t columnWidth = 0U;
    if (!handleDefaultMode(title, 6U, 3U, false, columnWidth))
    {
        uint32_t const percentage
            = (total != 0U) ? static_cast<uint32_t>(
                  static_cast<uint64_t>(value) * 10000U / static_cast<uint64_t>(total))
                            : 0U;
        (void)_writer.printf(
            " %*s%3d.%02d %%", columnWidth - 6U, "", percentage / 100U, percentage % 100U);
    }
}

bool StatisticsWriter::handleDefaultMode(
    char const* const title,
    uint32_t const minWidth,
    uint32_t const padding,
    bool const isLeftAligned,
    uint32_t& columnWidth)
{
    if (_isLineStart)
    {
        _isLineStart = false;
    }
    else
    {
        (void)_writer.write((_mode == Mode::Type::LINE) ? LINE_CHAR : ' ');
    }
    columnWidth                = minWidth + padding;
    uint32_t const titleLength = static_cast<uint32_t>(strlen(title));
    if (columnWidth < titleLength)
    {
        columnWidth = titleLength;
    }
    switch (_mode)
    {
        case Mode::Type::HEADER:
        {
            if (isLeftAligned)
            {
                (void)_writer.printf("%-*s", columnWidth, title);
            }
            else
            {
                (void)_writer.printf("%*s", columnWidth, title);
            }
            break;
        }
        case Mode::Type::LINE:
        {
            for (uint32_t idx = 0U; idx < columnWidth; ++idx)
            {
                (void)_writer.write("-");
            }
            break;
        }
        // case Mode::VALUE:
        default:
        {
            columnWidth -= padding;
            return false;
        }
    }
    return true;
}

} // namespace runtime
