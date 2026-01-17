#include <nspire-utils/hook/Hooks.hpp>
#include <nspire-utils/platform/Caches.hpp>

extern "C" {
    extern uint32_t HookJump_template[];
    extern uint32_t EntryHookStub_template[];
    extern uint32_t ExitHookStub_template[];
    extern uint32_t Trampoline_template[];
}

namespace {

    void SyncICache(uintptr_t address, uintptr_t size) {
        uintptr_t start = address & ~(ntls::platform::CacheLineSize - 1);
        uintptr_t end = (address + size + ntls::platform::CacheLineSize - 1) & ~(ntls::platform::CacheLineSize - 1);
        ntls::platform::FlushDataCacheRange(start, end);
        ntls::platform::DrainWriteBuffer();
        ntls::platform::InvalidateICacheRange(start, end);
    }

    void ReadOriginalInstrutions(uintptr_t target_function, uint32_t* buffer, size_t count) {
        const uint32_t* func_ptr = reinterpret_cast<const uint32_t*>(target_function);
        for (size_t i = 0; i < count; i++) {
            buffer[i] = func_ptr[i];
        }
    }

    void BuildHookJump(uintptr_t hookStubAddress, uint32_t* outInstr) {
        outInstr[0] = HookJump_template[0];
        outInstr[1] = static_cast<uint32_t>(hookStubAddress);
    }
    void BuildEntryHookStub(void* user_data, uintptr_t hook_function, uintptr_t trampolineAddress, uint32_t* outInstr) {
        outInstr[0] = EntryHookStub_template[0];
        outInstr[1] = EntryHookStub_template[1];
        outInstr[2] = EntryHookStub_template[2];
        outInstr[3] = EntryHookStub_template[3];
        outInstr[4] = EntryHookStub_template[4];
        outInstr[5] = EntryHookStub_template[5];

        outInstr[6] = reinterpret_cast<uintptr_t>(user_data);
        outInstr[7] = static_cast<uintptr_t>(hook_function);
        outInstr[8] = static_cast<uintptr_t>(trampolineAddress);
    }
    void BuildExitHookStub(void* user_data, uintptr_t hook_function, uintptr_t trampolineAddress, uint32_t* outInstr) {
        outInstr[0] = ExitHookStub_template[0];
        outInstr[1] = ExitHookStub_template[1];
        outInstr[2] = ExitHookStub_template[2];
        outInstr[3] = ExitHookStub_template[3];
        outInstr[4] = ExitHookStub_template[4];
        outInstr[5] = ExitHookStub_template[5];
        outInstr[6] = ExitHookStub_template[6];
        outInstr[7] = ExitHookStub_template[7];
        outInstr[8] = ExitHookStub_template[8];
        outInstr[9] = ExitHookStub_template[9];

        outInstr[10] = reinterpret_cast<uintptr_t>(trampolineAddress);
        outInstr[11] = reinterpret_cast<uintptr_t>(user_data);
        outInstr[12] = static_cast<uintptr_t>(hook_function);
    }
    void BuildTrampoline(const uint32_t* saved_instructions, uintptr_t target_function, uint32_t* outInstr) {
        outInstr[0] = saved_instructions[0];
        outInstr[1] = saved_instructions[1];
        outInstr[2] = Trampoline_template[2];
        outInstr[3] = static_cast<uint32_t>(target_function + 8);
    }
}   


namespace ntls::hook {

    void HookFunctionEntry_ARM(EntryHookContext_ARM& context) {
        ReadOriginalInstrutions(context.target_function, context.saved_instructions, 2);
        BuildTrampoline(
            context.saved_instructions,
            context.target_function,
            context.trampoline
        );
        BuildEntryHookStub(
            context.user_data,
            context.hook_function,
            reinterpret_cast<uintptr_t>(context.trampoline),
            context.hookStub
        );
        BuildHookJump(
            reinterpret_cast<uintptr_t>(context.hookStub),
            reinterpret_cast<uint32_t*>(context.target_function)
        );

        SyncICache(context.target_function, 8);
        SyncICache(reinterpret_cast<uintptr_t>(context.hookStub), sizeof(uint32_t) * 9);
        SyncICache(reinterpret_cast<uintptr_t>(context.trampoline), sizeof(uint32_t) * 4);
    }

    void HookFunctionExit_ARM(ExitHookContext_ARM& context) {
        ReadOriginalInstrutions(context.target_function, context.saved_instructions, 2);
        BuildTrampoline(
            context.saved_instructions,
            context.target_function,
            context.trampoline
        );
        BuildExitHookStub(
            context.user_data,
            context.hook_function,
            reinterpret_cast<uintptr_t>(context.trampoline),
            context.hookStub
        );
        BuildHookJump(
            reinterpret_cast<uintptr_t>(context.hookStub),
            reinterpret_cast<uint32_t*>(context.target_function)
        );
        SyncICache(context.target_function, 8);
        SyncICache(reinterpret_cast<uintptr_t>(context.hookStub), sizeof(uint32_t) * 13);
        SyncICache(reinterpret_cast<uintptr_t>(context.trampoline), sizeof(uint32_t) * 4);
    }

    void UnhookFunction_ARM(uintptr_t target_function, const uint32_t* saved_instructions) {
        uint32_t* func_ptr = reinterpret_cast<uint32_t*>(target_function);
        func_ptr[0] = saved_instructions[0];
        func_ptr[1] = saved_instructions[1];
        SyncICache(target_function, 8);
    }
}