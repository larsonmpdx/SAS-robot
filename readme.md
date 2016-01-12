## overview ##
* This repository contains code for the arduino-based DFRobot gamepad v2+xbee and a matching program for an arduino-based robot with an xbee.  Together, these programs enable simple remote control projects in which both the controller and vehicle are programmable as arduinos and communicate using their xbees.  This project was commissioned by the Singapore American School's robotics program to provide a hardware kit for their intro course.  The kit cost is relatively low because of the use of off-the-shelf arduino and xbee parts, about $225US for the gamepad, robot arduino, xbee shield, two 60mW xbees, and motor controller in 2015.

## operating instructions ##
* With a working default system: turn on both the robot and gamepad.  The gamepad's "RX" led (green) indicates connection status with the robot's xbee.  Connection status is updated continuously: if the robot goes out of range, the LED will go off.  If it comes back in range, it will turn back on.
* if the gamepad has low batteries, normally the green "RX" led will no longer stay solid
* the robot will follow gamepad commands to move motor 1/motor 2 according to the analog stick x-axes, but after 500ms of no valid packets it will go into a safe mode
* both gamepad and robot will output debug messages on their USB serial ports containing the status of all buttons and the analog sticks at regular intervals, or a "timed out" or error message

## hardware setup ##

# gamepad #
* DFRobot arduino+xbee gamepad v2.  see <http://www.dfrobot.com/wiki/index.php/Wirless_GamePad_V2.0_(SKU:DFR0182)>
* the gamepad acts as an xbee leonardo for programming.  the "analog" button on the front is the reset button, and the red "mode" LED is the normal arduino power LED.  The "RX" LED is user programmable.  inside is an xbee slot (after removing all of the rear screws)
* **the 60mW series 1 xbee is recommended because it's simple to set up, has low operating current and good range**.  A 2mW series 1 xbee on both sides resulted in about 10 feet of range.  A series 2 xbee will also work.  With additional code, an xbee-compatible module that isn't series 1 or series 2 could be made to work.  Use an xbee with a flexible wire antenna, this is the best antenna option that will fit inside the gamepad.
* the gamepad's xbee can have its flexible wire antenna bent up at an angle away from the circuit board, maybe 30*, and still fit inside.  This will give better range than lying flat

