#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <signal.h>
#include <cstdlib>
#include "BrickPi3.h"

using namespace std;

BrickPi3 bp; //check if the brickpi is connected

//specify what sensors are connected
sensor_color_t color;
sensor_light_t light;
sensor_ultrasonic_t ultrasonic;

int power = 60; //speed variable

void exit_signal_handler(int signo);

//calculate light brightness
int16_t measure_light(int16_t val, uint16_t MIN, uint16_t MAX) {
    if (val < MIN) val = MIN;
    if (val > MAX) val = MAX;
    return (power*(val - MIN))/(MAX - MIN);
}

//calculate color brightness
int16_t measure_color(int16_t val, uint16_t MIN, uint16_t MAX) {
    if (val < MIN) val = MIN;
    if (val > MAX) val = MAX;
    return (power*(MAX - val))/(MAX - MIN);
}

int million = 1000000; //usleep is in microseconds, in x*million is x in secconds

void avoid_object(){ //if an object is detected, go around it
	bp.set_motor_power(PORT_A, 55);
	bp.set_motor_power(PORT_B, -55);
	usleep(1 * million);
	bp.set_motor_power(PORT_A, 50);
	bp.set_motor_power(PORT_B, 50);
	usleep(1 * million);
	bp.set_motor_power(PORT_A, -55);
	bp.set_motor_power(PORT_B, 55);
	usleep(1 * million);
	bp.set_motor_power(PORT_A, 50);
	bp.set_motor_power(PORT_B, 50);
	usleep(2.5 * million);
	bp.set_motor_power(PORT_A, -55);
	bp.set_motor_power(PORT_B, 55);
	usleep(1 * million);
	bp.set_motor_power(PORT_A, 50);
	bp.set_motor_power(PORT_B, 50);
	usleep(1 * million);
	bp.set_motor_power(PORT_A, 50);
	bp.set_motor_power(PORT_B, -50);
	usleep(1 * million);
	bp.set_motor_power(PORT_A, 0);
	bp.set_motor_power(PORT_B, 0);
}

void intersect(){
	bp.set_motor_power(PORT_A, 20);
	bp.set_motor_power(PORT_B, 20);
	usleep(1 * million);
	if (rand() % 2) {	//the rabot chooses at random if it will go to the left or the right
		//random number is 1, go to the right
		bp.set_motor_power(PORT_A, -40);
		bp.set_motor_power(PORT_B, 40);
	}else{
		//random number is 0, go to the left
		bp.set_motor_power(PORT_A, 40);
		bp.set_motor_power(PORT_B, -40);
	}
	usleep(1 * million);
}

int main() {
    signal(SIGINT, exit_signal_handler);

    bp.detect(true);

	//set motor limits
    bp.set_motor_limits(PORT_A, power, 0);
    bp.set_motor_limits(PORT_B, power, 0);

	//specify whitch sensor is coneccted to witch port
    bp.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_FULL);
    bp.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);
    bp.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);

	//sensor values
    uint16_t left_min = 1710;
    uint16_t left_max = 2600;
    uint16_t right_min = 230;
    uint16_t right_max = 627;

	//light values
    int16_t leftval;
    int16_t rightval;

	//to check how long the robot lost the line
    int zerocount = 0;

	//motor values
    int8_t apower;
    int8_t bpower;

	//to check witch way to turn when the line is lost
    bool left = false;
    bool right = false;

	//indecates the robot lost the line
    bool alarm = false;

    while(true){
		//check if the ultrasonic sensor works correctly
        if(bp.get_sensor(PORT_2, ultrasonic) == 0){
            int distanceCm = ultrasonic.cm;
            if(distanceCm <= 5){
                avoid_object;
            }
        }

		//check if the color and light sensors work correctly
        if (bp.get_sensor(PORT_1, color) == 0 && bp.get_sensor(PORT_3, light) == 0){
            //calculating light values
            leftval = measure_light(light.reflected, left_min, left_max);
            rightval = measure_color(color.reflected_red, right_min, right_max);

			//checks if the last seen line was left or right
            if (leftval - rightval > 10){
                left = true;
                right = false;
                alarm = false;
            }else if (leftval - rightval < -10){
                right = true;
                left = false;
                alarm = false;
            }

			//if the robot lost the line, it ups the counter
            if (leftval <= 0 && rightval <= 0) {
                zerocount++;
				//if the counter reaches 10, the robot has lost the line
                if (zerocount >= 10) {
                    alarm = true;
                    cout << "ALARM" << endl;
                    cout << right << ", " << left << endl;
                }
            }else{	//if the robot finds the line the counter resets to 0
                zerocount = 0;
            }

			//if both sensors output a value higher than 57, the robot is on an intersection
            if (leftval >= 57 && rightval >= 57) {
                intersect;
            }else if(alarm){ //checks if the robot has lost the line
                if (left) { //if the last seen line was left, turn left
                    apower = 40;
                    bpower = -40;
                }
                if (right) { //if the last seen line was right, turn right
                    apower = -40;
                    bpower = 40;
                }
            }else{ //go forward, follow the line
                apower = power - (rightval);
                bpower = power - (leftval);
            }

			//turn the motor on with te specified power
            bp.set_motor_power(PORT_A, apower);
            bp.set_motor_power(PORT_B, bpower);

            cout << leftval << ", " << rightval << endl;
            cout << "power: " << static_cast<int16_t>(apower) << ", " << static_cast<int16_t>(bpower) << endl;
        }
    }
}

void exit_signal_handler(int signo){
    if(signo == SIGINT){
        bp.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}