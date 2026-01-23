#include <os.h>
#include <nspireio/console.hpp>

#include <nspire-utils/devices/PL190.hpp> // For interrupt controller
#include <nspire-utils/mem/Alloc.hpp> // For aligned allocations
#include <nspire-utils/platform/Caches.hpp> // For cache management
#include <nspire-utils/platform/EVT.hpp> // For exception vector table
#include <nspire-utils/platform/MMUEditor.hpp> // For MMU management

#include <nspire-utils/platform/CPU.hpp> // For interrupt stuff
#include <nspire-utils/devices/SP804.hpp> // For timer (example device)

#include <cstring>
#include <functional>

nio::console csl;

// the instruction to set the PC to a specific address (0x100 bytes ahead)
extern "C" uint32_t exception_vector_jump[]; // Defined in templates.S
extern "C" void handleIRQ();
extern "C" void Timer_ISR(); // does reenable interrupts, requires hardware interrupt priority in the VIC
extern "C" void Timer_ISR2(); // does not reenable interrupts, will never nest. required for emulator

extern "C" void HandleTimer1() {
    csl << "Timer 1 Interrupt Triggered!\n";
    // Acknowledge the interrupt
    ntls::devices::SP804Timer_Adjustable timer(ntls::devices::Timer1BaseAddress);
    timer.clearInterrupt(ntls::devices::SP804SelectedTimer::Timer1);
}

void interrupts(ntls::devices::PL190VIC_Editor& vicEditor) {
    csl << "Setting up timer interrupt handler...\n";
    // setup the IRQ vector to jump to our handler
    *reinterpret_cast<uint32_t*>(
        static_cast<uintptr_t>(ntls::platform::EVT_Position::High) + 0x100 + 
        static_cast<uintptr_t>(ntls::platform::EVT_Vector::IRQ)
    )
        = reinterpret_cast<uint32_t>(&handleIRQ);
    ntls::platform::SetEVTValue(
        ntls::platform::EVT_Vector::IRQ,
        ntls::platform::EVT_Position::High,
        exception_vector_jump[0]
    );

    // register device in the vic (use timer for example)
    // setup timer interrupt
    ntls::devices::SP804Timer_Adjustable timer(ntls::devices::Timer1BaseAddress);
    timer.clearInterrupt(ntls::devices::SP804SelectedTimer::Timer1);
    timer.setControl(
        ntls::devices::SP804SelectedTimer::Timer1,
        ntls::devices::TIMER_CTRL_WRAP | ntls::devices::TIMER_CTRL_32BIT |
        ntls::devices::TIMER_CTRL_PRESCALER_DIV1 |
        ntls::devices::TIMER_CTRL_INT_ENABLE |
        ntls::devices::TIMER_CTRL_PERIODIC |
        ntls::devices::TIMER_CTRL_DISABLE // start disabled
    );
    timer.setSpeed(ntls::devices::TimerSpeedControl_Speed12MHz);
    timer.setLoadReg(ntls::devices::SP804SelectedTimer::Timer1, 12000000); // one second
    timer.setBackgroundLoadReg(ntls::devices::SP804SelectedTimer::Timer1, 12000000);
    timer.setControl(ntls::devices::SP804SelectedTimer::Timer1,
        timer.getControl(ntls::devices::SP804SelectedTimer::Timer1) |
        ntls::devices::TIMER_CTRL_ENABLE
    );

    // add to vic
    vicEditor.setVectorControl(0,
        ntls::devices::PL190VIC_VectControlEnabledBit | 
        static_cast<uint8_t>(ntls::devices::TiNspireInterruptSource::FirstTimer)
    );
    vicEditor.setVectorAddress(0,
        reinterpret_cast<uint32_t>(&Timer_ISR2)
    );
    vicEditor.enableInterrupt(
        static_cast<uint8_t>(ntls::devices::TiNspireInterruptSource::FirstTimer)
    );
    
    ntls::platform::FlushDataCacheRange(
        static_cast<uintptr_t>(ntls::platform::EVT_Position::High),
        static_cast<uintptr_t>(ntls::platform::EVT_Position::High) + 0x1000
    );
    ntls::platform::DrainWriteBuffer();
    ntls::platform::InvalidateBothCaches();

    csl << "Timer setup! loop idle on main \n";
    bool irqState = ntls::platform::IRQSave();
    ntls::platform::IRQEnable();
    int i = 0;
    while(i < 5) {
        ntls::platform::WaitForInterrupt();
        csl << "Woke from interrupt...  i=" << ++i << "\n";
    }
    ntls::platform::IRQRestore(irqState);

    csl << "Exiting interrupt test loop...\n";
    vicEditor.disableInterrupt(
        static_cast<uint8_t>(ntls::devices::TiNspireInterruptSource::FirstTimer)
    );
    timer.setControl(ntls::devices::SP804SelectedTimer::Timer1,
        ntls::devices::TIMER_CTRL_DISABLE | ntls::devices::TIMER_CTRL_INT_DISABLE
    );
}