# robot #
* The robot comprises an Ardunio, xbee shield, xbee, motor controller, DC motors, and a 12V battery
* it is very helpful to use an arduino with its USB serial port available for debugging.  Because the xbee is accessed using a serial port, this rules out using an UNO with a basic xbee shield, because basic xbee shields only allow connecting the xbee to D0/D1 serial, the UNO's usb-connected serial port
* option one **(recommended)**: use a leonardo or another arduino with two hardware serial ports.  This lets the xbee be connnected to D0/D1 while still retaining USB serial debug output.  On a leonardo, D0/D1 is "Serial1", and USB serial is just "Serial".  The default robot code is set up with a leonardo's serial ports in mind.  DFRobot makes a leonardo with an integrated xbee slot that works well
* option two: use an UNO, but select an xbee cape that allows connecting the xbee to pins other than D0/D1
* the xbee type should match the type in the gamepad (both series 1 or both series 2, and same power amount or else range might be limited to that of the lower-power module, because of xbee's 2-way nature).  The onboard arduino power supply is limited in the current it can supply to the xbee by a heat dissipation limit on its voltage regulator; a series 1 60mW xbee (as used in the example) that's mostly receiving is near the limit.  Take care if using a higher power xbee (series 2 63mW for example) or if transmitting very much from the robot.  A separate arduino power supply could be added to support these more powerful options; the less voltage drop that has to occur in the arduino, the more current it can supply to the xbee.
* the robot's xbee should have either a flexible wire antenna (as in the gamepad) or external antenna.  The onboard chip antenna has less range than these options.  If range is important, an external antenna and coax can be carefully chosen and placed high up on the robot away from any metal obstructions.  Alternatively, the entire arduino+xbee unit could be moved to a better location to better position a flexible wire antenna
* motor controller: dimension engineering sabertooth 2x5 (2 channels, 5A each).  This controller was selected for its easy serial interface, moderate cost, and built-in protection features while having a high enough current rating to drive the tetrix DC gear motors.  There are many other possible controllers that could work with an arduino.  <http://www.dimensionengineering.com/products/sabertooth2x5>
* 2x tetrix DC gear motors.  <http://www.pitsco.com/TETRIX_DC_Gear_Motor>
* 12V input from a lead acid battery powers both the ardunio (through the power port) and the sabertooth 2x5.  The connection is made by using the sabertooth's input block to hold two pairs of wires
* a fuse is recommended, inline with the battery.  5-10A fast blow or 2-5A slow blow would probably be appropriate depending on the robot

## quick start guide ##

# xbee setup #
* the xbees initially need to be set up using a computer and an xbee-explorer.  the xbee-explorer is a board that lets you connect a computer to the xbee's serial port for programming.  Use digi's X-CTU software to program the xbees.  important settings:
 * API mode: **`AP=2` is required.  `AP=1`/`AP=0` will not work.**  For a series 2 xbee, first the API firmware must be loaded using X-CTU (series 2 xbees normally come from the factory with transparent mode firmware).  Series 1 xbees have only one firmware type that supports both.  Using the wrong AP setting can give odd errors, especially AP=1
 * PAN ID: **pick the same PAN ID for both robot and gamepad**.  I used 0
* normally, one xbee is chosen to be the `coordinator` and one to be an `end device`.  I don't know if this actually matters in a simple setup.  I made the robot the `coordinator`
* all other settings can be default
* optional: to disable receipt of 16-bit messages on the robot, set `MY=0xFFFF` on the robot's xbee.  This will ensure that only 64-bit addressed messages are received, possibly limiting interference.  default is MY=0
* **note the address of the xbee that is going in the robot**.  this address will be loaded into the gamepad's arduino sketch so that the gamepad only sends commands to this robot.  In this way many gamepads and robots can be in use at one time and not interfere with each other.  The address can be obtained through X-CTU or by looking at the sticker on the back of the xbee.  See the gamepad arduino sketch for an example address.  The address is printed in hexadecimal (using characters 0-9 and A-F)

# required libraries #
* xbee-arduino 0.6.0 or newer - can be obtained from github or (recommended) through the arduino library search: `Sketch -> include library -> manage libraries`, search for xbee-arduino
* dimension engineering arduino serial control library (needed for robot only).  download this from the dimension engineering website at <http://www.dimensionengineering.com/info/arduino> (retreived December 2016).  The example uses the sabertooth 2x5 motor controller and the sabertooth simplified serial library.  It is possible to change to the packetized serial library for more control.  place the library like this: `/Documents/Arduino/libraries/SabertoothSimplified/`
* internal library: packets.h/packets.cpp.  Find the file inside `/library/` in this repository.  Place this inside the arduino libraries directory like this: `/Documents/Arduino/libraries/robot/packets.h and packets.cpp`.  This lets both gamepad and robot share the packet definition and the debug printing function

# code customization #
* the only required code change is to put the hardware ID of the receiving xbee in the gamepad's arduino sketch.  This goes in this block (only the last line needs to be edited.  The other lines are comments, an example of how to format the address):

`// SH + SL Address of receiving XBee (hex numbers, find through X-CTU or the sticker on the destination xbee (robot xbee))
//SH: address high  13A200 (example)
//SL: address low 40DB7E0A (example)
//XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40DB7E0A); //(example)
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40DB7E0A);`

* find the address on the robot's xbee sticker, or through X-CTU.  The comments show how the address is changed into code: add 0x to the front to let the compiler know it's a hexadecimal number, and pad out the address high with 0s to make it 4 bytes long (optional)
* optional change: change the behavior of the robot by editing the `updateRobotOutputs()` function in the robot arduino sketch.  This is the function that takes the inputs received from the gamepad and decides how to apply them to the robot's outputs (in the example, two motors are controlled in speed mode by the X-axes of the left and right sticks on the gamepad)
* analog values are 10 bit/ 0-1023.  With the sticks centered all values will be somewhere around 512
* the analog stick buttons are read using analog inputs, but they're converted to digital values at the gamepad before transmission
* all digital inputs are active low (0=on, 1=off).  This includes the analog stick buttons

# uploading sketches #
* the gamepad includes a tiny adapter to plug in a micro usb cable to the pin header on the back.  The correct orientation is on the DFRobot wiki ( with the gamepad's handles down, the circuit board should be "down", connectors "up").  **the gamepad is programmed as a leonardo**
* **incorrect orientation of the usb adapter board might render the gamepad's usb port unusable** (ask me how I know!).  if this happens, there is still an ICSP header inside the controller that can be programmed using another arduino.  Find instructions for this on the internet -the ICSP header does work fine, but once the USB port is dead you won't be able to use serial debugging on the gamepad which is a major hindrance
* ICSP programming instructions (if your gamepad's USB is dead)
 1. load "arduinoisp" onto an UNO (it's in the examples menu under file-> in the arduino IDE)
 2. turn off the gamepad, open it up, and connect up to the ICSP header.  find a connection diagram online
 3. in the arduino IDE, open the gamepad sketch.  connect to the UNO's usb port.  select programmer "arduino as ISP" and select the port of the UNO.  then run sketch->"upload using programmer" (NOT "upload")
 4. this takes a bit longer than usb programming.  you'll see the SPI LEDs flash on the programmer UNO to confirm progress
* robot arduino programming: this is normal, depending on the xbee shield
* Ideally you are using an xbee shield that doesn't connect the xbee to the USB serial port, so there are no special steps.  Some basic xbee shields do connect to D0/D1 and these will have an "xbee/usb" switch.  This is because the arduino UNO only has one serial port, and an xbee connected to this serial port (in the "xbee" setting) will block programming.  To program the arduino, disconnect the xbee by setting the switch to "usb".  **In the UNO example, the xbee is connected to the regular usb serial port and the switch needs to be moved to "xbee" for operation and "usb" for arduino programming.  there is also an example leonardo with an integrated xbee slot; this requires no special steps**
* The xbee needs to be removed from the arduino and placed on an xbee-explorer in order to use X-CTU to change xbee settings with a computer

# dimension engineering sabertooth 2x5 motor controller setup #
* see <http://www.dimensionengineering.com/products/sabertooth2x5> for documentation
* the dimension sabertooth 2x5 can be controlled in a variety of ways.  Simplified serial was chosen for ease of use.  This requires a connection from any pin that can run an arduino software serial.  **In the example, arduino D11 (SoftwareSerial) is wired to S1 on the sabertooth 2x5.**  The sabertooth and arduino need a shared ground connection for this to work; in the example it's provided by the single shared power supply.  The sabertooth is set to 9600 baud and is RX-only, so the arduino can be TX-only
* settings used in the example: (see <https://www.dimensionengineering.com/datasheets/SabertoothDIPWizard/nonlithium/serial/simple/single.htm>)
`9600baud simple serial
1 up/on
2 down
3 up
4 down
5 up
6 up`

## software overview ##
* both gamepad and robot contain extra code to support both series 1 and series 2 xbees.  This adds some size to the project but affords flexibility.  The only edit required to change to a different series xbee is switching which xbee.send() command is called in the gamepad sketch, using either tx (series 1) or zbTx (series 2).  **the default code is for a series 1 xbee**
* the gamepad is set up with the unique hardware ID of the robot's xbee.  The robot is set up to accept any commands it receives.  This was done for simplicity; a different strategy (for example, auto-pairing) could be implemented in software.
* the gamepad sends its entire button and analog axis state at regular intervals (this is 8 bytes).  This way, the gamepad and radio code never has to change to support different robot styles, as the robot always has all control information.
* the robot and gamepad arduino programs share the struct definition of the input state packet.  The packet is defined by its first byte; additional packet types could be added by defining other first bytes and implementing a switch statement
* the gamepad runs a loop: update input states, send the packet, and check for packet receipt (receipt checking is a function provided by the xbee).  Based on the receipt state, it turns the "RX" LED on or off.  For debugging, the gamepad outputs some data to the arduino usb debug serial port every 500ms.  Serial.print() can be used in other parts of the program as on any arduino for this purpose.
* the robot runs its own loop: wait for and receive a packet, and then run a function that maps the input state to control outputs.  Currently this maps the Y-axis on each stick to a different motor.  This occurs inside the updateRobotOutputs() function.  The updateRobotOutputs() has two states: if there is valid input, it applies its normal control mapping.  If there is no valid input (meaning, the gamepad is not in radio contact anymore), it applies a "safe" output state to the robot.  In the example, the safe state is both motors at 0 speed.

## debugging ##
* both gamepad and robot sketch can be configured to output debug values to the usb serial port, "Serial".  Connecting to these serial ports will usually give good information
* if nothing is working on a new system, the first things to check are the settings of the xbees.  Try restoring defaults using X-CTU and then changing only the required settings (API mode AP=2 and PAN ID matching between both xbees)
* the controller needs fairly fresh batteries to work.  It might not be able to work with the lower voltage of rechargeable batteries.  If the connection LED blinks on/off this might mean it has low batteries

## exercises ##
* implement advanced control schemes.  For example, the analog sticks could be made to have a non-linear mapping to control outputs, or a button on the gamepad could change a control scheme to be backwards (for a robot driving a different direction).  The robot could also have pre-scripted actions that it would perform at the press of a button.  This could be done either in the gamepad code or robot code (although for simplicity, making all changes to the robot and not touching the gamepad/radio code is probably best).
* implement some kind of radio auto-pairing, to remove the need to program the gamepad's arduino sketch with the receiver's hardware ID.  This could be done with the a special button on the gamepad ("start" or "select" would make sense - not "analog", it's the gamepad's reset button).  This could include saving the address information of the paired xbee so that, once paired, the robot and gamepad would stay paired even after being turned off.  Many different auto-pairing strategies are possible
* implement messages from the robot back to the gamepad.  The xbees fully support 2-way communications.  Currently this is not used except for the receive notifications on sent messages.  The robot could send a command back to the gamepad that would make the gamepad buzz (it has two buzzer motors inside).  The green "RX" LED could also display some information in the form of blink codes or varying PWM brightness.
* the robot's arduino could be expanded with many different peripherals, like any arduino.  All input states are already sent to the robot so it won't be necessary to update the gamepad or radio code
* xbees support multiple connections at the same time: two gamepads could be set up to control a single robot.  This could be done for redundancy, or to split control of the robot between two operators.  Many different software strategies are possible.  Multiple robots could also communicate directly.  These communication modes require all xbees to have compatible settings, and will depend on the addressing type used.  With the 64-bit addressing used in the example it's fairly simple: make all xbees have the same PAN ID and add a new 64 bit address for each destination.  The sender's address can be checked with a function call on the receiver.
* the robot code could be changed to use a leonardo with minimal effort (the serial ports are the only things that are different).  There are integrated arduino + xbee units that don't require an xbee shield (the arduino itself has an xbee plug).  See <http://www.dfrobot.com/wiki/index.php/DFRobot_Leonardo_with_Xbee_R3(SKU:DFR0221)>
* the dimension engineering sabertooth 2x5 motor controller has a few different control modes.  simplified serial was the mode used in the example.  Another mode could be implemented.  See the website for arduino libraries/specs: <http://www.dimensionengineering.com/info/arduino>
* use different types of xbees: in the example, a series 1 xbee is used.  The code for gamepad/robot includes unused sections to support a series 2 xbee, so that would be a simple swap.  There are also other styles of xbee; the xbee form-factor created by digi has come to support a variety of devices, many not even made by digi.  As long as the replacement device uses serial communications and is within the power budget of the gamepad/robot, it could be made to work.
