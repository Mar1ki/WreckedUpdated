#include "pch.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortPoiVolume.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void VHook1(void* Base, uintptr_t Index, void* Detour, void** Original = nullptr) {
    auto VTable = (*(void***)Base);
    if (!VTable)
        return;

    if (!VTable[Index])
        return;

    if (Original)
        *Original = VTable[Index];

    DWORD dwOld;
    VirtualProtect(&VTable[Index], 8, PAGE_EXECUTE_READWRITE, &dwOld);
    VTable[Index] = Detour;
    DWORD dwTemp;
    VirtualProtect(&VTable[Index], 8, dwOld, &dwTemp);
}

std::map<std::string, FVector> PoiTagToLocation = {
    {
        "Athena.Location.POI.Megalodon",
        FVector(-108949.0, -47133.0, 0)
    },
    {
        "Athena.Location.POI.NitroDome",
        FVector(-45298.4, -14526.2, 0)
    },
    {
        "Athena.Location.POI.AbandonedRuins",
        FVector(-974.2, -16046.1,0)
    },
    {
        "Athena.Location.POI.SocietyEstate",
        FVector(-70165.1, -12091.7, 0)
    },
    {
        "Athena.Location.POI.RiverCapital",
        FVector(-20657.7, 48529.1, 0)
    },
    {
        "Athena.Location.POI.Arena",
        FVector(-94259.9, 58210.9, 0)
    },
    {
        "Athena.Location.POI.RuralVillage",
        FVector(-20443.5, -69527.8, 0)
    },
    {
        "Athena.Location.POI.Hades",
        FVector(50288.9, -86553.2, 0)
    },
    {
        "Athena.Location.POI.MountOlympus",
        FVector(-59859.1, 67790.9, 0)
    },
    {
        "Athena.Location.POI.CostalCity",
        FVector(-78215.6, -74551.9, 0)
    },
    {
        "Athena.Location.POI.GrandGlacierHotel",
        FVector(-24712.2, 98868.8, 0)
    },
    {
        "Athena.Location.POI.Gate",
        FVector(21121.7, -56121.5, 0)
    },
    {
        "Athena.Location.POI.TennisClub",
        FVector(88908.3, 48183.2, 0)
    },
    {
        "Athena.Location.POI.AbandonedMansion",
        FVector(90746.5, -61949.4, 0)
    },
    {
        "Athena.Location.POI.NitroTown",
        FVector(-99926.3, 18619.4, 0)
    },

};

UBodySetup* (*GetBodySetupOG)(UBrushComponent* Comp);
UBodySetup* GetBodySetup(UBrushComponent* Comp) {
    if (!Comp)
        return nullptr;
    return Comp->BrushBodySetup;
}

