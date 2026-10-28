// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "AIAgent.generated.h"

struct FSharedMemoryType
{
	// 0:4
	uint32 IsTraining;
	
	// 4:8
	uint32 State;

	// 8:12
	uint32 NextState;

	// 12:16
	uint32 Action;

	// 16:20
	float Reward;
	
	// 20:24
	uint32 IsBrainRunning;

	// 24:28
	uint32 IsStateSetted;

	// 28:32
	uint32 IsActionSetted;

	// 32:36
	uint32 IsRewardSetted;

	// 36:40
	uint32 IsTrained;

	// 40:44
	uint32 danger;

	// 44:48
	uint32 goal;
};

class AAIAgent;
class APlayBoard;
class FAIModelConnector : public FRunnable
{
public:
	
	FAIModelConnector();
	
	FAIModelConnector(AAIAgent* NewControlAgent);
	
	~FAIModelConnector();
	
	virtual bool Init() override;
	
	virtual uint32 Run() override;
	
	virtual void Stop() override;
	
	virtual void Exit() override;
	
protected:
	
	FRunnableThread* Thread = nullptr;
	FThreadSafeBool bStopRequested = false;
	
	TObjectPtr<AAIAgent> ControlAgent;
	
	int32 MaxEpisode;
	int32 CurrentEpisode;
	int32 MaxStep;
	int32 CurrentStep;
};


UCLASS(Blueprintable, BlueprintType)
class RL_T4_UE_API AAIAgent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAIAgent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	
	TUniquePtr<FAIModelConnector> ModelConnector;

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxEpisode;;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxStep;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<APlayBoard> PlayBoard;

public:
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void BrainTrain();
	
	UFUNCTION(Blueprintable, BlueprintNativeEvent, CallInEditor)
	void PostEndTrain();
	void PostEndTrain_Implementation();

public:
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	float PlayAction(int32 Action);
};
