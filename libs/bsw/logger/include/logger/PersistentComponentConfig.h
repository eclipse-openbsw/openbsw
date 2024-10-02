// Copyright 2024 Accenture.

#ifndef GUARD_31E2381E_019E_47EB_A493_AE62464BA432
#define GUARD_31E2381E_019E_47EB_A493_AE62464BA432

#include "logger/ComponentConfig.h"
#include "logger/IPersistenceManager.h"

#include <util/string/ConstString.h>

#include <estd/array.h>

namespace logger
{
template<uint8_t IndexUpperBound, class Crc8>
class PersistentComponentConfig : public ComponentConfig<IndexUpperBound>
{
public:
    PersistentComponentConfig(
        ComponentMapping<IndexUpperBound>& componentMapping,
        IPersistenceManager& persistenceManager);

    void readLevels() override;
    void writeLevels() override;

private:
    static uint8_t const BUFFER_SIZE = IndexUpperBound + 1U;

    uint8_t getComponentCrc(uint8_t componentCount) const;

    IPersistenceManager& _persistenceManager;
    ::estd::array<uint8_t, BUFFER_SIZE> _buffer{};
};

template<uint8_t IndexUpperBound, class Crc8>
PersistentComponentConfig<IndexUpperBound, Crc8>::PersistentComponentConfig(
    ComponentMapping<IndexUpperBound>& componentMapping, IPersistenceManager& persistenceManager)
: ComponentConfig<IndexUpperBound>(componentMapping), _persistenceManager(persistenceManager)
{}

template<uint8_t IndexUpperBound, class Crc8>
void PersistentComponentConfig<IndexUpperBound, Crc8>::readLevels()
{
    ::estd::slice<uint8_t const> const readBuffer = _persistenceManager.readMapping(_buffer);
    if (readBuffer.size() > 1U)
    {
        uint8_t componentCount = static_cast<uint8_t>(readBuffer.size()) - 1U;
        if (componentCount > IndexUpperBound)
        {
            componentCount = IndexUpperBound;
        }
        uint8_t const crc = readBuffer[0U];
        if (crc == getComponentCrc(componentCount))
        {
            for (uint8_t idx = 0U; idx < componentCount; ++idx)
            {
                this->setLevel(
                    idx,
                    static_cast<::util::logger::Level>(readBuffer[static_cast<size_t>(idx) + 1U]));
            }
        }
    }
}

template<uint8_t IndexUpperBound, class Crc8>
void PersistentComponentConfig<IndexUpperBound, Crc8>::writeLevels()
{
    uint8_t const crc = getComponentCrc(IndexUpperBound);
    _buffer[0U]       = crc;
    for (uint8_t idx = 0U; idx < IndexUpperBound; ++idx)
    {
        _buffer[static_cast<size_t>(idx) + 1U] = static_cast<uint8_t>(this->getLevel(idx));
    }
    (void)_persistenceManager.writeMapping(_buffer);
}

template<uint8_t IndexUpperBound, class Crc8>
uint8_t PersistentComponentConfig<IndexUpperBound, Crc8>::getComponentCrc(
    uint8_t const componentCount) const
{
    Crc8 crc;
    for (uint8_t idx = 0U; idx < componentCount; ++idx)
    {
        ::util::logger::ComponentInfo const componentInfo = this->getComponentInfo(idx);
        ::util::string::ConstString const name(componentInfo.getName().getString());
        (void)crc.update(reinterpret_cast<uint8_t const*>(name.data()), name.length());
    }
    return crc.digest();
}

} // namespace logger

#endif // LOGGER_COMPONENTCONFIG_H_
