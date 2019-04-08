#ifndef TI_PROJECT_JOYSTICK_H
#define TI_PROJECT_JOYSTICK_H

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

#define JOY_DEV "/dev/input/js0"

enum BUTTONS {
    BUTTON_B = 0,
    BUTTON_A = 1,
    BUTTON_Y = 2,
    BUTTON_X = 3,
    BUTTON_L1 = 4,
    BUTTON_R1 = 5,
    BUTTON_SELECT = 6,
    BUTTON_START = 7,
    BUTTON_L3 = 8,
    BUTTON_R3 = 9,
};

enum AXIS {
    AXIS_L3X = 0,
    AXIS_L3Y = 1,
    AXIS_L2  = 2,
    AXIS_R3X = 3,
    AXIS_R3Y = 4,
    AXIS_R2  = 5,
    AXIS_LR  = 6,
    AXIS_UD  = 7,
};

int detect();

int update();

char getButton(int id);

int getAxis(int id);

#endif //TI_PROJECT_JOYSTICK_H
