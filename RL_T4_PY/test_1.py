import torch
import ctypes # ctype 을 쓸 수 있도록 하는 라이브러리. windows type 도 ctype 이다.
from multiprocessing import shared_memory

import struct
import time
import random

from pathlib import Path

from datetime import datetime
from pathlib import Path
import inspect

LOG_FILE_PATH = Path(f"logs.txt")

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
    def isActionSetted(self, newActionSetted) :
        self.shared.buf[28:32] = struct.pack("<i", newActionSetted)

    @property
    def isRewradSetted(self) :
        isRewardSetted, = struct.unpack("<i", self.shared.buf[32:36])
        return isRewardSetted

    @property
    def isTrained(self, newIsTrained) :
        self.shared.buf[36:40] = struct.pack("<i", newIsTrained)

    def close(self):
        if self.shared != None :
            # 현재 python 에서 연결 해제
            self.shared.close()

            # 운영체제에서 공유메모리 이름제거. 근데 이건 unreal 에서 해야하지 않나 싶다.
            self.shared.unlink()
    

def main():
    print("Hello from rl-t4-py!")

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

    sharedMemory = SharedMemoryType()

    while True : 

        if sharedMemory.isTraining  > 0:

            print(sharedMemory.isTraining)
            time.sleep(100)
        else : 
            break;

    print("trainEnd!")

if __name__ == "__main__":
    main()
