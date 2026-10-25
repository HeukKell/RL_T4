// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "AIAgent.generated.h"

struct FSharedMemoryType
{
	uint32 IsTraining;
	
	uint32 State;
	uint32 NextState;
	uint32 Action;
	float Reward;
	
	uint32 IsBrainRunning;
	uint32 IsStateSetted;
	uint32 IsActionSetted;
	uint32 IsRewardSetted;
	uint32 IsTrained;
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
