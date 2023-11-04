from enum import Enum

class MotorModeHigh(Enum):
    IDLE = 0
    FORCE_STAND = 1
    VEL_WALK = 2 # the same as pos walk
    POS_WALK = 3
    PATH = 4 #not work
    STAND_DOWN = 5
    STAND_UP = 6
    DAMPING = 7
    RECOVERY = 8 #back to stand
    BACKFLIP = 9
    JUMPYAW = 10
    STRAIGHTHAND = 11 #bai year
    DANCE1 = 12  # different jump and space walk
    DANCE2 = 13  # different jump without space walk

class GaitType(Enum):
    IDLE = 0
    TROT = 1
    TROT_RUNNING = 2
    CLIMB_STAIR = 3
    TROT_OBSTACLE = 4 #our go1 has not this

class SpeedLevel(Enum):
    LOW_SPEED = 0
    MEDIUM_SPEED = 1
    HIGH_SPEED = 2

class Motor(Enum):
    FR_0 = 0,
    FR_1 = 1
    FR_2 = 2
    FL_0 = 3
    FL_1 = 4
    FL_2 = 5
    RR_0 = 6
    RR_1 = 7
    RR_2 = 8
    RL_0 = 9
    RL_1 = 10
    RL_2 = 11

class MotorModeLow(Enum):
    Damping = 0x00
    Servo = 0x0A
    Overheat = 0x08
