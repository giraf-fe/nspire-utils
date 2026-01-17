#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::devices {

    // https://new.hackspire.org/Memory-mapped_IO_ports_on_CX_II/#90090000---real-time-clock-rtc
    constexpr uintptr_t PL031Deriv_RTC_BaseAddress = 0x90090000;

    constexpr uintptr_t PL031Deriv_RTC_CurrentTime           = 0x00;
    constexpr uintptr_t PL031Deriv_RTC_AlarmValue            = 0x04;
    constexpr uintptr_t PL031Deriv_RTC_SetTime               = 0x08;
    constexpr uintptr_t PL031Deriv_RTC_InterruptMask         = 0x0C;
    constexpr uintptr_t PL031Deriv_RTC_MaskedInterruptStatus = 0x10;
    constexpr uintptr_t PL031Deriv_RTC_Status                = 0x14;

    enum class PL031Deriv_RTC_Statuses : uint32_t {
        TimeSetInProgress = 1 << 0,
        AlarmSetInProgress = 1 << 1,
        InterruptAckInProgress = 1 << 2,
        InterruptMaskSetInProgress = 1 << 3
    };

    // Derivative of the PL031 Real-Time Clock (RTC),
    // used by the TI-Nspire.
    class PL031Deriv_RTC {
        uintptr_t baseAddress;
    public:
        PL031Deriv_RTC(uintptr_t baseAddr = PL031Deriv_RTC_BaseAddress)
            : baseAddress(baseAddr) {}

        uint32_t getCurrentTime() const {
            return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL031Deriv_RTC_CurrentTime);
        }

        // May take time to set.
        void setCurrentTime(uint32_t time) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL031Deriv_RTC_SetTime) = time;
        }   

        void setAlarm(uint32_t alarmTime) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL031Deriv_RTC_AlarmValue) = alarmTime;
        }

        void setInterruptMask(bool enabled) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL031Deriv_RTC_InterruptMask) = enabled ? 1 : 0;
        }
        bool isInterruptMasked() const {
            return (*reinterpret_cast<volatile uint32_t*>(baseAddress + PL031Deriv_RTC_MaskedInterruptStatus)) != 0;
        }

        void acknowledgeInterrupt() {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL031Deriv_RTC_Status) = 1;
        }

        uint32_t getStatus() const {
            return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL031Deriv_RTC_Status);
        }
        bool getStatus(PL031Deriv_RTC_Statuses status) const {
            return (getStatus() & static_cast<uint32_t>(status)) != 0;
        }
        
    };


};