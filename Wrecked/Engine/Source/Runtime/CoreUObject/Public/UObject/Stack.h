#pragma once
#include "pch.h"

class FOutputDevice
{
public:
    bool bSuppressEventTag;
    bool bAutoEmitLineTerminator;
    uint8_t _Padding1[0x6];
};

struct FOutParmRec
{
    FProperty* Property;
    uint8* PropAddr;
    FOutParmRec* NextOutParm;
};

class FFrame : public FOutputDevice
{
public:
    void** VTable;
    UFunction* Node;
    UObject* Object;
    uint8* Code;
    uint8* Locals;
    FProperty* MostRecentProperty;
    uint8_t* MostRecentPropertyAddress;
    uint8* MostRecentPropertyContainer;
    TArray<void*> FlowStack;
    FFrame* PreviousFrame;
    FOutParmRec* OutParms;
    uint8_t _Padding1[0x20]; // wtf else do they store here
    FField* PropertyChainForCompiledIn;
    UFunction* CurrentNativeFunction;
    FFrame* PreviousTrackingFrame;
    bool bArrayContextFailed;

public:
    void StepExplicitProperty(void* const Result, FProperty* Property)
    {
        if (!Result)
            return;

        if (Property->PropertyFlags & 0x100LL)
        {
            FOutParmRec* Out = OutParms;
            while (Out->Property != Property)
            {
                Out = Out->NextOutParm;
            }
            MostRecentPropertyAddress = Out->PropAddr;
            MostRecentPropertyContainer = nullptr;
        }
        else
        {
            MostRecentPropertyAddress = (uint8_t*)(uint32(Locals) + Property->Offset);
            MostRecentPropertyContainer = Locals;
        }
    }
    void StepCompiledIn(void* const Result, bool ForceExplicitProp = false) {
        if (Code && !ForceExplicitProp)
        {
            static void(__fastcall * FFrameStep) (FFrame*, UObject*, void* const) = decltype(FFrameStep) (InSDKUtils::GetImageBase() + 0x18E737C);
            FFrameStep(this, Object, Result);
        }
        else
        {
            FField* _Prop = PropertyChainForCompiledIn;
            PropertyChainForCompiledIn = _Prop->Next;
            static void(__fastcall * FFrameStepExplicit)(FFrame*, void* const, FField*) = decltype(FFrameStepExplicit)(InSDKUtils::GetImageBase() + 0x18E40F0);
            FFrameStepExplicit(this, Result, _Prop);
        }
    }
    void* StepCompiledInRefInternal(void* _Tm)
    {
        MostRecentPropertyAddress = nullptr;
        MostRecentPropertyContainer = nullptr;

        if (Code)
        {
            static void(__fastcall * FFrameStep) (FFrame*, UObject*, void* const) = decltype(FFrameStep) (InSDKUtils::GetImageBase() + 0x18E737C);
            FFrameStep(this, Object, _Tm);
        }
        else
        {
            FField* _Prop = PropertyChainForCompiledIn;
            PropertyChainForCompiledIn = _Prop->Next;
            static void(__fastcall * FFrameStepExplicit)(FFrame*, void* const, FField*) = decltype(FFrameStepExplicit)(InSDKUtils::GetImageBase() + 0x18E40F0);
            FFrameStepExplicit(this, _Tm, _Prop);
        }

        return MostRecentPropertyAddress ? MostRecentPropertyAddress : _Tm;
    }

    template <typename T>
    T& StepCompiledInRef()
    {
        T TempVal{};
        return *(T*)StepCompiledInRefInternal(&TempVal);
    }

    void IncrementCode() {
        Code = (uint8_t*)(__int64(Code) + (bool)Code);
    }
};