int main() {
    ntls::platform::CPU_State_Checkpoint ckpt;
    csl << "Goal: Disable all interrupts and move the vector table high.\n";

    // steps:
    // 1. Disable interrupts in the interrupt controller
    // 2. Allocate a 4kB aligned block of memory for the new vector table
    // 3. Allocate another block for the irq stack
    // 4. mmu map the block to 0xFFFF0000
    // 5. move evt high

    csl << "1. Disabling interrupts...\n";
    ntls::devices::PL190VIC_Editor vicEditor(ntls::devices::PL190_BaseAddress);
    vicEditor.clear();

    csl << "2. Allocating memory for new vector table...\n";
    constexpr size_t vectorTableAlignment = 0x1000; // 4kB
    void* newVectorTable = ntls::mem::aligned_alloc(vectorTableAlignment, vectorTableAlignment);
    if (!newVectorTable) {
        csl << "Failed to allocate memory for new vector table!\n";
        return 1;
    }

    csl << "3. Allocating memory for irq stack...\n";
    constexpr size_t irqStackAlignment = 0x1000; // 4kB
    void* irqStack = ntls::mem::aligned_alloc(irqStackAlignment, irqStackAlignment);
    if (!irqStack) {
        csl << "Failed to allocate memory for irq stack!\n";
        return 1;
    }
    // set irq sp
    ntls::platform::SetBankedSP(
        ntls::platform::CPU_Mode::IRQ,
        reinterpret_cast<uintptr_t>(irqStack) + irqStackAlignment
    );


    // zero out the new vector table
    std::memset(newVectorTable, 0, vectorTableAlignment);
    // flush the cache to ensure the new table is written to RAM
    ntls::platform::FlushDataCacheRange(
        reinterpret_cast<uintptr_t>(newVectorTable),
        reinterpret_cast<uintptr_t>(newVectorTable) + vectorTableAlignment
    );
    ntls::platform::DrainWriteBuffer();

    csl << "4. Mapping new vector table to 0xFFFF0000...\n";
    ntls::platform::MMUEditor mmuEditor;
    ntls::platform::MMUL2CoarsePageTableEditor coarseEditor(mmuEditor, 0xFFFF0000,
        /*domain=*/ntls::platform::MMUDomainFromEntry(mmuEditor.getEntry(0x0))); // domain of existing mapping at 0x0

    coarseEditor.Map4KBPage(0xFFFF0000, reinterpret_cast<uintptr_t>(newVectorTable),
        ntls::platform::MMUL2EntirePageAccessPermissions(ntls::platform::MMUAccessPermission::FullAccess) |
        static_cast<uint32_t>(ntls::platform::MMUCachePolicy::NonCachedNonBuffered)
    );

    csl << "5. Moving vector table high...\n";
    ntls::platform::SetEVTPosition(ntls::platform::EVT_Position::High);

    csl << "All done!\n";
    csl << "You can now set up your own interrupt handlers at the new vector table location (0xFFFF0000).\n";

    // do something with the new vector table here...
    interrupts(vicEditor);

    csl << "Cleaning up...\n";

    ntls::platform::SetEVTPosition(ntls::platform::EVT_Position::Low);
    ntls::mem::aligned_free(newVectorTable);
    ntls::mem::aligned_free(irqStack);

    csl.get();

    return 0;
}
