/*
 *
 *  https://github.com/janzuurbier/...
 *
 *  Copyright (c) 2018 Jan Zuurbier
 *
 *
 *  This code is an example for line following.
 *	Due to lack of a light sensor a color sensor is used with only color red.
 *
 *  Hardware: Connect sensor to PORT_1. Motors to PORT_B and PORT_C.
 *
 *  Results:  When you run this program, your vehicle should follow a line.
 *
 *	Video: https://youtu.be/IH7PWprLPkc
 *
 *  Example compile command:
 *    g++ -o program BrickPi3.cpp simplebot3.cpp
 *  Example run command:
 *     ./program
 *
 */

#include "BrickPi3.h" // for BrickPi3
#include <iostream>      // for cin and cout
#include <unistd.h>     // for usleep
#include <signal.h>     // for catching exit signals

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

uint16_t MIN;
uint16_t MAX;
uint16_t IRMIN;
uint16_t IRMAX;
sensor_color_t mycolor;
sensor_light_t light;

//measure reflected light
//if the value is exactly in the middle between MIN and MAX the motors should have equal speed.
//if the value is closer to MIN motor on PORT_B should be a bit slower.
//if the value is closer to MAX motor on PORT_C should be a bit slower.
//return value is between 0 and 100
//0 means value MIN is measured so the sensor is above the line
//100 means value MAX is measured so the sensor is next to the line.

int16_t measureLight() {

	BP.get_sensor(PORT_1, mycolor);
	uint16_t val = mycolor.reflected_red;
	if (val < MIN) val = MIN;
	if (val > MAX) val = MAX;
	return (100*(val - MIN))/(MAX - MIN);
}

uint16_t measureIR() {
    BP.get_sensor(PORT_3, light);
    uint16_t val = light.reflected;
    if (val < IRMIN) val = IRMIN;
    if (val > IRMAX) val = IRMAX;
    return (100*(val - IRMIN))/(IRMAX - IRMIN);
}

int main(){
  signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C

  BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.

  BP.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_RED);
  BP.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);

  //calibrate
//  string regel;
//  cout << "plaats sensor recht boven de lijn (zwart) en voer in a gevolgd door enter" << endl;
//  cin >> regel;
//  BP.get_sensor(PORT_1, mycolor);
//  MIN = mycolor.reflected_red;
//  cout << "MIN = " << MIN << endl;
//  cout << "plaats sensor helemaal naast de lijn (wit) en voer in b gevolgd door enter" << endl;
//  cin >> regel;
//  BP.get_sensor(PORT_1, mycolor);
//  MAX = mycolor.reflected_red;
//  cout << "MAX = " << MAX << endl;
//  cout << "plaats het voertuig met de sensor half boven de lijn en voer in c gevolgd door enter" << endl;
//  cin >> regel;

    MIN = 275;
    MAX = 630;
    IRMIN = 1680;
    IRMAX = 2600;

  //follow line
  int16_t lightval;
  int16_t irval;
  int16_t power = 20;
  while(true){
    lightval = measureLight();
    irval = measureIR();
    cout << irval << endl;
    if (lightval <= 50 && lightval < 5){ // steer left
		BP.set_motor_power(PORT_A, lightval*power/50);
		BP.set_motor_power(PORT_B, power);
	}
	if (lightval > 50 || irval > 5) { // steer right
		BP.set_motor_power(PORT_A, power);
		BP.set_motor_power(PORT_B, (100-lightval)*power/50);
	}
//    usleep(100000);//sleep 100 ms
    sleep(0.1);
  }
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    BP.set_sensor_type(PORT_1, SENSOR_TYPE_NONE);  //Doesn't work, sorry.
    exit(-2);
  }
}
