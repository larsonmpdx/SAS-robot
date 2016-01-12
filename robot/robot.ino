/*
  // Robot sketch
  // based on Andrew Rapp's xbee-arduino samples (due to the copyleft-nature of this library's license, that this makes this code GPL too)
  // code is set up for a leonardo with an xbee attached to D0/D1 and a sabertooth 2x5 motor controller attached to D11 in simplified serial mode, 9600baud
  // this sketch should ideally be run on an arduino with two serial ports (a leonardo is perfect)
  // so that serial debug output is available (the xbee normally connects to D0/D1 serial, which on an UNO is the same serial port used for debugging)
  // alternatively, an xbee shield that allows the xbee to be connected to different pins will allow an UNO to use SoftwareSerial for the xbee and D0/D1 for debug output
*/

#include <SoftwareSerial.h>
#include <SabertoothSimplified.h>
#include <XBee.h>
#include <packets.h>    //shared header so controller and robot can send/receive a packet with the current input states

SoftwareSerial SWSerial(NOT_A_PIN, 11); // Sabertooth 2x5 serial port: RX unused, TX on pin 11 is connected to sabertooth motor controller's S1
SabertoothSimplified ST(SWSerial);      // set serial port for sabertooth control library

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();


/*
   this file includes methods to read from both series 1 and series 2 xbees
   this results in some extra code, but it makes the program more flexible
   the entirety of the loop() function is spent waiting for and processing xbee API mode packets,
   then calling updateRobotOutputs() with either true for false depending on whether the control inputs are valid
*/

//series 2 data structure
ZBRxResponse rx = ZBRxResponse();

//series 1 data structures
Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();

//both series 1 and series 2 use this for some modem events
ModemStatusResponse msr = ModemStatusResponse();

inputs_packet* inputspacket;    //pointer to interpret data received from gamepad.  see packets.h

void setup()
{
  //serial port for the dimension engineering sabertooth 2x5
  SWSerial.begin(9600);

  //serial ports note: a leonardo will use "Serial" as its debug serial port (available on USB), and "Serial1" (D0/D1) as its xbee serial port
  //an UNO with a cape that is able to put the xbee serial port on a pair of D pins (not D0/D1) can use another SoftwareSerial port for the xbee and use Serial as the USB debug port
  //an UNO with a cape that can only put the Xbee on D0/D1 serial will not be able to use USB debug serial while the xbee is connected.  this is not recommended

  //debugging serial port (USB)
  Serial.begin(9600);

  //serial port for the XBee module on leonardo Serial1 (D0/D1)
  Serial1.begin(9600);
  xbee.setSerial(Serial1);

  updateRobotOutputs(false);   //call robot settings update with false to trigger a "no inputs->safe settings" update
}

