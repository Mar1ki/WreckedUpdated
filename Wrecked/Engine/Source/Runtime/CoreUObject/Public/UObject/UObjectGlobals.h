#pragma once
#include "pch.h"

template <typename T>
inline T* StaticFindObject(const UEAllocatedString Path, UClass* Class = UObject::StaticClass(), UObject* Package = nullptr) {
    static auto fn = reinterpret_cast<
        SDK::UObject* (*)(SDK::UClass*, SDK::UObject*, const wchar_t*, bool)
    >(
        ImageBase + 0x146D58C
    );
    if (!fn) UE_LOG(LogTemp, Fatal, "Failed to find StaticFindObject");
    
    UEAllocatedWString widePath(Path.begin(), Path.end());
    SDK::UObject* obj = fn ? fn(Class, Package, widePath.c_str(), false) : nullptr;
    return reinterpret_cast<T*>(obj);
}

template<typename T>
T* StaticLoadObject(const UEAllocatedString name) {
    static auto fn = (SDK::UObject* (*)(SDK::UClass*, SDK::UObject*, const TCHAR*, const TCHAR*, uint32_t, SDK::UObject*, bool, void*)) (uint64_t(ImageBase + 0x1E9B2D0));
    T* Object = StaticFindObject<T>(name);
    UEAllocatedWString wideName(name.begin(), name.end());
    UObject* BaseObject = fn(T::StaticClass(), nullptr, wideName.c_str(), nullptr, 0, nullptr, false, nullptr);
    Object = static_cast<T*>(BaseObject);
    return Object;
}

template<typename T>
T* StaticLoadObject(const UEAllocatedString name, UClass* ClassToLoad) {
    static auto fn = (SDK::UObject* (*)(SDK::UClass*, SDK::UObject*, const TCHAR*, const TCHAR*, uint32_t, SDK::UObject*, bool, void*)) (uint64_t(ImageBase + 0x1E9B2D0));
    T* Object = nullptr;
    UEAllocatedWString wideName(name.begin(), name.end());
    UObject* BaseObject = fn(ClassToLoad, nullptr, wideName.c_str(), nullptr, 0, nullptr, false, nullptr);
    Object = static_cast<T*>(BaseObject);
    return Object;
}

inline UObject* StaticLoadObjectNoType(const UEAllocatedString name, UClass* ClassToLoad) {
    static auto fn = (SDK::UObject* (*)(SDK::UClass*, SDK::UObject*, const TCHAR*, const TCHAR*, uint32_t, SDK::UObject*, bool, void*)) (uint64_t(ImageBase + 0x1E9B2D0));
    UObject* Object = StaticFindObject<UObject>(name);
    UEAllocatedWString wideName(name.begin(), name.end());
    UObject* BaseObject = fn(ClassToLoad, nullptr, wideName.c_str(), nullptr, 0, nullptr, false, nullptr);
    Object = BaseObject;
    return Object;
}

namespace SDK
{
    inline UObject *InternalObjectGet(FSoftObjectPtr *Ptr, UClass *Class)
    {
        if (!Ptr)
            return nullptr;
        auto Ret = Ptr->WeakPtr.ObjectIndex && Ptr->WeakPtr.ObjectSerialNumber ? Ptr->Get() : nullptr;
        if ((!Ret || !Ret->IsA(Class)) && Ptr->ObjectID.AssetPath.AssetName.ComparisonIndex > 0)
        {
            auto Path = Ptr->ObjectID.AssetPath.PackageName.GetRawString();
            if (Ptr->ObjectID.AssetPath.AssetName)
            {
                Path += "." + Ptr->ObjectID.AssetPath.AssetName.GetRawString();
            }
            Ret = StaticLoadObjectNoType(Path, Class);
        }
        return Ret;
    }
}