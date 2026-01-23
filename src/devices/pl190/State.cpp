#include <nspire-utils/devices/PL190.hpp>

namespace ntls::devices {

    PL190VIC_State SavePL190State(const uintptr_t baseAddress) {
        PL190VIC_State state;

        state.intSelect = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptSelect);
        state.intEnable = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptEnable);
        state.softwareInt = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_SoftwareInt);
        state.protection = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_Protection);
        state.defaultVectAddr = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_DefaultVectAddr);

        for (size_t i = 0; i < 16; i++) {
            state.vectAddr[i] = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectAddrTable + i * 4);
            state.vectControl[i] = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectControlTable + i * 4);
        }

        return state;
    }

    void RestorePL190State(const uintptr_t baseAddress, const PL190VIC_State& state) {

        // disable all interrupts before restoring state
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptEnableClear) = 0xFFFFFFFF;
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_SoftwareIntClear) = 0xFFFFFFFF;

        // clear any pending interrupts
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectAddr) = 0;

        // restore configuration
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptSelect) = state.intSelect;
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_Protection) = state.protection;
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_DefaultVectAddr) = state.defaultVectAddr;

        // restore vector tables
        for (size_t i = 0; i < 16; i++) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectAddrTable + (i * sizeof(uint32_t))) = state.vectAddr[i];
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectControlTable + (i * sizeof(uint32_t))) = state.vectControl[i];
        }

        // restore software interrupts
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_SoftwareInt) = state.softwareInt;

        // re-enable interrupts
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptEnable) = state.intEnable;
    }
};