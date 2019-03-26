#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <signal.h>
#include "BrickPi3.h"

using namespace std;

BrickPi3 bp;

sensor_color_t color;
sensor_light_t light;
sensor_ultrasonic_t ultrasonic;

int power = 60;

void exit_signal_handler(int signo);

int16_t measure_light(int16_t val, uint16_t MIN, uint16_t MAX) {
    if (val < MIN) val = MIN;
    if (val > MAX) val = MAX;
    return (power*(val - MIN))/(MAX - MIN);
}

int16_t measure_color(int16_t val, uint16_t MIN, uint16_t MAX) {
    if (val < MIN) val = MIN;
    if (val > MAX) val = MAX;
    return (power*(MAX - val))/(MAX - MIN);
}

int main() {
    signal(SIGINT, exit_signal_handler);

    bp.detect(true);

    bp.set_motor_limits(PORT_A, 60, 0);
    bp.set_motor_limits(PORT_B, 60, 0);

    bp.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_FULL);
    bp.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);
    bp.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);

//    uint16_t left_min = 1670;
//    uint16_t left_max = 2540;
//    uint16_t right_min = 270;
//    uint16_t right_max = 630;
    uint16_t left_min = 1670;
    uint16_t left_max = 2540;
    uint16_t right_min = 270;
    uint16_t right_max = 630;

    int16_t leftval;
    int16_t rightval;

    bp.set_motor_power(PORT_A, 0);
    bp.set_motor_power(PORT_B, 0);

    int zerocount = 0;

    while(true){
        if (bp.get_sensor(PORT_1, color) == 0 && bp.get_sensor(PORT_3, light) == 0){
            leftval = measure_light(light.reflected, left_min, left_max);
            rightval = measure_color(color.reflected_red, right_min, right_max);

            if (leftval == 0 && rightval == 0) {
                zerocount++;

                if (zerocount >= 10) {
                    cout << "ALARM" << endl;
                    bp.reset_all();
                    exit(-2);
                }
            }else{
                zerocount = 0;
            }

            uint16_t apower = power - (rightval);
            uint16_t bpower = power - (leftval);

            bp.set_motor_power(PORT_A, apower);
            bp.set_motor_power(PORT_B, bpower);

//            cout << "Color:" << setw(4) << color.reflected_red << endl;
//            cout << "Infrared:" << setw(4) << light.reflected << endl;
            cout << leftval << ", " << rightval << endl;
            cout << "power: " << apower << ", " << bpower << endl;
        }


//        if (bp.get_sensor(PORT_1, color) == 0) {
//            cout << "Color sensor (S1): detected  " << (int) color.color;
//            cout << " red" << setw(4) << color.reflected_red;
//        }
//
//        if (bp.get_sensor(PORT_3, light) == 0) {
//            cout << "Light sensor (S3): reflected " << setw(4) << light.reflected << endl;
//        }
//
//        int state = bp.get_sensor(PORT_2, ultrasonic);
//        if (state == 0) {
//            cout << "Ultrasonic sensor (S2): "   << ultrasonic.cm << "cm" << endl;
//        }

        sleep(0.1);
    }

//    bp.set_motor_power(PORT_A, 20);
//    bp.set_motor_power(PORT_B, 20);
//    sleep(3);
//    bp.set_motor_power(PORT_A, 0);
//    bp.set_motor_power(PORT_B, 0);

    bp.reset_all();
}

void exit_signal_handler(int signo){
    if(signo == SIGINT){
        bp.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}