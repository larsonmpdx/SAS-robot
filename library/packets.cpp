#include "packets.h"
#include <Arduino.h>

//print structure contents
void printInputs(inputs_packet * inputspacket, Print& port)
{
  //analog sticks
  port.print("Lx");
  port.print(inputspacket->left_stick_x);
  port.print("y");
  port.print(inputspacket->left_stick_y);
  port.print("Rx");
  port.print(inputspacket->right_stick_x);
  port.print("y");
  port.print(inputspacket->right_stick_y);
  port.print("\n");
  
  //buttons
  if(inputspacket->select == 0)
    port.print("Select,");
  if(inputspacket->start == 0)
    port.print("Start,");
  if(inputspacket->up == 0)
    port.print("Up,");
  if(inputspacket->down == 0)
    port.print("Down,");
  if(inputspacket->left == 0)
    port.print("Left,");
  if(inputspacket->right == 0)
    port.print("Right,");
  if(inputspacket->button1 == 0)
    port.print("1,");
  if(inputspacket->button2 == 0)
    port.print("2,");
  if(inputspacket->button3 == 0)
    port.print("3,");
  if(inputspacket->button4 == 0)
    port.print("4,");
  if(inputspacket->right_Z1 == 0)
    port.print("RZ1,");
  if(inputspacket->right_Z2 == 0)
    port.print("RZ2,");
  if(inputspacket->left_Z1 == 0)
    port.print("LZ1,");
  if(inputspacket->left_Z2 == 0)
    port.print("LZ2,");
  if(inputspacket->left_stick_button == 0)
    port.print("L_STICK,");
  if(inputspacket->right_stick_button == 0)
    port.print("R_STICK");
  port.print("\n");
}
