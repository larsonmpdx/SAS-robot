/*
  // DFRobot arduino gamepad sketch
  // based on DFRobot gamepad example code and Andrew Rapp's xbee-arduino samples (due to the copyleft-nature of this library's license, that this makes this code GPL too)
  // compile as leondardo
  // NOTE: when loading using the micro usb adapter board, ensure the board is in the correct orientation.  The USB port may be damaged if it's upside down
  // if the USB port becomes damaged it will be necessary to program the board using the ICSP port inside the gamepad, and serial debug output won't be available
*/

#include <XBee.h>
#include <packets.h>    //shared header so gamepad and robot can send/receive a packet with the current input states

//inputs
//note the "analog" button (center of gamepad, right above LEDs) is a reset button for the internal arduino
#define SELECT_DIN      3
#define START_DIN       4

#define UP_DIN          5
#define DOWN_DIN        6
#define LEFT_DIN        7
#define RIGHT_DIN       8

#define BUTTON1_DIN     9
#define BUTTON2_DIN     11
#define BUTTON3_DIN     12
#define BUTTON4_DIN     10

#define RIGHT_Z1_DIN    13
#define RIGHT_Z2_DIN    14
#define LEFT_Z1_DIN     15
#define LEFT_Z2_DIN     16	//#17 (0-referenced)

#define NUM_DIN_INPUTS  17

#define LEFT_STICK_BUTTON_AIN   A1
#define LEFT_STICK_X_AIN        A5
#define LEFT_STICK_Y_AIN        A4

#define RIGHT_STICK_BUTTON_AIN  A0
#define RIGHT_STICK_X_AIN       A3
#define RIGHT_STICK_Y_AIN       A2

//outputs
//note the MODE LED is not user-controlled - it is same as the power LED on an arduino
#define SHAKE_MOTOR_OUT   2
#define RX_LED_OUT        17

inputs_packet inputspacket;

/*
  The Xbee library uses different structures for series 1 / series 2 xbees
  in order to be flexible, this code includes both structures.  the only difference is in sending the packet - comment out the series not in use
  the reply packet will automatically be parsed as either series 1 or series 2

  the xbee must be in AP=2 (API mode with escaping)   note: for a series 2 xbee, this means the API mode firmware must be loaded.  Series 1 xbees have only one firmware that covers both transparent and api mode
  AP=1 (API mode without escaping) will not work (and it can give very odd errors)
  AP=0 (transparent mode) will not work
  both Xbees must have the same PAN ID

  use an xbee-explorer and X-CTU to change xbee settings

  the gamepad gets programmed with the address high/address low of the robot
  this is printed on the sticker on the bottom of the xbee, or it can be viewed in X-CTU

  the robot accepts any commands sent to it that make it past its address filters
  the messages that make it past will depend on the receiving xbee's MY address and PAN ID
  normally, with a MY address set, there are many possible 16-bit addressing methods that will send a message
  to the robot, some that could be guessed.  to disable 16-bit addressing, set MY=0xFFFF on the receiver.
  This will require that the sender know the full 64-bit address of the receiver.
  A worthwhile exercise would be to implement auto-pairing, or another method to ensure robots do not accept commands
  from gamepads other than the originally-configured gamepad (it is possible to check the address of the sender)
*/

// create the XBee object
XBee xbee = XBee();

// SH + SL Address of receiving XBee (hex numbers, find through X-CTU or the sticker on the destination xbee (robot xbee))
//SH: address high  13A200 (example)
//SL: address low 40DB7E0A (example)
//XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40DB7E0A); (example)
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40DB7E0A);

//series 1 64 bit request
Tx64Request tx = Tx64Request(addr64, (uint8_t *)&inputspacket, sizeof(inputspacket));

//series 2 64 bit request
ZBTxRequest zbTx = ZBTxRequest(addr64, (uint8_t *)&inputspacket, sizeof(inputspacket));

//series 1 response
TxStatusResponse s1txStatus = TxStatusResponse();

//series 2 response
ZBTxStatusResponse s2txStatus = ZBTxStatusResponse();


void setup()
{
  //because the DFR gamepad is leonardo-based, "Serial" is the USB debug serial port, and "Serial1" is the D0/D1 serial port (connected to the xbee host socket)
  Serial.begin(57600);  // USB serial (debug)
  Serial1.begin(9600);  // D0/D1 serial (xbee)
  xbee.setSerial(Serial1);
  InitIO();             // Initialize the inputs/outputs and the buffers
  inputspacket.ID = INPUTS_PACKET_ID;   //packet identifier for this packet (first byte, allows sorting of different packet types)
}

void InitIO() {
  for (int i = 0; i < NUM_DIN_INPUTS; i++) pinMode(i, INPUT);
  pinMode(SHAKE_MOTOR_OUT, OUTPUT);
  digitalWrite(SHAKE_MOTOR_OUT, LOW);  // don't shake
  pinMode(RX_LED_OUT, OUTPUT);
  digitalWrite(RX_LED_OUT, HIGH);      // default: off
}

