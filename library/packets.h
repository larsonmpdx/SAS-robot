#ifndef ROBOT_PACKETS_H
#define ROBOT_PACKETS_H

#include <Arduino.h>

#define INPUTS_PACKET_ID  123

struct inputs_packet {
  uint8_t ID                : 8;  //byte 1
  bool select               : 1;  //DIN: bytes 2,3	bit 1
  bool start                : 1;  //bit 2
  bool up                   : 1;  //3
  bool down                 : 1;  //4
  bool left                 : 1;  //5
  bool right                : 1;  //6
  bool button1              : 1;  //7
  bool button2              : 1;  //8
  bool button3              : 1;  //9
  bool button4              : 1;  //10
  bool right_Z1             : 1;  //11
  bool right_Z2             : 1;  //12
  bool left_Z1              : 1;  //13
  bool left_Z2              : 1;  //14
  bool left_stick_button    : 1;  //15
  bool right_stick_button   : 1;  //16
  
  uint16_t left_stick_x     : 10; //bytes 4-8
  uint16_t left_stick_y     : 10;
  uint16_t right_stick_x    : 10;
  uint16_t right_stick_y    : 10;
};

//todo: add some kind of compile-time assert that the above struct is actually 8 bytes

//note that using a struct as an on-wire data type is not portable between architectures or even compilers,
//because struct layout in memory is not defined in C.
//this is should be viewed as a hack that is only appropriate for a well-defined case -
//for example two arduinos talking to each other

//print a nicely-formatted output of the structure's contents
void printInputs(inputs_packet * inputspacket, Print& port);

#endif	//ROBOT_PACKETS_H
