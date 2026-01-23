#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::devices {
    constexpr uintptr_t SHA256_BaseAddress = 0xCC000000;
    
    constexpr uintptr_t SHA256_Control        = 0x00;
    constexpr uintptr_t SHA256_BusWriteAllow  = 0x08;
    constexpr uintptr_t SHA256_512bBlockStart = 0x10;
    constexpr uintptr_t SHA256_256bStateStart = 0x60;


    class SHA256_Device {
        uintptr_t baseAddress;
    public:
        SHA256_Device(uintptr_t baseAddr = SHA256_BaseAddress) : baseAddress(baseAddr) {}

        bool isBusy() {
            return (*reinterpret_cast<volatile uint32_t*>(baseAddress + SHA256_Control) & 0x1) != 0;
        }

        void initialize() {
            volatile uint32_t* ctrl_ptr = reinterpret_cast<volatile uint32_t*>(baseAddress + SHA256_Control);
            // write 0x10 then 0x0 to initialize
            *ctrl_ptr = 0x10;
            while(isBusy());
            *ctrl_ptr = 0x0;
        }

        bool WriteAllowed() {
            return (*reinterpret_cast<volatile uint32_t*>(baseAddress + SHA256_BusWriteAllow) & (1 << 8)) != 0;
        }
        void AllowWrite() {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + SHA256_BusWriteAllow) = (1 << 8);
        }

        void Write512bBlock(const uint8_t* block) {
            volatile uint8_t* block_ptr = reinterpret_cast<volatile uint8_t*>(baseAddress + SHA256_512bBlockStart);
            for (size_t i = 0; i < 64; i++) {
                block_ptr[i] = block[i];
            }
        }
        void Read512bBlock(uint8_t* block) {
            volatile uint8_t* block_ptr = reinterpret_cast<volatile uint8_t*>(baseAddress + SHA256_512bBlockStart);
            for (size_t i = 0; i < 64; i++) {
                block[i] = block_ptr[i];
            }
        }
        void Read256bState(uint8_t* state) {
            volatile uint8_t* state_ptr = reinterpret_cast<volatile uint8_t*>(baseAddress + SHA256_256bStateStart);
            for (size_t i = 0; i < 32; i++) {
                state[i] = state_ptr[i];
            }
        }
        
        void ProcessFirstBlock() {
            volatile uint32_t* ctrl_ptr = reinterpret_cast<volatile uint32_t*>(baseAddress + SHA256_Control);
            // write 0xA to process first block
            *ctrl_ptr = 0xA;
        }

        void ProcessSubsequentBlock() {
            volatile uint32_t* ctrl_ptr = reinterpret_cast<volatile uint32_t*>(baseAddress + SHA256_Control);
            // write 0xE to process subsequent block
            *ctrl_ptr = 0xE;
        }
    };
}