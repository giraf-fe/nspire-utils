#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::devices {
    // The device has 3 timers.
    // https://new.hackspire.org/Memory-mapped_IO_ports_on_CX_II/
    constexpr uintptr_t FastTimerBaseAddress  = 0x90010000;
    constexpr uintptr_t Timer1BaseAddress     = 0x900C0000;
    constexpr uintptr_t Timer2BaseAddress     = 0x900D0000;

    // Each timer has 2 timers within it.
    constexpr uintptr_t Timer1Offset          = 0x00;
    constexpr uintptr_t Timer2Offset          = 0x20;
    enum class SP804SelectedTimer {
        Timer1, Timer2
    };
    static inline uintptr_t GetSelectedTimerOffset(SP804SelectedTimer timer) {
        switch(timer) {
            case SP804SelectedTimer::Timer1:
                return Timer1Offset;
            case SP804SelectedTimer::Timer2:
                return Timer2Offset;
            default:
                return 0; // Should never happen
        }
    }
    
    // https://developer.arm.com/documentation/ddi0271/latest/
    constexpr uintptr_t TimerLoadOffset       = 0x00;
    constexpr uintptr_t TimerValueOffset      = 0x04;
    constexpr uintptr_t TimerControlOffset    = 0x08;
    constexpr uintptr_t TimerIntClrOffset     = 0x0C;
    constexpr uintptr_t TimerRISOffset        = 0x10;
    constexpr uintptr_t TimerMISOffset        = 0x14;
    constexpr uintptr_t TimerBGLoadOffset     = 0x18;

    constexpr uint8_t TimerControl_WrapBit             = (1 << 0); // 0 = wrap, 1 = one-shot
    constexpr uint8_t TimerControl_TimerSizeBit        = (1 << 1); // 0 = 16-bit, 1 = 32-bit
    constexpr uint8_t TimerControl_PrescaleBits        = (3 << 2); // 00 = div1, 01 = div16, 10 = div256, 11 = undefined
    constexpr uint8_t TimerControl_InterruptEnabledBit = (1 << 5); // 0 = disable, 1 = enable 
    constexpr uint8_t TimerControl_ModeBit             = (1 << 6); // 0 = free-running, 1 = periodic
    constexpr uint8_t TimerControl_TimerEnableBit      = (1 << 7); // 0 = disable, 1 = enable

    // Unique to ti nspire adjustable timers
    constexpr uintptr_t TimerSpeedControlOffset      = 0x80;
    constexpr uint8_t TimerSpeedControl_SpeedMask    = 0x3; // 0 = APB, 1 = 12 MHz, 2 = 32768 Hz
    constexpr uint8_t TimerSpeedControl_SpeedAPB     = 0;
    constexpr uint8_t TimerSpeedControl_Speed12MHz   = 1;
    constexpr uint8_t TimerSpeedControl_Speed32768Hz = 2;

    // Flags for TimerControl, some may be zero for self-documenting code.
    constexpr uint8_t TIMER_CTRL_ONE_SHOT      = TimerControl_WrapBit;
    constexpr uint8_t TIMER_CTRL_WRAP          = 0;

    constexpr uint8_t TIMER_CTRL_32BIT         = TimerControl_TimerSizeBit;
    constexpr uint8_t TIMER_CTRL_16BIT         = 0;

    constexpr uint8_t TIMER_CTRL_PRESCALER_DIV1   = 0;
    constexpr uint8_t TIMER_CTRL_PRESCALER_DIV16  = (1 << 2);
    constexpr uint8_t TIMER_CTRL_PRESCALER_DIV256 = (1 << 3);

    constexpr uint8_t TIMER_CTRL_INT_ENABLE    = TimerControl_InterruptEnabledBit;
    constexpr uint8_t TIMER_CTRL_INT_DISABLE   = 0;

    constexpr uint8_t TIMER_CTRL_PERIODIC      = TimerControl_ModeBit;
    constexpr uint8_t TIMER_CTRL_FREE_RUNNING  = 0;

    constexpr uint8_t TIMER_CTRL_ENABLE        = TimerControl_TimerEnableBit;
    constexpr uint8_t TIMER_CTRL_DISABLE       = 0;

    struct SP804Timer_Adjustable {
        uintptr_t baseAddress;
        SP804Timer_Adjustable(uintptr_t baseAddr) : baseAddress(baseAddr) {}

        void setLoadReg(SP804SelectedTimer timer, uint32_t value) {
            volatile uint32_t* loadReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerLoadOffset);
            *loadReg = value;
        }
        uint32_t getLoadReg(SP804SelectedTimer timer) {
            volatile uint32_t* loadReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerLoadOffset);
            return *loadReg;
        }

        uint32_t getCurrentValue32(SP804SelectedTimer timer) {
            volatile uint32_t* valueReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerValueOffset);
            return *valueReg;
        }
        uint16_t getCurrentValue16(SP804SelectedTimer timer) {
            volatile uint32_t* valueReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerValueOffset);
            return *valueReg & 0xFFFF;
        }

        void setControl(SP804SelectedTimer timer, uint8_t controlFlags) {
            volatile uint32_t* controlReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerControlOffset);
            *controlReg = controlFlags;
        }
        uint8_t getControl(SP804SelectedTimer timer) {
            volatile uint32_t* controlReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerControlOffset);
            return static_cast<uint8_t>(*controlReg);
        }

        void clearInterrupt(SP804SelectedTimer timer) {
            volatile uint32_t* intClrReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerIntClrOffset);
            *intClrReg = 1;
        }

        bool getRawInterruptStatus(SP804SelectedTimer timer) {
            volatile uint32_t* risReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerRISOffset);
            return (*risReg & 1) != 0;
        }
        bool getMaskedInterruptStatus(SP804SelectedTimer timer) {
            volatile uint32_t* misReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerMISOffset);
            return (*misReg & 1) != 0;
        }

        void setBackgroundLoadReg(SP804SelectedTimer timer, uint32_t value) {
            volatile uint32_t* bgLoadReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerBGLoadOffset);
            *bgLoadReg = value;
        }

        // Always returns 0 on the TI Nspire, despite the docs saying otherwise
        uint32_t getBackgroundLoadReg(SP804SelectedTimer timer) {
            volatile uint32_t* bgLoadReg = reinterpret_cast<volatile uint32_t*>(baseAddress + GetSelectedTimerOffset(timer) + TimerBGLoadOffset);
            return *bgLoadReg;
        }

        void setSpeed(uint8_t speed) {
            volatile uint32_t* speedControlReg = reinterpret_cast<volatile uint32_t*>(baseAddress + TimerSpeedControlOffset);
            *speedControlReg = (speed & TimerSpeedControl_SpeedMask);
        }

    };
};