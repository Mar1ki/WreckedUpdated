#include "pch.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

void UKismetHookingLibrary::Hook(UHook* Hook, EHook Type)
{
    switch (Type)
    {
    case EveryVFT:
        {
            if (Hook->Original && !*Hook->Original)
            {
                for (int i = 0; i < UObject::GObjects->Num(); i++)
                {
                    auto Obj = UObject::GObjects->GetByIndex(i);
                    if (Obj && Obj->IsA(Hook->Class))
                    {
                        *Hook->Original = Obj->VTable[Hook->Address];
                        break; 
                    }
                }
            }

            for (int i = 0; i < UObject::GObjects->Num(); i++)
            {
                auto Obj = UObject::GObjects->GetByIndex(i);
                if (Obj && Obj->IsA(Hook->Class))
                {
                    DWORD oldProtect;
                    if (!VirtualProtect(&Obj->VTable[Hook->Address], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
                        continue; 

                    Obj->VTable[Hook->Address] = Hook->Detour;

                    DWORD temp;
                    VirtualProtect(&Obj->VTable[Hook->Address], sizeof(void*), oldProtect, &temp);
                }
            }
        }
        break;
    case Exec:
        {
            UFunction* Func = StaticFindObject<UFunction>(Hook->Path);
            if (Func)
            {
                if (Hook->Original)
                {
                    *Hook->Original = Func->ExecFunction;
                }

                Func->ExecFunction = reinterpret_cast<UFunction::FNativeFuncPtr>(Hook->Detour);

                UE_LOG(LogKismet, Verbose, "UKismetHookingLibrary::Hook | Exec: %s", Hook->Path.c_str());
            } else
            {
                UE_LOG(LogKismet, Fatal, "UKismetHookingLibrary::Hook | Exec: Failed to find function %s", Hook->Path.c_str());
            }
            break;
        }
    case Modify:
        {
            uint8_t *DetourAddr = (uint8_t *)Hook->Swap;
            uint8_t *instrAddr = (uint8_t *)Hook->Address;
            int64_t delta = (int64_t)(DetourAddr - (instrAddr + 5));
            auto addr = reinterpret_cast<int32_t *>(instrAddr + 1);
            DWORD dwProtection;
            VirtualProtect(addr, 4, PAGE_EXECUTE_READWRITE, &dwProtection);

            *addr = static_cast<int32_t>(delta);

            DWORD dwTemp;
            VirtualProtect(addr, 4, dwProtection, &dwTemp);
             
            MH_CreateHook((LPVOID)Hook->Swap, Hook->Detour, Hook->Original);
            MH_EnableHook((LPVOID)Hook->Swap);
            break;
        }
    case VFT:
        {
            UObject* DefaultObj = Hook->Class->DefaultObject;
            if (!DefaultObj)
            {
                UE_LOG(LogKismet, Fatal, "UKismetHookingLibrary::Hook | VFT: Invalid DefaultObj for class!");
                return;
            }

            if (Hook->Original)
            {
                *Hook->Original = DefaultObj->VTable[Hook->Address];
            }

            DWORD oldProtect;
            if (!VirtualProtect(&DefaultObj->VTable[Hook->Address], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
            {
                UE_LOG(LogKismet, Fatal, "UKismetHookingLibrary::Hook | VFT: VirtualProtect failed!");
                return;
            }

            DefaultObj->VTable[Hook->Address] = Hook->Detour;

            DWORD temp;
            VirtualProtect(&DefaultObj->VTable[Hook->Address], sizeof(void*), oldProtect, &temp);

            break;
        }
  case FTrue:
        {
            DWORD og;
            VirtualProtect(LPVOID(Hook->Address), sizeof(byte), PAGE_EXECUTE_READWRITE, &og);
            *(uint8_t*)Hook->Address = 0xE8;
            VirtualProtect(LPVOID(Hook->Address), sizeof(byte), og, &og);    
            break;
        }
    case Byte:
        {
            DWORD og;
            if (VirtualProtect(LPVOID(Hook->Address), sizeof(byte), PAGE_EXECUTE_READWRITE, &og))
            {
                *(uint8_t*)Hook->Address = Hook->Byte;
                VirtualProtect(LPVOID(Hook->Address), sizeof(byte), og, &og);
                UE_LOG(LogKismet, Log, "VirtualProtect successful for Byte at address: 0x%p", Hook->Address);
            }
            else
            {
                UE_LOG(LogKismet, Error, "VirtualProtect failed for Byte at address: 0x%p", Hook->Address);
            }
            break;
        }
    case RTrue:
        {
            MH_CreateHook(LPVOID(Hook->Address), RetTrue, nullptr);
            MH_EnableHook(LPVOID(Hook->Address));
            break;
        }
    case RFalse:
        {
            MH_CreateHook(LPVOID(Hook->Address), RetFalse, nullptr);
            MH_EnableHook(LPVOID(Hook->Address));
            break;
        }
    case Address:
        if (Hook->Address == 0x0) return;
        MH_CreateHook(LPVOID(Hook->Address), Hook->Detour, Hook->Original);
        MH_EnableHook(LPVOID(Hook->Address));
        break;

    case Interface:
        if (Hook->Interface == nullptr)
            return;

        for (int i = 0; i < UObject::GObjects->Num(); i++)
        {
            auto Obj = UObject::GObjects->GetByIndex(i);
            
            if (!Obj || !Obj->IsDefaultObject() || !Obj->IsA(Hook->Class))
                continue;

            auto InterfaceAddress = Obj->GetInterface<void>(Hook->Interface);
            if (!InterfaceAddress)
                continue;

            auto VTable = (*(void***)InterfaceAddress);
            if (!VTable)
                continue;

            if (!VTable[Hook->Address])
                continue;

            if (Hook->Original && !*Hook->Original)
            {
                *Hook->Original = VTable[Hook->Address];
            }

            DWORD oldProtect;
            if (!VirtualProtect(&VTable[Hook->Address], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
                continue;

            VTable[Hook->Address] = Hook->Detour;

            DWORD temp;
            VirtualProtect(&VTable[Hook->Address], sizeof(void*), oldProtect, &temp);
        }
        break;
    default:
        break;
    }

    Hook->Address = 0x0;
    Hook->Class = nullptr;
    Hook->Path = "";
    Hook->Original = nullptr;
    Hook->Detour = nullptr;
    Hook->Swap = 0x0;
}