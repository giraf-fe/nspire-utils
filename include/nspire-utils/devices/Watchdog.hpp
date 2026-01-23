#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::devices {
    // Should be an SP805 or compatible watchdog timer
    // https://new.hackspire.org/Memory-mapped_IO_ports_on_CX_II/#90060000---watchdog-timer

    constexpr uintptr_t WatchdogTimerBaseAddress = 0x90060000;

    constexpr uintptr_t Watchdog_Load        = 0x00;
    constexpr uintptr_t Watchdog_Value       = 0x04;
    constexpr uintptr_t Watchdog_Control     = 0x08;
    constexpr uintptr_t Watchdog_IntClear    = 0x0C;
    constexpr uintptr_t Watchdog_RawIntStat  = 0x10;
    constexpr uintptr_t Watchdog_MaskedIntStat = 0x14;

    constexpr uintptr_t Watchdog_Lock        = 0xC00;
    constexpr uint32_t  Watchdog_Lock_Key    = 0x1ACCE551;

    // enables both timer and interrupt
    // i guess that basically means interrupts are always enabled if the watchdog is running
    constexpr uint32_t WatchdogControl_Enable = (1 << 0);
    constexpr uint32_t WatchdogControl_EnableResetBit = (1 << 1);

    class Watchdog_Device {
        uintptr_t baseAddress;
    public:
        Watchdog_Device(uintptr_t baseAddr = WatchdogTimerBaseAddress) : baseAddress(baseAddr) {}

        void Load(uint32_t value) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Load) = value;
        }
        uint32_t LoadedValue() {
            return *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Load);
        }

        uint32_t Value() {
            return *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Value);
        }

        void setControl(uint32_t controlBits) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Control) = controlBits;
        }
        uint32_t getControl() {
            return *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Control);
        }

        void clearInterrupt() {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_IntClear) = 1;
        }

        bool rawInterruptStatus() {
            return (*reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_RawIntStat)) != 0;
        }

        bool maskedInterruptStatus() {
            return (*reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_MaskedIntStat)) != 0;
        }

        void lock() {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Lock) = Watchdog_Lock_Key;
        }
        void unlock() {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Lock) = 0;
        }
        bool isLocked() {
            return (*reinterpret_cast<volatile uint32_t*>(baseAddress + Watchdog_Lock)) != 0;
        }
    };

} // namespace ntls::devices
