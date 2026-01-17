#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::hook {
    struct EntryHookContext_ARM {
        uintptr_t target_function;  // address of the target function
        uintptr_t hook_function;    // address of the hook function
        void* user_data;           // user data to pass to the hook

        // the first two instructions of the hooked function
        uint32_t saved_instructions[2];
        uint32_t hookStub[9]; // space for the hook stub
        uint32_t trampoline[4];  // space for the trampoline
    };
    struct ExitHookContext_ARM {
        uintptr_t target_function;  // address of the target function
        uintptr_t hook_function;    // address of the hook function
        void* user_data;           // user data to pass to the hook

        // the first two instructions of the hooked function
        uint32_t saved_instructions[2];
        uint32_t hookStub[13]; // space for the hook stub
        uint32_t trampoline[4];  // space for the trampoline
    };

    // Simple hook utility
    // More complicated functionality (intercepting parameters, etc.) would
    // need to be implemented manually.
    void HookFunctionEntry_ARM(EntryHookContext_ARM& context);
    void HookFunctionExit_ARM(ExitHookContext_ARM& context);

    void UnhookFunction_ARM(uintptr_t target_function, const uint32_t* saved_instructions);
};