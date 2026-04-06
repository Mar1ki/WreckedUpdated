#pragma once
#include "pch.h"

#define UE_WITH_IRIS

enum class EReplicationSystemSendPass : unsigned
{
    Invalid,

    // Sending data directly after PostDispatch, this is a partial update only, not updating scope and filtering and will only process RPC/Attachmeents
    PostTickDispatch,

    // Sending data part of TickFlush, this will do a full update and replicate data to all connections
    TickFlush,
};

class FNetRefHandle
{
public:

	inline static FNetRefHandle GetInvalid() { return FNetRefHandle(); }

private:
	enum { InvalidValue = 0 };
	enum { IdBits = 60 };
	enum { ReplicationSystemIdBits = 4 };

public:

	FNetRefHandle() : Value(InvalidValue) {}

	uint64 GetId() const { return Id; }
	uint32 GetReplicationSystemId() const { check(ReplicationSystemId != 0); return (uint32)(ReplicationSystemId - 1); }
	bool IsValid() const { return Value != InvalidValue; }

	bool IsCompleteHandle() const { return Value != InvalidValue && ReplicationSystemId != 0U; }

	bool IsStatic() const { return Id & StaticIdMask; }

	bool IsDynamic() const { return IsValid() && !IsStatic(); }

	bool operator==(const FNetRefHandle& Other)const { return Id == Other.Id; }
	bool operator<(const FNetRefHandle& Other)const { return Id < Other.Id; }
	bool operator!=(const FNetRefHandle& Other)const { return Id != Other.Id; }

	static bool FullCompare(FNetRefHandle A, FNetRefHandle B) { return A.Value == B.Value; }

public:

	static constexpr uint64 StaticIdMask = 1;
	static constexpr uint64 IdMask = (1ULL << IdBits) - 1;
	static constexpr uint64 MaxReplicationSystemId = (1ULL << ReplicationSystemIdBits) - 1;

	union
	{
		struct
		{
			uint64 Id : IdBits;
			uint64 ReplicationSystemId : ReplicationSystemIdBits;
		};
		uint64 Value;
	};
};

struct FObjectKey {
public:
	int32		ObjectIndex;
	int32		ObjectSerialNumber;
};
class FNetHandle
{
public:
	struct FInternalValue
	{
		uint32 Id;
		uint32 Epoch;
	};

	union
	{
		FObjectKey Value;
		FInternalValue InternalValue;
	};
};

struct FSendUpdateParams
{
    EReplicationSystemSendPass SendPass = EReplicationSystemSendPass::TickFlush;

    float DeltaSeconds = 0.f;		
};
typedef uint32 FReplicationProtocolIdentifier;

enum class EReplicationProtocolTraits : uint16
{
	None = 0,
	// Whether any of the replication states has dynamic state
	HasDynamicState = 1U << 0U,
	// Whether there's a state with legacy lifetime conditionals
	HasLifetimeConditionals = HasDynamicState << 1U,
	// Whether there's a changemask for conditionals stored in the internal state, such as when there are custom conditionals
	HasConditionalChangeMask = HasLifetimeConditionals << 1U,
	// Whether there's a state that has connection specific serialization
	HasConnectionSpecificSerialization = HasConditionalChangeMask << 1U,
	// Whether there's a state that has object references
	HasObjectReference = HasConnectionSpecificSerialization << 1U,
	// Whether delta compression is supported or not, essentially whether it makes sense to create baselines or not.
	SupportsDeltaCompression = HasObjectReference << 1U,
};
typedef uint32 FNetDebugNameId;

struct FNetDebugName
{
	const TCHAR* Name = nullptr;
	mutable FNetDebugNameId DebugNameId = 0;
};
 
struct FReplicationProtocol {
	const void** ReplicationStateDescriptors;
	uint32 ReplicationStateCount;		// Number of states
	uint32 InternalTotalSize;			// Total memory required to store the complete state
	uint32 InternalTotalAlignment;		// Alignment of the internal state
	uint32 MaxExternalStateSize;		// Max external state size, required when we push temporary states to game
	uint32 MaxExternalStateAlignment;	// Max external state alignment, required when allocating temporary state buffer

	// These two members are only valid if traits include HasConditionalChangeMask. The target state has the HasLifetimeConditionals trait.
	uint16 FirstLifetimeConditionalsStateIndex;
	uint16 LifetimeConditionalsStateCount;
	uint32 FirstLifetimeConditionalsChangeMaskOffset;

	uint32 ChangeMaskBitCount;			// How many bits do we need to store the full changemask
	uint32 InternalChangeMasksOffset;

	FReplicationProtocolIdentifier ProtocolIdentifier;
	const FNetDebugName* DebugName;

	// TypeStatsIndex assigned to this protocol
	int32 TypeStatsIndex;

	// TODO: Cache parts of the descriptors in the protocol to avoid having to pull in the descriptor itself for operations iterating over the protocol

	// Selected traits from the ReplicationStateDescriptors that might be handy to cache in the protocol.
	EReplicationProtocolTraits ProtocolTraits;

	mutable int32 RefCount;
};
struct FNetRefhandleManagerInitParams
{
	uint32 ReplicationSystemId = 0;
	uint32 MaxActiveObjectCount = 0;
	uint32 InternalNetRefIndexInitSize = 0;
	uint32 NetChunkedArrayCount = 0;
	uint32 InternalNetRefIndexGrowSize = 0;
};

namespace NetDriver
{
	DefineOriginal(void, TickFlush, UNetDriver* NetDriver, float DeltaSecodnds);
    void Init();
}


inline void UNetDriver::UpdateIrisReplicationViews() const
{
	static void(*UpdateReplicationViewsInternal)(const UNetDriver*) = decltype(UpdateReplicationViewsInternal)(ImageBase + 0x6CE5844);
	UpdateReplicationViewsInternal(this);
}

inline void PreSendUpdate(UReplicationSystem* ReplicationSystem, FSendUpdateParams& Params)
{
	static void(*PreSendUpdateInternal)(UReplicationSystem*, FSendUpdateParams&) = decltype(PreSendUpdateInternal)(ImageBase + 0x5F8FD5C);
	PreSendUpdateInternal(ReplicationSystem, Params);
}

inline void APlayerController::SendClientAdjustment()
{
	static void (__fastcall* SendClientAdjustmentInternal)(APlayerController*) = decltype(SendClientAdjustmentInternal)(ImageBase + 0x6DD2A10);
	SendClientAdjustmentInternal(this);
}


inline void UNetDriver::SendClientMoveAdjustments()
{

	for (UNetConnection* Connection : this->ClientConnections)
	{
		if (Connection == nullptr || Connection->ViewTarget == nullptr)
			continue;
		

		if (APlayerController* PC = Connection->PlayerController)
			PC->SendClientAdjustment();
		

		for (UNetConnection* ChildConnection : Connection->Children)
		{
			if (ChildConnection == nullptr)
				continue;
			

			if (APlayerController* PC = ChildConnection->PlayerController)
				PC->SendClientAdjustment();
			
		}
	}
}