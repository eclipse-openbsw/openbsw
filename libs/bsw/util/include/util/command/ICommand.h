// Copyright 2024 Accenture.

#pragma once

#include "util/string/ConstString.h"

#include <etl/intrusive_forward_list.h>

namespace util
{
namespace stream
{
class ISharedOutputStream;
}

namespace command
{
class ICommand : public ::etl::forward_link<0>
{
public:
    ICommand& operator=(ICommand const&) = delete;

    /**
     * enumeration defining results of command execution.
     */
    enum class Result : uint8_t
    {
        NOT_RESPONSIBLE,
        OK,
        BAD_TOKEN,
        UNEXPECTED_TOKEN,
        BAD_VALUE,
        ERROR
    };

    /**
     * interface for help information.
     */
    class IHelpCallback
    {
    public:
        IHelpCallback() = default;

        IHelpCallback& operator=(IHelpCallback const&) = delete;

        /**
         * Called to indicate that the help information about a command will begin.
         *
         * \param id identifier of command
         * \param description description of command
         * \param end true indicates that the command is simple and ends here. Otherwise
         *        a corresponding call to endCommand() is expected after all nested help
         *        information has been reported
         */
        virtual void startCommand(char const* id, char const* description, bool end) = 0;
        /**
         * Called to indicate the end of the information started with startCommand().
         */
        virtual void endCommand()                                                    = 0;
    };

    /**
     * structure holding execution result.
     */
    class ExecuteResult
    {
    public:
        explicit ExecuteResult(Result const result = ICommand::Result::OK)
        : ExecuteResult(result, {}, nullptr)
        {}

        ExecuteResult(
            Result const result, ::util::string::ConstString const& suffix, ICommand* const cmd)
        : _result(result), _suffix(suffix), _command(cmd)
        {}

        ExecuteResult(ExecuteResult const& src) = default;

        bool isValid() const { return _result == Result::OK; }

        Result getResult() const { return _result; }

        ::util::string::ConstString const& getSuffix() const { return _suffix; }

        ICommand* getCommand() const { return _command; }

        ExecuteResult& operator=(ExecuteResult const& src)
        {
            if (this != &src)
            {
                _result  = src._result;
                _suffix  = src._suffix;
                _command = src._command;
            }
            return *this;
        }

    private:
        Result _result;
        ::util::string::ConstString _suffix;
        ICommand* _command;
    };

    /**
     * get string that identifies the command.
     *
     * \return constant string that identifies the command
     */
    virtual char const* getId() const = 0;

    /**
     * execute the command with the given argument string.
     *
     * \param arguments String holding the arguments of the command. This buffer is
     *        only valid for use until the end of the function call
     * \param sharedOutputStream output stream for writing. If null then only a lookup
     *        for the executing sub command will take place
     *
     * \return object holding result of execution
     */
    virtual ExecuteResult execute(
        ::util::string::ConstString const& arguments,
        ::util::stream::ISharedOutputStream* sharedOutputStream)
        = 0;

    /**
     * get help information.
     *
     * \param callback reference to interface that retrieves information
     */
    virtual void getHelp(IHelpCallback& callback) const = 0;
};

} // namespace command
} // namespace util
