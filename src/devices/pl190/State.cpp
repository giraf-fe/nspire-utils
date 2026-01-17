#include <nspire-utils/devices/PL190.hpp>

namespace ntls::devices {

    PL190VIC_State SavePL190State(const uintptr_t baseAddress) {
        PL190VIC_State state;

        state.intSelect = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICInterruptSelect);
        state.intEnable = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICInterruptEnable);
        state.softwareInt = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICSoftwareInt);
        state.protection = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICProtection);

        for (size_t i = 0; i < 16; i++) {
            state.vectAddr[i] = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICVectAddrTable + i * 4);
            state.vectControl[i] = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICVectControlTable + i * 4);
        }

        return state;
    }

    void RestorePL190State(const uintptr_t baseAddress, const PL190VIC_State& state) {

        // disable all interrupts before restoring state
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICInterruptEnableClear) = 0xFFFFFFFF;
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICSoftwareIntClear) = 0xFFFFFFFF;

        // restore configuration
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICInterruptSelect) = state.intSelect;
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICProtection) = state.protection;

        // restore vector tables
        for (size_t i = 0; i < 16; i++) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICVectAddrTable + (i * sizeof(uint32_t))) = state.vectAddr[i];
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICVectControlTable + (i * sizeof(uint32_t))) = state.vectControl[i];
        }

        // restore software interrupts
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICSoftwareInt) = state.softwareInt;

        // re-enable interrupts
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190_VICInterruptEnable) = state.intEnable;
    }
};