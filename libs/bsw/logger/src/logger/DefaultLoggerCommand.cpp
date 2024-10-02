// Copyright 2024 Accenture.

#include "logger/DefaultLoggerCommand.h"

#include <util/format/SharedStringWriter.h>

namespace logger
{
DefaultLoggerCommand::DefaultLoggerCommand(IComponentConfig& componentConfig)
: _componentConfig(componentConfig)
, _levelCommand(
      "level",
      "get/set levels. Usage: logger level [<component>] [<new_level>]",
      ::util::command::SimpleCommand::ExecuteFunction::
          create<DefaultLoggerCommand, &DefaultLoggerCommand::levelCommand>(*this))
, _root("logger", "logger settings")
{
    _root.addCommand(_levelCommand);
}

void DefaultLoggerCommand::levelCommand(::util::command::CommandContext& context)
{
    if (context.hasToken())
    {
        ::util::string::ConstString const id = context.scanIdentifierToken();
        ::util::logger::ComponentInfo const componentInfo
            = _componentConfig.getComponentInfoByName(id);
        if (componentInfo.isValid())
        {
            if (context.hasToken())
            {
                ::util::logger::LevelInfo const levelInfo
                    = _componentConfig.getLevelInfoByName(context.scanIdentifierToken());
                if (context.check(levelInfo.isValid()) && context.checkEol())
                {
                    _componentConfig.setLevel(componentInfo.getIndex(), levelInfo.getLevel());
                    _componentConfig.writeLevels();
                }
            }
            else
            {
                ::util::logger::LevelInfo const levelInfo = _componentConfig.getLevelInfo(
                    _componentConfig.getLevel(componentInfo.getIndex()));
                char const* const lvlInfo = levelInfo.getName().getString();
                (void)::util::format::SharedStringWriter(context).printf("%s\n", lvlInfo);
            }
        }
        else
        {
            ::util::logger::LevelInfo const levelInfo = _componentConfig.getLevelInfoByName(id);
            if (context.check(levelInfo.isValid()) && context.checkEol())
            {
                uint8_t const componentCount = _componentConfig.getMappingSize();
                for (uint8_t idx = 0U; idx < componentCount; ++idx)
                {
                    _componentConfig.setLevel(idx, levelInfo.getLevel());
                }
                _componentConfig.writeLevels();
            }
        }
    }
    else
    {
        ::util::format::SharedStringWriter writer(context);
        (void)writer.printf("%-15s Level\n", "Component");
        uint8_t const componentCount = _componentConfig.getMappingSize();
        for (uint8_t idx = 0U; idx < componentCount; ++idx)
        {
            char const* const componentConfig
                = _componentConfig.getComponentInfo(idx).getName().getString();
            (void)writer.printf(
                "%-15s %s\n",
                componentConfig,
                _componentConfig.getLevelInfo(_componentConfig.getLevel(idx))
                    .getName()
                    .getString());
        }
    }
}

} /* namespace logger */
