// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "PlayBoard.generated.h"

UCLASS(Blueprintable, BlueprintType)
class RL_T4_UE_API APlayBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayBoard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<USceneComponent> RootComp;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> Boards;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBillboardComponent> Mark_Start;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBillboardComponent> Mark_Danger;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBillboardComponent> Mark_End;
	
	/** Agent의 시작점 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 StartPos;
	
	/** Agent의 목표지점 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 EndPos;
	
	/** 위험지점 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 DangerPos;
	
	/** Player 의 지점 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 PlayerPos;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<AActor> PlayerAgent;
	
	
public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector2D IndexToPoint(int32 Index) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector PointToLocation(int32 X,  int32 Y) const;
	
	/**
	 *	랜덤한 위치에 시작점과 위험지역 목표지점을 설정합니다.
	 * 
	 */
	UFUNCTION(BlueprintCallable, CallInEditor)
	void SetRandmark();
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void SetPlayerToStartLoc();
	
	/** 5x5 grid board 생성
	* 
	*/ 
	UFUNCTION(BlueprintCallable)
	void init();

	/**	플레이어를 움직입니다.
	 * @param Action 상하좌우, 0 이면 위로(-5) 1 이면 아래로(+5) 좌 (-1) 우 (+1)
	 */
	UFUNCTION(BlueprintCallable)
	float MovePlayer(int32 Action);
	
public:
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	float MovePlayer_Up(){
		return MovePlayer(0);
	}
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	float MovePlayer_Down()
	{
		return MovePlayer(1);
	}
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	float MovePlayer_Left()
	{
		return MovePlayer(2);
	}
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	float MovePlayer_Right()
	{
		return MovePlayer(3);
	}
};
