/*
    Plugin-SDK (Grand Theft Auto) SHARED source file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma warning(disable: 4311)
#pragma warning(disable: 4302)

#include "DynAddress.h"
#include <windows.h>
#include "Base.h"
#if !(defined (_M_IX86) || defined (_X86_))
#include "..\modutils\Trampoline.h"
#endif

uintptr_t _NOINLINE_ plugin::GetBaseAddress() {
    static uintptr_t addr = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));
    return addr;
}

uintptr_t _NOINLINE_ plugin::GetGlobalAddress(uintptr_t address) {
#ifdef GTA2
    // GTA2 executables have relocations stripped and plugin_II addresses are
    // fixed virtual addresses. Version 11.44 changes the preferred image base
    // from 0x400000 to 0x3F0000 while preserving the original code and data at
    // their old absolute addresses, so applying a module-base delta points all
    // hooks 0x10000 too low.
    return address;
#else
    return GetBaseAddress() - STARTING_ADDRESS + address;
#endif
}

const uintptr_t _NOINLINE_ plugin::GetExternalAddress(const char* processName, uintptr_t shift, uintptr_t address) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(GetModuleHandleA(processName));
#ifdef GTA2
    // External GTA2 module addresses must only account for that module's
    // actual load base. The main gta2.exe image-base difference in 11.44 is
    // unrelated and would move d3ddll.dll pointers 0x10000 too low.
    return addr - shift + address;
#else
    return (GetBaseAddress() - STARTING_ADDRESS) + (addr - shift + address);
#endif
}

#if !(defined (_M_IX86) || defined (_X86_))
void* plugin::MakeTrampoline(uintptr_t address, void* func) {
    auto trampoline = Trampoline::MakeTrampoline(address);
    return trampoline->Jump(func);
}
#endif
