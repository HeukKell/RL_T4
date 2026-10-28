import torch
import ctypes # ctype 을 쓸 수 있도록 하는 라이브러리. windows type 도 ctype 이다.
from multiprocessing import shared_memory

import struct
import time
import random

import os
from pathlib import Path

from datetime import datetime
from pathlib import Path
import inspect

import math

LOG_FILE_PATH = Path(f"logs.txt")
MODEL_PATH = "model.pth"

# ctype wind dll 라이브러리 연결
kernel32 = ctypes.WinDLL("kernel32", use_last_error = True)

# dll 함수 시그니처 선언

kernel32.OpenEventW.argtypes = [
    ctypes.c_uint32,
    ctypes.c_int,
    ctypes.c_wchar_p
]

kernel32.OpenEventW.restype = ctypes.c_void_p

kernel32.WaitForSingleObject.argtypes = [
    ctypes.c_void_p,
    ctypes.c_uint32
]

kernel32.WaitForSingleObject.restype = ctypes.c_uint32

kernel32.CloseHandle.argtypes = [ctypes.c_void_p]

SYNCHRONIZE = 0x00100000
EVENT_MODIFY_STATE = 0x0002
INFINITE = 0xFFFFFFFF

WAIT_OBJECT_0 = 0x00000000
WAIT_TIMEOUT = 0x00000102
WAIT_FAILED = 0xFFFFFFFF

def PLOG(msg : str, writeFile : bool = False):

    caller = inspect.stack[1]

    filepath = Path(caller.filename)
    filename = filepath.name
    lineno = caller.lineno
    funcname = caller.function

    now = datetime.now().strftime("%Y%m%d.%H:%M:%S.%f")[:-3]

    debug_msg = f"[{now}]  {filename}({lineno}).{funcname} >> {msg}"

    print(debug_msg)

    if writeFile :
        LOG_FILE_PATH.parent.mkdir(parents = True, exist_ok = True)

        with open(LOG_FILE_PATH, "a", encoding="utf-8") as file:
            file.write(debug_msg + "\n")


SHARED_MEMORY_NAME = "Local\\AgentMemory"
SHARED_MEMORY_SIZE = 20

shared = None

class SharedMemoryType():

    def __init__(self):
        self.shared = None

        try :
            self.shared = shared_memory.SharedMemory(
                name = SHARED_MEMORY_NAME,
                create = False,
                size = SHARED_MEMORY_SIZE
            )

        except Exception as e :
            PLOG(f"공유 메모리 연결에 실패하였습니다. (e)")

        finally:
            pass

        if self.shared != None :
            PLOG(f"공유 메모리 연결 완료")
        else :
            PLOG("공유 메모리 연결 실패")


    @property
    def isTraining(self) :        
        isTraining, = struct.unpack("<i", self.shared.buf[0:4])
        return isTraining

    @property
    def currentState(self) : 
        state , = struct.unpack("<i", self.shared.buf[4:8])
        return state
    
    @property
    def nextState(self) :
        nextState, = struct.unpack("<i", self.shared.buf[8:12])
        return nextState

    @property
    def action(self, newAction):
        self.shared.buf[12:16] = struct.pack("<i", newAction)

    @property
    def reward(self) :
        reward, = struct.unpack("<f", self.shared.buf[16:20])
        return reward

    @property
    def isBrainRunning(self, newIsBrainRunning) :
        self.shared.buf[20:24] = struct.pack("<i", newIsBrainRunning)

    
    @property
    def isStateSetted(self) :
        state, = struct.unpack("<i", self.shared.buf[24:28])
        return state

    @property
    def isStateSetted(self, newIsStateSetted) :
        self.shared.buf[24:28] = struct.pack("<i", newIsStateSetted)

    @property
    def isActionSetted(self, newActionSetted) :
        self.shared.buf[28:32] = struct.pack("<i", newActionSetted)

    @property
    def isRewradSetted(self) :
        isRewardSetted, = struct.unpack("<i", self.shared.buf[32:36])
        return isRewardSetted

    @property
    def isRewardSetted(self, newIsRewardSetted) :
        self.shared.buf[32:36] = struct.pack("<i", newIsRewardSetted )

    @property
    def isTrained(self, newIsTrained) :
        self.shared.buf[36:40] = struct.pack("<i", newIsTrained)

    @property
    def danger(self) :
        danger, = struct.pack("<i", self.shared.buf[40:44])
        return danger

    @property
    def goal(self) :
        goal, = struct.pack("<i", self.shared.buf[44:48])
        return goal

    def close(self):
        if self.shared != None :
            # 현재 python 에서 연결 해제
            self.shared.close()

            # 운영체제에서 공유메모리 이름제거. 근데 이건 unreal 에서 해야하지 않나 싶다.
            self.shared.unlink()
    
