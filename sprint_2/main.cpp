#include "joystick.h"
#include "BrickPi3.h"
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
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
    //Start driving with the controller
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
                        int distance_left= look_left(config);
                        int distance_right = look_right(config);
                        cout << "left: " << distance_left << endl << "right: " << distance_right << endl;
                        bp.set_motor_position(PORT_D, config.straight);
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