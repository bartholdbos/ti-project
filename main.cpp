#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <signal.h>
#include <cstdlib>
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

    uint16_t left_min = 1710;
    uint16_t left_max = 2600;
    uint16_t right_min = 230;
    uint16_t right_max = 627;

    int16_t leftval;
    int16_t rightval;

    int zerocount = 0;

    int8_t apower;
    int8_t bpower;

    bool left = false;
    bool right = false;
    bool alarm = false;
    bool intersect = false;
    bool lastturn = false;
    while(true){
        if(bp.get_sensor(PORT_2, ultrasonic) == 0){
            int distanceCm = ultrasonic.cm;
//            cout << "Ultrasonic sensor (S2): " << distanceCm << "cm" << endl;
            if(distanceCm <= 5){
                bp.set_motor_power(PORT_A, 55);
                bp.set_motor_power(PORT_B, -55);
                sleep(1);
                bp.set_motor_power(PORT_A, 50);
                bp.set_motor_power(PORT_B, 50);
                sleep(1);
                bp.set_motor_power(PORT_A, -55);
                bp.set_motor_power(PORT_B, 55);
                sleep(1);
                bp.set_motor_power(PORT_A, 50);
                bp.set_motor_power(PORT_B, 50);
                usleep(2500000);
                bp.set_motor_power(PORT_A, -55);
                bp.set_motor_power(PORT_B, 55);
                sleep(1);
                bp.set_motor_power(PORT_A, 50);
                bp.set_motor_power(PORT_B, 50);
                sleep(1);
                bp.set_motor_power(PORT_A, 50);
                bp.set_motor_power(PORT_B, -50);
                sleep(1);
                bp.set_motor_power(PORT_A, 0);
                bp.set_motor_power(PORT_B, 0);
            }
        }

        if (bp.get_sensor(PORT_1, color) == 0 && bp.get_sensor(PORT_3, light) == 0){
            leftval = measure_light(light.reflected, left_min, left_max);
            rightval = measure_color(color.reflected_red, right_min, right_max);

            if (leftval - rightval > 10){
                left = true;
                right = false;
                alarm = false;
            }else if (leftval - rightval < -10){
                right = true;
                left = false;
                alarm = false;
            }

            if (leftval <= 0 && rightval <= 0) {
                zerocount++;

                if (zerocount >= 10) {
                    alarm = true;
                    cout << "ALARM" << endl;
                    cout << right << ", " << left << endl;
                }
            }else{
                zerocount = 0;
            }

            if (leftval >= 57 && rightval >= 57) {
                intersect = true;
            }

            if (intersect) {
                cout << "intersect" << lastturn << endl;

                bp.set_motor_power(PORT_A, 20);
                bp.set_motor_power(PORT_B, 20);
                sleep(1);
                if (rand() % 2) {
                    bp.set_motor_power(PORT_A, -40);
                    bp.set_motor_power(PORT_B, 40);
                    lastturn = false;
                }else{
                    bp.set_motor_power(PORT_A, 40);
                    bp.set_motor_power(PORT_B, -40);
                    lastturn = true;
                }
                usleep(1000000);
                intersect = false;
            }else if(alarm) {
                if (left) {
                    apower = 40;
                    bpower = -40;
                }

                if (right) {
                    apower = -40;
                    bpower = 40;
                }
            }else{
                apower = power - (rightval);
                bpower = power - (leftval);
            }

            bp.set_motor_power(PORT_A, apower);
            bp.set_motor_power(PORT_B, bpower);

//            cout << "Color:" << setw(4) << color.reflected_red << endl;
//            cout << "Infrared:" << setw(4) << light.reflected << endl;
            cout << leftval << ", " << rightval << endl;
            cout << "power: " << static_cast<int16_t>(apower) << ", " << static_cast<int16_t>(bpower) << endl;
        }
//        sleep(0);
    }
}

void exit_signal_handler(int signo){
    if(signo == SIGINT){
        bp.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}