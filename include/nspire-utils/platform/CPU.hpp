#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::platform {

    static inline void IRQEnable() {
        uint32_t cpsr;
        __asm__ volatile ("mrs %0, cpsr" : "=r"(cpsr));
        cpsr &= ~(1u << 7);                 // clear I
        __asm__ volatile ("msr cpsr_c, %0" :: "r"(cpsr) : "memory");
    }
    static inline void IRQDisable() {
        uint32_t cpsr;
        __asm__ volatile ("mrs %0, cpsr" : "=r"(cpsr));
        cpsr |= (1u << 7);                  // set I
        __asm__ volatile ("msr cpsr_c, %0" :: "r"(cpsr) : "memory");
    }

    static inline bool IRQSave() {
        uint32_t cpsr;
        __asm__ volatile ("mrs %0, cpsr" : "=r"(cpsr));
        return (cpsr & (1u << 7)) == 0;     // return previous state of I
    }
    static inline void IRQRestore(bool enabled) {
        if (enabled) {
            IRQEnable();
        } else {
            IRQDisable();
        }
    }

    static inline void WaitForInterrupt() {
        uint32_t zero = 0;
        __asm__ volatile (
            "mcr p15, 0, %0, c7, c0, 4\n"   // Wait For Interrupt
            :
            : "r"(zero)
            : "memory"
        );
    }

    struct CPU_State {
        uint32_t cpsr;
        uint32_t sp_irq;
        uint32_t sp_fiq;
        uint32_t spsr_irq;
        uint32_t spsr_fiq;
        uint32_t spsr_svc;
    };

    CPU_State SaveCPUState() {
        CPU_State state;
        uint32_t original_cpsr;
        uint32_t mode_cpsr;
        
        // Save current CPSR
        asm volatile("mrs %0, cpsr" : "=r"(original_cpsr));
        state.cpsr = original_cpsr;
        
        // Switch to IRQ mode (0x12) with interrupts disabled (0xC0)
        mode_cpsr = (original_cpsr & ~0x1F) | 0x12 | 0xC0;
        asm volatile(
            "msr cpsr_c, %[mode]\n\t"
            "mov %[sp], sp\n\t"
            "mrs %[spsr], spsr"
            : [sp] "=r"(state.sp_irq), [spsr] "=r"(state.spsr_irq)
            : [mode] "r"(mode_cpsr)
            : "memory"
        );
        
        // Switch to FIQ mode (0x11) with interrupts disabled
        mode_cpsr = (original_cpsr & ~0x1F) | 0x11 | 0xC0;
        asm volatile(
            "msr cpsr_c, %[mode]\n\t"
            "mov %[sp], sp\n\t"
            "mrs %[spsr], spsr"
            : [sp] "=r"(state.sp_fiq), [spsr] "=r"(state.spsr_fiq)
            : [mode] "r"(mode_cpsr)
            : "memory"
        );
        
        // Switch to SVC mode (0x13) with interrupts disabled
        mode_cpsr = (original_cpsr & ~0x1F) | 0x13 | 0xC0;
        asm volatile(
            "msr cpsr_c, %[mode]\n\t"
            "mrs %[spsr], spsr"
            : [spsr] "=r"(state.spsr_svc)
            : [mode] "r"(mode_cpsr)
            : "memory"
        );
        
        // Restore original CPSR (return to original mode)
        asm volatile("msr cpsr_c, %0" : : "r"(original_cpsr) : "memory");
        
        return state;
    }

    void RestoreCPUState(const CPU_State& state) {
        uint32_t current_cpsr;
        uint32_t mode_cpsr;
        
        // Get current CPSR
        asm volatile("mrs %0, cpsr" : "=r"(current_cpsr));
        
        // Switch to IRQ mode and restore SP_IRQ and SPSR_IRQ
        mode_cpsr = (current_cpsr & ~0x1F) | 0x12 | 0xC0;
        asm volatile(
            "msr cpsr_c, %[mode]\n\t"
            "mov sp, %[sp]\n\t"
            "msr spsr_cxsf, %[spsr]"
            :
            : [mode] "r"(mode_cpsr), [sp] "r"(state.sp_irq), [spsr] "r"(state.spsr_irq)
            : "memory"
        );
        
        // Switch to FIQ mode and restore SP_FIQ and SPSR_FIQ
        mode_cpsr = (current_cpsr & ~0x1F) | 0x11 | 0xC0;
        asm volatile(
            "msr cpsr_c, %[mode]\n\t"
            "mov sp, %[sp]\n\t"
            "msr spsr_cxsf, %[spsr]"
            :
            : [mode] "r"(mode_cpsr), [sp] "r"(state.sp_fiq), [spsr] "r"(state.spsr_fiq)
            : "memory"
        );
        
        // Switch to SVC mode and restore SP_SVC and SPSR_SVC
        mode_cpsr = (current_cpsr & ~0x1F) | 0x13 | 0xC0;
        asm volatile(
            "msr cpsr_c, %[mode]\n\t"
            "msr spsr_cxsf, %[spsr]"
            :
            : [mode] "r"(mode_cpsr), [spsr] "r"(state.spsr_svc)
            : "memory"
        );
        
        // Restore saved CPSR (restores mode, flags, and interrupt state)
        asm volatile("msr cpsr_cxsf, %0" : : "r"(state.cpsr) : "memory");
    }

    // RAII helper to save/restore CPU state
    class CPU_State_Checkpoint {
        CPU_State state;
    public:
        CPU_State_Checkpoint() : state(SaveCPUState()) {}
        ~CPU_State_Checkpoint() {
            RestoreCPUState(state);
        }
    };

    enum class CPU_Mode : uint32_t {
        FIQ        = 0x11,
        IRQ        = 0x12,
        Supervisor = 0x13,
        Abort      = 0x17,
        Undefined  = 0x1B,
        System     = 0x1F
    };

    static inline uintptr_t GetBankedSP(CPU_Mode target_mode) {
        uintptr_t sp_value;
        uint32_t original_cpsr;
        uint32_t temp_cpsr;

        asm volatile (
            "mrs %0, cpsr \n\t"         // save original cpsr
            "bic %1, %0, #0x1F \n\t"       // clear mode bits
            "orr %1, %1, %2 \n\t"       // set target mode bits
            "orr %1, %1, #0xC0 \n\t"     // disable FIQ and IRQ

            "msr cpsr_c, %1 \n\t"       // switch to target mode
            "mov %3, sp \n\t"           // get banked sp
            "msr cpsr_c, %0 \n\t"       // switch back
            : "=&r" (original_cpsr), "=&r" (temp_cpsr), "=&r" (sp_value)
            : "r" (target_mode)
            : "memory", "cc"
        );

        return sp_value;
    }

    static inline void SetBankedSP(CPU_Mode target_mode, uintptr_t sp_value) {
        uint32_t original_cpsr;
        uint32_t temp_cpsr;

        asm volatile (
            "mrs %0, cpsr \n\t"         // save original cpsr
            "bic %1, %0, #0x1F \n\t"       // clear mode bits
            "orr %1, %1, %2 \n\t"       // set target mode bits
            "orr %1, %1, #0xC0 \n\t"     // disable FIQ and IRQ

            "msr cpsr_c, %1 \n\t"       // switch to target mode
            "mov sp, %3 \n\t"           // set banked sp
            "msr cpsr_c, %0 \n\t"       // switch back
            : "=&r" (original_cpsr), "=&r" (temp_cpsr)
            : "r" (target_mode), "r" (sp_value)
            : "memory", "cc"
        );
    }

} // namespace ntls::platform