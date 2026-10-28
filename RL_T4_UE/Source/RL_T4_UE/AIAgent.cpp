// Fill out your copyright notice in the Description page of Project Settings.


#include "AIAgent.h"
#include "PlayBoard.h"

#include "Async/Async.h"
#include "Windows/WindowsHWrapper.h"

#define TEST_1	// 임시 테스트 모드 활성화

FAIModelConnector::FAIModelConnector()
{
}

FAIModelConnector::FAIModelConnector(AAIAgent* NewControlAgent)
{
	ControlAgent = NewControlAgent;
	
	MaxEpisode = ControlAgent->MaxEpisode;
	MaxStep = ControlAgent->MaxStep;
	
	Thread = FRunnableThread::Create(this, TEXT("FAIModelConnector"));
}

bool FAIModelConnector::Init()
{
	return FRunnable::Init();
}

uint32 FAIModelConnector::Run()
{
	UE_LOG(LogTemp, Log, TEXT("FAIModelConnector::Run >> Start"));

	CurrentEpisode = 0;
	CurrentStep = 0;

	// 공유메모리 생성
	HANDLE SharedMemory_Handle = CreateFileMappingW(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE,
		0,
		sizeof(FSharedMemoryType),
		L"Local\\AgentMemory"
	);

	if (nullptr == SharedMemory_Handle) {
		int32 ErrorNo = int32(GetLastError());
		
		if (ErrorNo == ERROR_ALREADY_EXISTS) {
			UE_LOG(LogTemp, Log, TEXT("CreateFileMappingW already exist error :  %d"), ErrorNo);
			// 문제없으므로 계속해서 진행
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("CreateFileMappingW Error (%d)"), ErrorNo);
			CloseHandle(SharedMemory_Handle);
			return 1;
		}
	}

	void* SharedMemory_Addr = MapViewOfFile(
		SharedMemory_Handle,
		FILE_MAP_ALL_ACCESS,
		0, 0,
		sizeof(FSharedMemoryType)
	);

	if (SharedMemory_Addr == nullptr) {
		int32 ErrorNo = int32(GetLastError());
		UE_LOG(LogTemp, Warning, TEXT("MapViewOfFile is Failed. %d"), ErrorNo);
		UnmapViewOfFile(SharedMemory_Addr);
		CloseHandle(SharedMemory_Handle);
	}

	FSharedMemoryType* SharedMemory = static_cast<FSharedMemoryType*>(SharedMemory_Addr);


#ifdef TEST_1

#else 
	// 이벤트 생성

	HANDLE QuitTrainEvent = CreateEventW(nullptr, false, false, L"Local\\QuitTrainEvent");

	auto CloseEvent = [QuitTrainEvent]() {	
		CloseHandle(QuitTrainEvent);
	};

#endif


	if (!(MaxEpisode > CurrentEpisode && MaxStep > CurrentEpisode))
	{
		UE_LOG(LogTemp, Log, TEXT("FAIModelConnector::Run >> Episode or Step is not setted correctly"));
		return 0;
	}
	
	
	while (!bStopRequested)
	{
		for (int i = 0 ; i < MaxEpisode ; i++)
		{
			CurrentEpisode = i;
			for (int j = 0 ; j < MaxStep ; j++)
			{
				if (bStopRequested){
					break;
				}
				
				CurrentStep = j;

#ifdef TEST_1

				UE_LOG(LogTemp, Log, TEXT("FAIModelConnector::Run >> Episode : %d/%d , Step : %d/%d "), CurrentEpisode, MaxEpisode, CurrentStep, MaxStep);
				SharedMemory->IsTraining = j;
				FPlatformProcess::Sleep(1);

#else

				// 실제로직
#endif

			}

			
			if (bStopRequested)
			{
				break;		
			}
		}
	}


#ifdef TEST_1

#else
	SetEvent(QuitTrainEvent);

	CloseEvent();
#endif



	UE_LOG(LogTemp, Log, TEXT("FAIModelConnector::Run >> End"));

	return 0;
}

void FAIModelConnector::Stop()
{
	FRunnable::Stop();
	bStopRequested = true;
	
	UE_LOG(LogTemp, Log, TEXT("FAIModelConnector::Stop >> Called "));
}

void FAIModelConnector::Exit()
{
	FRunnable::Exit();
	
	UE_LOG(LogTemp, Log, TEXT("FAIModelConnector::Exit >> Called "));
}

FAIModelConnector::~FAIModelConnector()
{
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
		Thread = nullptr;
	}
}

// Sets default values
AAIAgent::AAIAgent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MaxEpisode = 10;
	MaxStep = 50;
}

// Called when the game starts or when spawned
void AAIAgent::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAIAgent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (ModelConnector != nullptr){
		ModelConnector->Stop();
	}
}

// Called every frame
void AAIAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAIAgent::BrainTrain()
{
	// 자동으로 Init , Run 된다.
	ModelConnector = MakeUnique<FAIModelConnector, AAIAgent*>(this);
}

void AAIAgent::PostEndTrain_Implementation()
{
	ModelConnector = nullptr;
}

float AAIAgent::PlayAction(int32 Action)
{
	if (IsValid(PlayBoard))
	{
		return PlayBoard->MovePlayer(Action);
	}else
	{
		return 0.0f;
	}
}

