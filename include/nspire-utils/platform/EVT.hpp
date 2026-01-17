#pragma once

#include <cstdint>
#include <cstddef>

// EVT: Exception Vector Table

namespace ntls::platform {

    enum class EVT_Vector : uintptr_t {
        Reset = 0x00000000,
        UndefinedInstruction = 0x00000004,
        SoftwareInterrupt = 0x00000008,
        PrefetchAbort = 0x0000000C,
        DataAbort = 0x00000010,
        IRQ = 0x00000018,
        FIQ = 0x0000001C
    };

    enum class EVT_Position : uintptr_t {
        Low = 0x00000000,
        High = 0xFFFF0000
    };

    void SetEVTPosition(EVT_Position position);
    EVT_Position GetEVTPosition();

    void SetEVTValue(EVT_Vector vector, EVT_Position position, uint32_t instrution);
    uint32_t GetEVTValue(EVT_Vector vector, EVT_Position position);
}