// Copyright 2024 Accenture.

/**
 * Alternative Input.
 *
 */
#ifndef GUARD_2D478B82_5925_45FD_ABBF_9A45FF41E6AC
#define GUARD_2D478B82_5925_45FD_ABBF_9A45FF41E6AC
#include "platform/estdint.h"

namespace bios
{
class AlternativeDigitalInput
{
public:
    AlternativeDigitalInput();
    void init(uint16_t port, uint16_t config, uint8_t init_state);
    uint8_t process(uint16_t port_in, uint16_t max_debounce, bool inv);

    bool getState() const { return static_cast<bool>(state); }

    bool getErEdge() const { return static_cast<bool>(edge_pos); }

    bool getFlEdge() const { return static_cast<bool>(edge_neg); }

private:
    uint32_t state      : 1;
    uint32_t edge_neg   : 1;
    uint32_t edge_pos   : 1;
    uint32_t status_old : 1;
    uint32_t reser      : 12;
    uint16_t debounced;
};

} // namespace bios

#endif /*GUARD_2D478B82_5925_45FD_ABBF_9A45FF41E6AC*/
