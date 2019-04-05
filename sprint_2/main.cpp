#include "joystick.h"
#include "BrickPi3.h"
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <cstdlib>
using namespace std;

BrickPi3 bp;

void exit_signal_handler(int signo);

sensor_ultrasonic_t ultrasonic;

//Motor variables easier to use
uint8_t left_motor = PORT_C;
uint8_t right_motor = PORT_A;

//ultraasoon position value's
int32_t ultrasoon_left = 0;
int32_t ultrasoon_right = 0;
int32_t ultrasoon_straight = 0;

//Left turn fucntion for the robot
void left_turn(){
    bp.set_motor_dps(right_motor, -360);
    bp.set_motor_dps(left_motor, 360);
    usleep(875000);
}

//Right turn function for the robot
void right_turn(){
    bp.set_motor_dps(right_motor, 360);
    bp.set_motor_dps(left_motor, -360);
    usleep(875000);
}

//Looking left with the ultrasoon function
int look_left(){
    bp.set_motor_position(PORT_D, ultrasoon_left);
    usleep(500000);
    int distance = ultrasonic.cm;
    usleep(500000);
    return distance;
}

//Looking right with the ultrasoon function
int look_right(){
    bp.set_motor_position(PORT_D, ultrasoon_right);
    usleep(500000);
    int distance = ultrasonic.cm;
    usleep(500000);
    return distance;
}


int main() {
    //Detecting the controller
    detect();

    //Detecting the brickpi
    bp.detect();

    //Exit handler
    signal(SIGINT, exit_signal_handler);

    //set motor limits
    bp.set_motor_limits(left_motor, 100, 0);
    bp.set_motor_limits(right_motor, 100, 0);

    //Set sensors
    bp.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_ULTRASONIC);

    //calibration ultrasoon
    string regel;
    cout << "############ Calibration Ultrasoon ############" << endl << "Put the ultrasoon straight and hit enter: " << endl;
    cin >> regel;
    ultrasoon_straight = bp.get_motor_encoder(PORT_D);
    cout << "Straight = " << static_cast<int32_t>(ultrasoon_straight) << endl;
    cout << "Put the ultrasoon left and hit enter: " << endl;
    cin >> regel;
    ultrasoon_left = bp.get_motor_encoder(PORT_D);
    cout << "Left = " << static_cast<int32_t>(ultrasoon_left) << endl;
    cout << "Put the ultrasoon right and hit enter: " << endl;
    cin >> regel;
    ultrasoon_right = bp.get_motor_encoder(PORT_D);
    cout << "Right = " << static_cast<int32_t>(ultrasoon_right) << endl;
    cout << "############## Calibration  Done ##############" << endl;
    bp.set_motor_position(PORT_D, ultrasoon_straight);



    while (true) {
        //Check the controller for inputs
        update();
        int l2 = int(float(getAxis(AXIS_L3Y) * -1) / 32767.0 * 100.0);
        int r2 = int(float(getAxis(AXIS_R3Y) * -1) / 32767.0 * 100.0);
        int butA = getButton(BUTTON_A);
        int butY = getButton(BUTTON_Y);
        int butB = getButton(BUTTON_B);
        int lr = getAxis(AXIS_L2);
        int voltBattery = bp.get_voltage_battery();
        int32_t position = bp.get_motor_encoder(PORT_D);
        //Start for autonoom driving
        if(butA == 1){
            while(true){
                update();
                int butB = getButton(BUTTON_B);
                if(butB == 1){
                    break;
                }
                if(bp.get_sensor(PORT_1, ultrasonic) == 0) { //check if the ultrasonic sensor works correctly
                    cout << "sensor working" << endl;
                    int distanceCm = ultrasonic.cm;
                    cout << "Afstand: " << distanceCm << "cm" << endl;
                    if(distanceCm <= 20){
                        bp.set_motor_power(right_motor, 0);
                        bp.set_motor_power(left_motor, 0);
                        sleep(2);
                        int distance_left= look_left();
                        int distance_right = look_right();
                        cout << "left: " << distance_left << endl << "right: " << distance_right << endl;
                        bp.set_motor_position(PORT_D, ultrasoon_straight);
                        if(distance_left > distance_right){
                            left_turn();
                        }else{
                            right_turn();
                        }
                    }
                }
                bp.set_motor_power(right_motor, 50);
                bp.set_motor_power(left_motor, 50);
            }
        }


//        if(butA == 1){
//            bp.set_motor_position(PORT_D, left);
//        }
//        if(butB == 1){
//            bp.set_motor_position(PORT_D, straight);
//        }
//        if(butY == 1){
//            bp.set_motor_position(PORT_D, right);
//        }



//        printf("int: %d    ", l2);
//        printf("\r");
//        fflush(stdout);
//        usleep(100000);

        bp.set_motor_power(right_motor, l2);
        bp.set_motor_power(left_motor, r2);
    }
}


void exit_signal_handler(int signo){
    if(signo == SIGINT){
        cout << "exit" << endl;
        bp.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}