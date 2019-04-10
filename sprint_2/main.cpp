#include "joystick.h"
#include "BrickPi3.h"
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

//includes voor stream
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
using namespace std;

BrickPi3 bp;

void exit_signal_handler(int signo);

sensor_ultrasonic_t ultrasonic;
sensor_touch_t touch;

//Motor variables easier to use
uint8_t left_motor = PORT_B;
uint8_t right_motor = PORT_A;

//ultraasoon position value's
int32_t ultrasoon_left = 0;
int32_t ultrasoon_right = 0;
int32_t ultrasoon_straight = 0;

//struct for values of the ultrasoon sensor.
struct Config {
    int32_t straight = 0;
    int32_t left = 0;
    int32_t right = 0;
};

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
int look_left(Config& config){
    bp.set_motor_position(PORT_D, config.left);
    usleep(500000);
    int distance = ultrasonic.cm;
    usleep(500000);
    return distance;
}

//Looking right with the ultrasoon function
int look_right(Config& config){
    bp.set_motor_position(PORT_D, config.right);
    usleep(500000);
    int distance = ultrasonic.cm;
    usleep(500000);
    return distance;
}

//write to the config file
void writeConfig(Config& config){
    ofstream fout;
    fout.open("config.txt");
    fout << "straight = " << config.straight << "\n";
    fout << "left = " << config.left << "\n";
    fout << "right = " << config.right;
}

//load the config file
void loadConfig(Config& config){
    ifstream fin("config.txt");
    string line;
    while (getline(fin, line)){
        istringstream sin(line.substr(line.find("=") + 1));
        if (line.find("straight") != -1)
            sin >> config.straight;
        else if (line.find("left") != -1)
            sin >> config.left;
        else if (line.find("right") != -1)
            sin >> config.right;
    }
}

void writestats(int speed, float voltage, int l2, int r2, bool autonoom){
    ofstream fout;
    fout.open("stats.txt");
    fout << "battery = " << voltage << "\n";
    fout << "left motor power = " << l2 << "\n";
    fout << "right motor power = " << r2 << "\n";
    if(speed == 35){
        fout << "mode = low"<< "\n";
    }else{
        fout << "mode = high"<< "\n";
    }
    if(autonoom == 1) {
        fout << "autonoom = " << "on" << "\n";
    }else{
        fout << "autonoom = " << "off" << "\n";
    }
}

void start_stream(){
    char command[50];

    strcpy( command, "./stream.sh");
    system(command);
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
    bp.set_sensor_type(PORT_3, SENSOR_TYPE_TOUCH_NXT);

    Config config;

    //calibration ultrasoon(y/n)
    string answer;
    cout << "Do you want to calibrate the ultrasoon sensor(recomended at first start)?(Y/n)" << endl;
    cin >> answer;
    if (answer == "yes" || answer == "y" || answer == "Y") {
        string regel;
        cout << "############ Calibration Ultrasoon ############" << endl << "Put the ultrasoon straight and hit enter: " << endl;
        cin >> regel;
        config.straight = bp.get_motor_encoder(PORT_D);
        cout << "Straight = " << static_cast<int32_t>(config.straight) << endl;
        cout << "Put the ultrasoon left and hit enter: " << endl;
        cin >> regel;
        config.left = bp.get_motor_encoder(PORT_D);
        cout << "Left = " << static_cast<int32_t>(config.left) << endl;
        cout << "Put the ultrasoon right and hit enter: " << endl;
        cin >> regel;
        config.right = bp.get_motor_encoder(PORT_D);
        cout << "Right = " << static_cast<int32_t>(config.right) << endl;
        sleep(1);
        bp.set_motor_position(PORT_D, config.straight);
        cout << "Writing to file........ " << endl;
        writeConfig(config);
        cout << "############## Calibration  Done ##############" << endl;

    }else{
        loadConfig(config);
        cout << static_cast<int32_t>(config.straight) << '\n';
        cout << static_cast<int32_t>(config.left) << '\n';
        cout << static_cast<int32_t>(config.right) << '\n';
    }

//    thread t1 (start_stream);

    bool autonoom = 0;
    float speed = 100;
    //Start driving with the controller
    while (true) {
        //Check the controller for inputs
        update();
        int l2 = int(float(getAxis(AXIS_L3Y) * -1) / 32767.0 * speed);
        int r2 = int(float(getAxis(AXIS_R3Y) * -1) / 32767.0 * speed);
        int UD = int(float(getAxis(AXIS_UD) * -1));
        int LR = int(float(getAxis(AXIS_LR) * -1));
        int butA = getButton(BUTTON_A);
        int butB = getButton(BUTTON_B);
        int butX = getButton(BUTTON_X);
        int butY = getButton(BUTTON_Y);
        int lr = getAxis(AXIS_L2);
        float voltBattery = bp.get_voltage_battery();
        float volt9 = bp.get_voltage_9v();
        int32_t position = bp.get_motor_encoder(PORT_D);



        //low and high power mode
        if(butY == 1){
            speed = 35;
        }
        if(butX == 1){
            speed = 100;
        }

        writestats(speed, voltBattery, l2, r2, autonoom);

        //Start for autonoom driving
        if(butA == 1){
            while(true){
                autonoom = 1;
                update();
                int l2 = 50;
                int r2 = 50;
                float voltBattery = bp.get_voltage_battery();
                int butB = getButton(BUTTON_B);
                if(butB == 1){
                    autonoom = 0;
                    break;
                }
                if(bp.get_sensor(PORT_1, ultrasonic) == 0) { //check if the ultrasonic sensor works correctly
                    cout << "sensor working" << endl;
                    int distanceCm = ultrasonic.cm;
                    cout << "Afstand: " << distanceCm << "cm" << endl;
                    if(bp.get_sensor(PORT_3, touch) == 0) {
                        bool pressed = touch.pressed;
                        if(pressed == 1){
                            cout<<"noodstop"<<endl;
                            break;
                        }
                    }
                    if(distanceCm <= 20){
                        bp.set_motor_power(right_motor, 0);
                        bp.set_motor_power(left_motor, 0);
                        int distance_left= look_left(config);
                        int distance_right = look_right(config);
                        cout << "left: " << distance_left << endl << "right: " << distance_right << endl;
                        bp.set_motor_position(PORT_D, config.straight);
                        usleep(100000);
                        bp.set_motor_power(PORT_D, 0);
                        if(distance_left > distance_right){
                            cout<<"----left turn"<<endl;
                            left_turn();
                        }else{
                            cout<<"----right turn"<<endl;
                            right_turn();
                        }
                    }
                }
                bp.set_motor_power(right_motor, r2);
                bp.set_motor_power(left_motor, l2);
                writestats(speed, voltBattery, l2, r2, autonoom);
            }
        }

        if(UD == 32767){
            bp.set_motor_position(PORT_D, config.straight);
        }while(LR == 32767){//links
                update();
                LR = int(float(getAxis(AXIS_LR) * -1));
                int32_t current_position = bp.get_motor_encoder(PORT_D);
                if(current_position <= config.left){
                    break;
                }
                bp.set_motor_position(PORT_D,(current_position -5));
            }
        while(LR == -32767){//rechts
            update();
            LR = int(float(getAxis(AXIS_LR) * -1));
            int32_t current_position = bp.get_motor_encoder(PORT_D);
            if(current_position >= config.right){
                break;
            }
            bp.set_motor_position(PORT_D,(current_position +5));
        }

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