void loop()
{
  if (xbee.readPacket(500))   //wait 500ms for a packet - if one doesn't come in time, consider the inputs stale and go into safe mode
  {
    if (xbee.getResponse().isAvailable()) {

      switch (xbee.getResponse().getApiId())
      {
        case ZB_RX_RESPONSE:          //series 2 packet
          xbee.getResponse().getZBRxResponse(rx);

          //series 2 xbees are able to tell the receiver (robot) whether or not the transmitter (gamepad) got an ACK
          //nothing is done with this information in this example; this just shows how to check
          if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
            // the sender got an ACK
            // flashLed(statusLed, 10, 10);
          } else {
            // we got it (obviously) but sender didn't get an ACK
            // flashLed(errorLed, 2, 20);
          }

          //optional: check remote address
          //rx.getRemoteAddress64();

          process_packet(rx.getData(), rx.getDataLength());
          break;
        case RX_16_RESPONSE:          // series 1 packet with 16 bit addressing
          xbee.getResponse().getRx16Response(rx16);

          //optional: check remote address
          //rx16.getRemoteAddress16();

          process_packet(rx16.getData(), rx16.getDataLength());
          break;
        case RX_64_RESPONSE:          // series 1 packet with 64 bit addressing
          xbee.getResponse().getRx64Response(rx64);

          //optional: check remote address
          //rx64.getRemoteAddress64();

          process_packet(rx64.getData(), rx64.getDataLength());
          break;
        case MODEM_STATUS_RESPONSE:
          // the local XBee sends this response on certain events, like association/dissociation
          //nothing is done with this information in this example; this just shows how to check
          xbee.getResponse().getModemStatusResponse(msr);
          if (msr.getStatus() == ASSOCIATED) {
            // yay this is great.  flash led
            // flashLed(statusLed, 10, 10);
          } else if (msr.getStatus() == DISASSOCIATED) {
            // this is awful.. flash led to show our discontent
            // flashLed(errorLed, 10, 10);
          } else {
            // another status
            // flashLed(statusLed, 5, 10);
          }
          break;
        default:
          Serial.print("unexpected packet\n");
          break;
      }
    }//.getResponse is not available
    else if (xbee.getResponse().isError())
    {
      Serial.print("Error reading packet.  Error code: ");    //note: error 3 / invalid parameter could mean the xbee is in API mode AP=1 when it should be in AP=2
      Serial.println(xbee.getResponse().getErrorCode());
    }
  }
  else  //timed out reading, or error on read
  {
    if (xbee.getResponse().isError())
    {
      Serial.print("Error reading packet.  Error code: ");    //note: error 3 / invalid parameter could mean the xbee is in API mode AP=1 when it should be in AP=2
      Serial.println(xbee.getResponse().getErrorCode());
    }
    else
    {
      Serial.print("timed out\n");
      updateRobotOutputs(false);    //run inputs update with valid = false (safe mode)
    }
  }
}

//the xbee library provides three separate functions to retreive packets depending on type: series 1 16 bit, series 1 64 bit, and series 2
//the type of packet received is decoded in loop(), then this function is called after the packet is processed by the xbee library
//parameters: the packet's start address and data length
void process_packet(uint8_t* data, int data_length)
{
  //in this example, there is only one packet type
  //different packet types are provided for by using the first byte as a packet identifer
  //once the packet has been sorted by its first byte, its length is checked against the known length of that packet type
  //and its uint8_t pointer is cast to a pointer for the correct packet type
  switch (data[0])
  {
    case INPUTS_PACKET_ID:
      {
        //check data length
        if (data_length != sizeof(inputs_packet))
        {
          Serial.print("bad length\n");
          break;
        }
        //if we get here, the packet ID checks out, it's the correct size, and (optional check in the caller) the sender's address matches
        inputspacket = (inputs_packet*)(data);
        updateRobotOutputs(true); //calling with "true" means the data in inputspacket is valid
        break;
      }
    default:
      Serial.print("unknown packet type\n");
      break;
  }
}

//update the outputs
//inputs_valid tells this function whether the inputsvalid global has good data or not
//if called with false, the robot should be set to safe settings

// analog values are 10 bit/ 0-1023.  With the sticks centered all values will be somewhere around 512
// the analog stick buttons are read using analog inputs, but they're converted to digital values at the gamepad before transmission
// all digital inputs are active low (0=on, 1=off).  This includes the analog stick buttons

//find the names of additional gamepad inputs inside /library/packets.h
//this file contains the definition of the data structure passed between gamepad and robot
//the robot always has access to all gamepad inputs, as they're all sent at once whether they're used or not
//this reduces the need to change gamepad or radio software; normally this function is all that will need to be changed to create a new robot

void updateRobotOutputs(bool inputs_valid)
{
  if (inputs_valid == false)
  {
    //place safe robot settings in case of control interruption here
    //this is called after a timeout in the xbee handling code in loop()
    //in this example, both motors are controlled in speed mode and are set to speed 0
    ST.motor(1, 0);
    ST.motor(2, 0);
  }
  else
  {
    printInputs(inputspacket, Serial);    //print the received inputs on debug serial.  this function is located in packets.cpp

    //set motor based on stick positions:  This math converts the 0-1023 analog stick values to motor settings, which are -127 to +127 (full reverse to full forward).
    ST.motor(1, (inputspacket->left_stick_x / 4) - 128);
    ST.motor(2, (inputspacket->right_stick_x / 4) - 128);
  }
}
