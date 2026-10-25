// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayBoard.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GeometryCollection/GeometryCollectionAlgo.h"


// Sets default values
APlayBoard::APlayBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);
	
	// FString SM_Cube_Path = TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'");
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_Mesh_Finder(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	
	if (SM_Mesh_Finder.Succeeded())
	{
		for (int i = 0 ; i < 25 ; i++)
		{
			int X = i / 5;
			int Y = i % 5;
			
			FVector SpawnPoint = PointToLocation(X, Y);
			
			FString BoardName = FString::Printf(TEXT("BoardElem_%d"), i);
			
			UStaticMeshComponent* Board = CreateDefaultSubobject<UStaticMeshComponent>(*BoardName);
			Board->SetStaticMesh(SM_Mesh_Finder.Object);
			
			Boards.Push(Board);
			
			Board->SetupAttachment(RootComp);
			Board->SetRelativeLocation(SpawnPoint);
			Board->SetRelativeScale3D(FVector(0.95f, 0.95f, 0.1f));
			Board->AddRelativeLocation(FVector(0.0f, 0.0f, -10.0f));

		}
	}
	
	
	Mark_Start = CreateDefaultSubobject<UBillboardComponent>(TEXT("Mark_Start"));
	Mark_Start->SetupAttachment(RootComp);
	static ConstructorHelpers::FObjectFinder<UTexture2D> Img_Start_Finder(TEXT("/Game/ProjectContent/Object/2D/Img_Star_1.Img_Star_1"));
	if (Img_Start_Finder.Succeeded()){
		Mark_Start->SetSprite(Img_Start_Finder.Object);
	}
	Mark_Start->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	Mark_Start->SetHiddenInGame(false);
	
	Mark_End = CreateDefaultSubobject<UBillboardComponent>(TEXT("Mark_End"));
	Mark_Start->SetupAttachment(RootComp);
	static ConstructorHelpers::FObjectFinder<UTexture2D> Img_End_Finder(TEXT("/Game/ProjectContent/Object/2D/Img_Reward_1.Img_Reward_1"));
	if (Img_End_Finder.Succeeded()){
		Mark_End->SetSprite(Img_End_Finder.Object);
	}
	Mark_End->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	Mark_End->SetHiddenInGame(false);
	
	Mark_Danger = CreateDefaultSubobject<UBillboardComponent>(TEXT("Mark_Danger"));
	Mark_Start->SetupAttachment(RootComp);
	static ConstructorHelpers::FObjectFinder<UTexture2D> Img_Danger_Finder(TEXT("/Game/ProjectContent/Object/2D/Img_Bomb_1.Img_Bomb_1"));
	if (Img_Danger_Finder.Succeeded()){
		Mark_Danger->SetSprite(Img_Danger_Finder.Object);
	}
	Mark_Danger->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	Mark_Danger->SetHiddenInGame(false);
	
	// '/Game/ProjectContent/Object/2D/Img_Star_1.Img_Star_1'
	// '/Game/ProjectContent/Object/2D/Img_Reward_1.Img_Reward_1'
	// '/Game/ProjectContent/Object/2D/Img_Bomb_1.Img_Bomb_1'
	
}

// Called when the game starts or when spawned
void APlayBoard::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector2D APlayBoard::IndexToPoint(int32 Index) const
{
	return FVector2D(Index / 5, Index % 5);
}

FVector APlayBoard::PointToLocation(int32 X, int32 Y) const
{
	float X_Pos = (X * 100.0f) - 200.0f;
	float Y_Pos = (Y * 100.0f) - 200.0f;
	
	return FVector(X_Pos, Y_Pos, 0.0f);
}

