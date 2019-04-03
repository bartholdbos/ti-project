#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

#define JOY_DEV "/dev/input/js0"

int joy_fd, *axis = NULL, num_of_axis = 0, num_of_buttons = 0;
char *button = NULL, name_of_joystick[80];
struct js_event js;

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

int detect() {
    if ((joy_fd = open(JOY_DEV, O_RDONLY)) == -1) {
        printf("Couldn't open joystick\n");
        return -1;
    }

    ioctl(joy_fd, JSIOCGAXES, &num_of_axis);
    ioctl(joy_fd, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);

    axis = (int *) calloc(num_of_axis, sizeof(int));
    button = (char *) calloc(num_of_buttons, sizeof(char));

    fcntl( joy_fd, F_SETFL, O_NONBLOCK );

    printf(
            "Joystick name: \t%s\nJoystick axis: \t%d\nJoystick btn: \t%d\n\n",
            name_of_joystick,
            num_of_axis,
            num_of_buttons
    );
}

int update() {
    read(joy_fd, &js, sizeof(struct js_event));

    switch (js.type & ~JS_EVENT_INIT) {
        case JS_EVENT_AXIS:
            axis[js.number] = js.value;
            break;
        case JS_EVENT_BUTTON:
            button[js.number] = js.value;
            break;
    }
}

char getButton(int id) {
    return button[id];
}

int getAxis(int id) {
    return axis[id];
}

int main() {
    detect();

    while (true) {
        update();
        printf("Axis: %d \r", getAxis(AXIS_L2));
        fflush(stdout);
        usleep(100000);
    }
}