void FortPoiVolume::PostInitializeComponents(AFortPoiVolume* Volume)
{
    Volume->BrushComponent = (UBrushComponent*)UGameplayStatics::SpawnObject(UBrushComponent::StaticClass(), Volume);
    Volume->K2_AttachToComponent(Volume->BrushComponent, FName(0), EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
    static void (*SetCollisonProfileName)(UBrushComponent*, __int64*, __int64*) =decltype(SetCollisonProfileName)(Volume->BrushComponent->VTable[(0x738 / 8)]);

    ((void (*)(UObject * Component, UObject * World))(ImageBase + 0x1697594))(Volume->BrushComponent, UWorld::GetWorld());
    SetCollisonProfileName(Volume->BrushComponent, reinterpret_cast<__int64*>(ImageBase + 0x12B9C7C4), (__int64*)1);
    Volume->BrushComponent->bGenerateOverlapEvents = true;
    Volume->BrushComponent->SetGenerateOverlapEvents(true);
   
    UBodySetup* BodySetup = (UBodySetup*)UGameplayStatics::SpawnObject(UBodySetup::StaticClass(), Volume->BrushComponent);
    BodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
    BodySetup->bGenerateMirroredCollision = false;
    BodySetup->bDoubleSidedGeometry = true;
    Volume->BrushComponent->Brush = Volume->Brush;
    Volume->BrushComponent->BrushBodySetup = BodySetup;
    VHook1(Volume->BrushComponent, 0x7B0 / 8, GetBodySetup, (void**)&GetBodySetupOG);
    if (Volume->Brush)
    {
        auto Points = Volume->Brush->Points;
        FVector3f MinPt = Points[0];
        FVector3f MaxPt = Points[0];
        for (int i = 1; i < Points.Num(); i++)
        {
            FVector3f pt = Points[i];
            MinPt.X = pt.X < MinPt.X ? pt.X : MinPt.X;
            MinPt.Y = pt.Y < MinPt.Y ? pt.Y : MinPt.Y;
            MinPt.Z = pt.Z < MinPt.Z ? pt.Z : MinPt.Z;
            MaxPt.X = pt.X > MaxPt.X ? pt.X : MaxPt.X;
            MaxPt.Y = pt.Y > MaxPt.Y ? pt.Y : MaxPt.Y;
            MaxPt.Z = pt.Z > MaxPt.Z ? pt.Z : MaxPt.Z;
        }

        FVector3f Center = (MinPt + MaxPt) * 0.5f;
        FVector3f Extent = (MaxPt - MinPt) * 0.5f;
        auto Tag = Volume->LocationTags.GameplayTags[0];
        auto Value = PoiTagToLocation.find(Tag.TagName.ToString().c_str());
        if (Value._Ptr)
        {
            Center.X += Value->second.X;
            Center.Y += Value->second.Y;
            Center.Z += Value->second.Z;
        }
        UE_LOG(LogTemp, Log, "Center(%f, %f, %f)", Center.X, Center.Y, Center.Z);
        UE_LOG(LogTemp, Log, "Extent(%f, %f, %f)", Extent.X, Extent.Y, Extent.Z);
        Volume->BrushComponent->Bounds.Origin.X = Center.X;
        Volume->BrushComponent->Bounds.Origin.Y = Center.Y;
        Volume->BrushComponent->Bounds.Origin.Z = Center.Z;
        Volume->BrushComponent->Bounds.BoxExtent.X = Extent.X;
        Volume->BrushComponent->Bounds.BoxExtent.Y = Extent.Y;
    }
    PostInitializeComponentsOG(Volume);
    VHook1(Volume->BrushComponent, 0x7B0 / 8, GetBodySetupOG);
}

void FortPoiVolume::SendComplexCustomStatEvent(UFortQuestManager* QuestManager, UObject* TargetObject, FGameplayTagContainer& AdditionalSourceTags, FGameplayTagContainer& TargetTags, int32 Count)
{
    std::string AdditionalTagsStr = "";
    std::string TargetTagsStr = "";
    for (auto Tag : AdditionalSourceTags.GameplayTags)
        AdditionalTagsStr += Tag.TagName.ToString().c_str() +std::string(" ");
    for (auto Tag : TargetTags.GameplayTags)
        TargetTagsStr += Tag.TagName.ToString().c_str() + std::string(" ");

  //  UE_LOG(LogTemp, Log, "Quest Stuff: TargetObj: %s; AdditionalTags: %s; TargetTags: %s; Count: %d", TargetObject ? TargetObject->GetName().c_str() : "None", AdditionalTagsStr.c_str(), TargetTagsStr.c_str(), Count);
    SendComplexCustomStatEventOG(QuestManager, TargetObject, AdditionalSourceTags, TargetTags, Count);
}

void FortPoiVolume::ProcessLocationTagDeltaForController(AFortPoiManager* Manager, AFortPlayerControllerGameplay* Controller, FGameplayTagContainer* TagsToRemove, FGameplayTagContainer* TagsToAdd) {
    if (TagsToAdd && TagsToAdd->GameplayTags.Num() > 0 || TagsToAdd->ParentTags.Num() > 0) {
        auto Tag = TagsToAdd->GameplayTags[0];
        if (Controller) {
            auto Comp = (UFortControllerComponent_MapDiscoverability*)Controller->GetComponentByClass(UFortControllerComponent_MapDiscoverability::StaticClass());
            Comp->SetDiscoverStatusByTag(Tag.TagName.GetRawWString().c_str(), true);
            Comp->DiscoveredData.MarkArrayDirty();
        }
        
        UE_LOG(LogTemp, Log, "Process Location Tags Poi Tag: %s", Tag.TagName.ToString().c_str());
    }
    return ProcessLocationTagDeltaForControllerOG(Manager, Controller, TagsToRemove, TagsToAdd);
}

void FortPoiVolume::Setup()
{
    UHook* Hook = new UHook();

    Hook->Address = ImageBase + 0x168CB00;
    Hook->Original = (void**)&PostInitializeComponentsOG;
    Hook->Detour = PostInitializeComponents;
    UKismetHookingLibrary::Hook(Hook, EHook::Address);

    Hook->Address = ImageBase + 0xA33D708;
    Hook->Original = (void**)&SendComplexCustomStatEventOG;
    Hook->Detour = SendComplexCustomStatEvent;
    UKismetHookingLibrary::Hook(Hook, EHook::Address);

    Hook->Address = ImageBase + 0x90E840C;
    Hook->Original = (void**)&ProcessLocationTagDeltaForControllerOG;
    Hook->Detour = ProcessLocationTagDeltaForController;
    UKismetHookingLibrary::Hook(Hook, EHook::Address);

    Hook->Address = ImageBase + 0x23507CC;
    UKismetHookingLibrary::Hook(Hook, EHook::RTrue);

    delete Hook;
}