void APlayBoard::SetRandmark()
{
	
	// 0 ~ 24 까지 하나씩 뽑는거지.
	
	TArray<int> Marks;
	
	while (Marks.Num() < 3)
	{
		int Mark = FMath::RandRange(0,24);
		bool IsNear = false;
		for (int i = 0 ; i < Marks.Num() ; i++)
		{
			int ComparePivot = Marks[i];
			
			if ((FMath::Abs(ComparePivot - Mark) == 1) || (FMath::Abs(ComparePivot - Mark) == 5))
			{
				IsNear = true;
			}
		}
		
		if (!IsNear)
		{
			Marks.AddUnique(Mark);
		}
	}
	
	StartPos = Marks[0];
	EndPos = Marks[1];
	DangerPos = Marks[2];
	
	FVector2D StartPos_2D = IndexToPoint(StartPos);
	FVector StartPos_3D = PointToLocation(StartPos_2D.X, StartPos_2D.Y);
	StartPos_3D.Z = 25.0f; 
	
	Mark_Start->SetRelativeLocation(StartPos_3D);
	
	FVector2D EndPos_2D = IndexToPoint(EndPos);
	FVector EndPos_3D = PointToLocation(EndPos_2D.X, EndPos_2D.Y);
	EndPos_3D.Z = 25.0f;
	
	Mark_End->SetRelativeLocation(EndPos_3D);
	
	FVector2D DangerPos_2D = IndexToPoint(DangerPos);
	FVector DangerPos_3D = PointToLocation(DangerPos_2D.X, DangerPos_2D.Y);
	DangerPos_3D.Z = 25.0f;
	
	Mark_Danger->SetRelativeLocation(DangerPos_3D);
}

void APlayBoard::SetPlayerToStartLoc()
{
	if (IsValid(PlayerAgent))
	{
		FTransform ParentTransform = RootComp->GetComponentTransform();
		
		FVector2D StartPos_2D = IndexToPoint(StartPos);
		FVector StartPos_3D = PointToLocation(StartPos_2D.X, StartPos_2D.Y);
		
		FVector WorldLocation = ParentTransform.TransformPosition(StartPos_3D);
		
		PlayerAgent->SetActorLocation(WorldLocation);
	}
}

void APlayBoard::init()
{
	
}

float APlayBoard::MovePlayer(int32 Action)
{
	if (!IsValid(PlayerAgent)){
		return 0.0f;
	}
	
	int32 PosToMove = PlayerPos;
	float Result = 0.0f;
	switch (Action)
	{
		case 0: // 상
		{
			PosToMove += 5;
				
			if (PosToMove < 0 || PosToMove > 24)
			{
				PosToMove = PlayerPos;
				Result = -1.0f;
			}
					
			break;
		}
		case 1: // 하
		{
			PosToMove -= 5;
				
			if (PosToMove < 0 || PosToMove > 24)
			{
				PosToMove = PlayerPos;
				Result = -1.0f;
			}
				
			break;
		}
		case 2: // 좌
		{
			if (PosToMove % 5 == 0)
			{
				Result = -1.0f;
			}else
			{
				PosToMove -= 1;
			}
				
			break;	
		}
		case 3: // 우
		{
			if (PosToMove % 5 == 4)
			{
				Result = -1.0f;
			}else
			{
				PosToMove +=1;
			}
			
			break;	
		}
		default :
		{
			break;
		}
	}
	
	if (!FMath::IsNearlyEqual(Result, -1.0f, 0.1f))
	{
		// 밖을 벗어난 경우가 아니라면, 
		
		if (PosToMove == DangerPos)
		{
			Result = -10.0f;
		}else if (PosToMove == EndPos)
		{
			Result = 20.0f;
		}
		else
		{
			Result = 1.0f;
		}
	}
	
	FVector2D PosToMove_2D = IndexToPoint(PosToMove);
	FVector PosToMove_3D = PointToLocation(int32(PosToMove_2D.X), int32(PosToMove_2D.Y));
	
	FTransform ParentTranform = GetTransform();
	FVector LocToMove_3D = ParentTranform.TransformPosition(PosToMove_3D);
	
	PlayerAgent->SetActorLocation(LocToMove_3D);
	PlayerPos = PosToMove;
	
	return Result;
}