//return value of millis() is unsigned long - make sure to use unsigned long variables when doing math with it
unsigned long last_usb_debug_print = 0;
unsigned long last_update = 0;

void loop()
{
  //this block provides rate limiting to the controller's update-and-transmit loop
  //Without this, the loop will attempt to send xbee packets as fast as they are acknowledged, using up battery power and shared radio bandwidth
  //using this delay, the max packet rate will be 1/50ms = 20Hz,
  //the sketch will run fine with zero delay but would provide needlessly fast control updates for most applications

  unsigned long time_since_last_update = millis() - last_update;  //calculate this once with a temp variable so that there is no race condition in the next block
  if (time_since_last_update < 50)
  {
    delay(50 - time_since_last_update);
  }
  last_update = millis();

  updateData();   //update button/joystick values and place them in inputspacket, which is pointed to by the xbee packet tx or zbTx

  //separate block to limit USB serial printing to every 500ms
  if (millis() - last_usb_debug_print > 500) {
    printInputs(&inputspacket, Serial);   //this function is in packets.cpp.  it prints the values of all sticks/buttons
    last_usb_debug_print = millis();
  }

  //choose which type of packet to send based on the xbee series in use (series 1 or series 2).  comment out the unused type
  //this is the only change required - the reply from the xbee is differentiated based on the API header

  //send packet (series 1)
  xbee.send(tx);

  //send packet (series 2)
  //  xbee.send(zbTx);

  // after sending a tx request, we expect a status response
  // wait up to 250ms for status response
  // responses will generally come more slowly with increased range or lower battery power, slowing down the rate of control updates provided to the robot
  // if this times out, but the robot is still receiving updates, it will result in the green "RX" LED going out, which is usually the first indication of low batteries
  // in my tests, the ACK delay increased to 100ms at 4.0V and 250ms at 3.8V.  Around 3.6V control was noticeably degraded
  if (xbee.readPacket(250))
  {
    //success - xbee module replied within the timeout period
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
    {
      // this is an xbee series 2 response
      xbee.getResponse().getZBTxStatusResponse(s2txStatus);
      Serial.print("series 2 reply");

      // get the delivery status, the fifth byte
      if (s2txStatus.getDeliveryStatus() == SUCCESS)
      {
        // success
        analogWrite(RX_LED_OUT, 0); //set RX LED on
      }
      else
      {
        // the remote XBee did not receive our packet. is it powered on?
        analogWrite(RX_LED_OUT, 240); //set RX LED off
      }
    }
    else if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE)
    {
      //this is an xbee series 1 response
      xbee.getResponse().getTxStatusResponse(s1txStatus);
      Serial.print("series 1 reply");

      // get the delivery status, the fifth byte
      if (s1txStatus.getStatus() == SUCCESS)
      {
        // success
        analogWrite(RX_LED_OUT, 0); //set RX LED on
      }
      else
      {
        // the remote XBee did not receive our packet. is it powered on?
        analogWrite(RX_LED_OUT, 240); //set RX LED off
      }
    }
  }
  else    //readPacket() timed out
  {
    if (xbee.getResponse().isError())
    {
      Serial.print("Error reading packet.  Error code: ");
      Serial.println(xbee.getResponse().getErrorCode());
    }
    else
    {
      Serial.print("timed out");
      analogWrite(RX_LED_OUT, 240); //set RX LED off
    }
  }
}

//updateData(): collect the button and stick states and place them into the packet that will be transmitted
//packet definition is in packets.h
void updateData()
{
  inputspacket.select              = digitalRead(SELECT_DIN);
  inputspacket.start               = digitalRead(START_DIN);

  inputspacket.up                  = digitalRead(UP_DIN);
  inputspacket.down                = digitalRead(LEFT_DIN);
  inputspacket.left                = digitalRead(DOWN_DIN);
  inputspacket.right               = digitalRead(RIGHT_DIN);

  inputspacket.button1             = digitalRead(BUTTON1_DIN);
  inputspacket.button2             = digitalRead(BUTTON2_DIN);
  inputspacket.button3             = digitalRead(BUTTON3_DIN);
  inputspacket.button4             = digitalRead(BUTTON4_DIN);

  inputspacket.right_Z1            = digitalRead(RIGHT_Z1_DIN);
  inputspacket.right_Z2            = digitalRead(RIGHT_Z2_DIN);
  inputspacket.left_Z1             = digitalRead(LEFT_Z1_DIN);
  inputspacket.left_Z2             = digitalRead(LEFT_Z2_DIN);

  inputspacket.left_stick_button   = analogRead(LEFT_STICK_BUTTON_AIN);    //this is an analog-to-digital conversion
  inputspacket.right_stick_button  = analogRead(RIGHT_STICK_BUTTON_AIN);   //this is an analog-to-digital conversion
  inputspacket.left_stick_x        = analogRead(LEFT_STICK_X_AIN);
  inputspacket.left_stick_y        = analogRead(LEFT_STICK_Y_AIN);
  inputspacket.right_stick_x       = analogRead(RIGHT_STICK_X_AIN);
  inputspacket.right_stick_y       = analogRead(RIGHT_STICK_Y_AIN);
}