class DQN(torch.nn.Module) :
    def __init__(self):
        super().__init__()

        # 신경망 생성
        # state ( currentState, danger, target ) : 4 input
        # action (상, 하, 좌, 우) : 4 output
        self.brain = None
        self.load_model()

    def forward(self, x) :
        # 예측 시킨다.
        return self.brain(x)

    def save_model(self):

        torch.save(
            self.brain.state_dict(),
            MODEL_PATH
        )

        print(f"{MODEL_PATH} 를 저장하였습니다.")

    def load_model(self) :

        self.brain = torch.nn.Sequential(
            torch.nn.Linear(3,32),
            torch.nn.ReLU(),
            torch.nn.Linear(32,32),
            torch.nn.ReLU(),
            torch.nn.Linear(32,4)
        )

        if os.path.exists(MODEL_PATH):
            print("기존의 모델이 존재하여 모델을 로드 합니다.")

            self.brain.load_state_dict(
                torch.load(
                    MODEL_PATH,
                    weights_only=True
                )
            )
        else :
            print("기존 모델이 없어 새로운 모델을 사용합니다.")

def main():
    print("Hello from rl-t4-py!")

    try :


        
        # 이벤트 생성
        quitTrainEvent = kernel32.OpenEventW(SYNCHRONIZE,False, "Local\\QuitTrainEvent" )

        # 1. 공유메모리 연결
        # 2. NeuralNetwork 생성
        # 3. isBrainRunning = 1
        # 5-0. sharedMemory.isTrainEnd = 0
        # 5-1. state waiting
        # 5-2. state = sharedMemory.state
        # 5-3. state -> NeuralNetwork -> action
        # 5-4. sharedMemory.action = action
        # 5-5. sharedMemory.isRewardSetted waiting
        # 5-6. reward = sharedMemory.reward
        # 5-7. state = currentState, nextstate = sharedMemory.nextState
        # 6. target 계산 ( reward, nextstate)
        # 7. loss 계산
        # 8. optimizer 
        # 9. sharedMemory.isTrainEnd = 1

        # ---

        # ----

        sharedMemory = SharedMemoryType()
        
        model = DQN()

        # 학습의 완전 종료를 알리는 이벤트
        TrainEndEvent = kernel32.OpenEventW()

        while True : 

            while sharedMemory.isTraining == 0 :
                print("waiting to start training...")
                time.sleep(0.5)

            while sharedMemory.isStateSetted == 0 :
                print("waiting to set state")
                time.sleep(0.5)
            sharedMemory.isStateSetted = 0

            currentState = sharedMemory.currentState
            danger = sharedMemory.danger
            goal = sharedMemory.goal

            # TODO : 공유메모리 추가 필요. danger, goal 도 읽어와야 한다.

            x = torch.tensor(
                [currentState, danger, goal],
                dtype=torch.float32
            )

            result = model(x)

            # e-greedy 정책
            epcilon = 0.2
            action = -1
            if random.random() < epcilon :
                # 탐험
                action = result[random.randint(0,3)].item()
            else:
                action = result.argmax().item()


            sharedMemory.action = action

            while sharedMemory.isRewradSetted == 0 :
                print("waiting to set reward")
                time.sleep(0.5)
            sharedMemory.isRewradSetted = 0

            reward = sharedMemory.reward
            prevState = sharedMemory.currentState
            nextState = sharedMemory.nextState

            loss_function = torch.nn.MSELoss()
            gamma = 0.9

            
            target = None
            # target 계산

            qValue_currAction = torch.tensor([currentState, danger, goal], dtype=torch.float32)[action].item()
            qVal_future_x = torch.tensor([nextState, danger, goal], dtype=torch.float32)
            qVal_future = model(qVal_future_x).max().item()

            # if reward == -10.0 or reward == math
            if math.isclose(reward, -10.0, abs_tol=1e-3) or math.isclose(reward, 20, abs_tol= 1e-3) :
                # 하나의 episode 종료 조건
                target = reward
            else :
                target = reward + gamma*(qVal_future)
                
            loss = loss_function(qValue_currAction, target)

            loss.backward()

            optimizer = torch.optim.Adam(
                params = model.parameters,
                lr=0.1
            )

            optimizer.step()

            sharedMemory.isTrained = 1

            # training 시작해놓고 끊는거면 종료했다는 거다.

            # 0.5 초 기다리고, 만약 종료신호가 왔었다면, 그대로 종료
            wait_quit_event_res = kernel32.WaitForSingleObject(quitTrainEvent,500)

            if wait_quit_event_res == WAIT_TIMEOUT :
                break;
        
    except Exception as e :
        PLOG(e)
    finally:
        model.save_model()

        sharedMemory.close()
        kernel32.CloseHandle(wait_quit_event_res)

    print("trainEnd!")

if __name__ == "__main__":
    main()
