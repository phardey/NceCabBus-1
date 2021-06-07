/*-------------------------------------------------------------------------------------------------------
// Model Railroading with Arduino - NCE USB Interface Example 
//
// Using Library created by 
// Copyright (c) 2021 Alex Shepherd
//
// This source file is subject of the GNU general public license 2,
// that is available at the world-wide-web at: http://www.gnu.org/licenses/gpl.txt
//-------------------------------------------------------------------------------------------------------
// file:      USBInterface.ino
// author:    Paul Hardey   
// history:   2021-06-04 Initial Version
//-------------------------------------------------------------------------------------------------------
// purpose:   Demonstrate how to use the NceCabBus library to build a USB Interface device to JMRI
//
// additional hardware:
//            - An RS485 Interface chip - there are many but the code assumes that the TX & RX Exnable pins
//              are wired together and connected to the Arduino Output Pin defined by RS485_TX_ENABLE_PIN
//              PC running JMRI  
//
// notes:     This example was developed on an Arduino Pro Micro which has the AVR MEGA32U4 chip.
//            It uses this native USB port for Serial Debug output which left the hardware UART 
//            for RS485 comms.
//
// required libraries:
//            NceCabBus which can be found at Alex Shepherd's web page http://mrrwa.org/ 
//        library can be installed using the Arduino Library Manager
//-------------------------------------------------------------------------------------------------------*/
#include <NceCabBus.h>

// Change the line below to match the RS485 Chip TX Enable pin 
#define RS485_TX_ENABLE_PIN 4

// Change the line below to match the RS485 Chip TX Enable pin for Serial Debug 
#define DEBUG_RS485_TX_ENABLE_PIN 3

// Change the line below to match the Serial port you're using for JMRI 
#define JMRISerial Serial

// Change the line below to match the Serial port you're using for RS485 
#define RS485Serial Serial1

// Change the line below to set the USB Cab Bus Address 
#define CAB_BUS_ADDRESS     3

// Uncomment the #define below to enable Debug Output and/or change to write Debug Output to another Serialx device 
#define DebugMonSerial Serial2

#ifdef DebugMonSerial
// Uncomment the #define below to enable printing of RS485 Bytes Debug output to the DebugMonSerial device
//#define DEBUG_RS485_BYTES

// Uncomment the line below to enable Debug output of the JMRI commands Changes
#define DEBUG_JMRI_INPUT

// Uncomment the #define below to enable printing of NceCabBus Library Debug output to the DebugMonSerial device
#define DEBUG_LIBRARY

#if defined(DEBUG_RS485_BYTES) || defined(DEBUG_JMRI_INPUT) || defined(DEBUG_LIBRARY) || defined(DebugMonSerial)
#define ENABLE_DEBUG_SERIAL
#endif
#endif

// Create Cab Bus Object
NceCabBus cabBus;

void sendUSBBytes(uint8_t *values, uint8_t length)
{
    JMRISerial.write(values,length);
    JMRISerial.flush();

    DebugMonSerial.print("\nsendUSBBytes: " );
    for( uint8_t i = 0; i < length; i++)
    {
      if(values[i] < 16)
        DebugMonSerial.print('0');
        
      DebugMonSerial.print(values[i], HEX);
      DebugMonSerial.print(' ');
    }

    DebugMonSerial.println();
}


void sendRS485Bytes(uint8_t *values, uint8_t length)
{
  // Seem to need a short delay to make sure the RS485 Master has disable Tx and is ready for our response
  delayMicroseconds(200);
  
  digitalWrite(RS485_TX_ENABLE_PIN, HIGH);
  RS485Serial.write(values, length);
  RS485Serial.flush();
  digitalWrite(RS485_TX_ENABLE_PIN, LOW);
  
  #ifdef DEBUG_RS485_BYTES  
  DebugMonSerial.print("T:");
  for(uint8_t i = 0; i < length; i++)
  {
    uint8_t value = values[i];
    
    if(value < 16)
      DebugMonSerial.print('0');
  
    DebugMonSerial.print(value, HEX);
    DebugMonSerial.print(' ');
  }
#endif
}
  

  
void setup() {
  uint32_t startMillis = millis();
  const char* splashMsg = "NCE USB Interface Example";
    
 #ifdef ENABLE_DEBUG_SERIAL
  pinMode(DEBUG_RS485_TX_ENABLE_PIN, OUTPUT);
  digitalWrite(DEBUG_RS485_TX_ENABLE_PIN, HIGH);
  DebugMonSerial.begin(115200);
  while (!DebugMonSerial && ((millis() - startMillis) < 3000)); // wait for serial port to connect. Needed for native USB

  if(DebugMonSerial)
  {
#ifdef DEBUG_LIBRARY    
    cabBus.setLogger(&DebugMonSerial);
#endif
    DebugMonSerial.println();
    DebugMonSerial.println(splashMsg);
#endif 
  }
  
  pinMode(RS485_TX_ENABLE_PIN, OUTPUT);
  digitalWrite(RS485_TX_ENABLE_PIN, LOW);
  JMRISerial.begin(9600); // opens serial port, sets data rate to 9600 bps
  RS485Serial.begin(9600,SERIAL_8N2); // serial port 1

  cabBus.setCabType(CAB_TYPE_SMART);
  cabBus.setCabAddress(CAB_BUS_ADDRESS);
  cabBus.setRS485SendBytesHandler(&sendRS485Bytes);
  cabBus.setUSBSendBytesHandler(&sendUSBBytes);
}

void loop() {
    uint8_t rubbish;
    
 // Read the incoming bytes on the RS485 cabbus network and processByte
 
 if(RS485Serial.available())
  {
    uint8_t rxByte = RS485Serial.read();
    cabBus.processByte(rxByte);  
  
#ifdef DEBUG_RS485_BYTES  
    if((rxByte & 0xC0) == 0x80)
    {
      DebugMonSerial.println();
      DebugMonSerial.println();
    }
      
    DebugMonSerial.print("R:");
    DebugMonSerial.print(rxByte, HEX);
    DebugMonSerial.print(' ');
#endif
  }
  
    // If we've been Polled and are currently executing Commands then skip other loop() processing
    // so we don't delay any command/response procesing
    
  if(cabBus.getCabState() == CAB_STATE_EXEC_MY_CMD)
    return;

 // Read the incoming bytes on the USB processByte
 
  if(JMRISerial.available())
  {
    uint8_t jmriByte = JMRISerial.read();
    
    cabBus.processUSBByte(jmriByte);  

    DebugMonSerial.print("\nJMRI R:");
    if(jmriByte < 16)
      DebugMonSerial.print('0');
  
    DebugMonSerial.println(jmriByte, HEX);
  } 

}  // End loop