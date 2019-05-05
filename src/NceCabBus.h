//------------------------------------------------------------------------
//
// Model Railroading with Arduino - NceCabBus.h 
//
// Copyright (c) 2019 Alex Shepherd
//
// This source file is subject of the GNU general public license 2,
// that is available at the world-wide-web at
// http://www.gnu.org/licenses/gpl.txt
// 
//------------------------------------------------------------------------
//
// file:      NceCabBus.h
// author:    Alex Shepherd
// webpage:   http://mrrwa.org/
// history:   2019-04-28 Initial Version
//
//------------------------------------------------------------------------
//
// purpose:   Provide a simplified interface to the NCE Cab Bus
//
//------------------------------------------------------------------------

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "keycodes.h"

#define AIU_NUM_IOS 14

#define CMD_LEN_MAX 9

typedef enum
{
  CAB_TYPE_UNKNOWN = 0,
  CAB_TYPE_LCD = 'a',
  CAB_TYPE_NO_LCD = 'b',
  CAB_TYPE_SMART = 'c',
  CAB_TYPE_AIU = 'd',
  CAB_TYPE_RESERVED = 'e',
  CAB_TYPE_XBUS_BRIDGE = 'f',
  CAB_TYPE_LOCONET_BRIDGE = 'g'
} CAB_TYPE;

typedef enum
{
  CAB_STATE_UNKNOWN = 0,
  CAB_STATE_PING_BROADCAST, // Ping Broadcast
  CAB_STATE_PING_OTHER,		// Pinging other nodes
  CAB_STATE_PING_NODE,		// Ping This Node
  CAB_STATE_EXEC_CMD,		// Handle Node Commands 
} CAB_STATE;

typedef void (*RS485SendByte)(uint8_t value);
typedef void (*RS485SendBytes)(uint8_t *values, uint8_t length);

class NceCabBus
{
  public:
    NceCabBus();
    
    CAB_TYPE getCabType(void);    
    void setCabType(CAB_TYPE newtype);    

    uint8_t getCabAddress(void);
    void setCabAddress(uint8_t addr);
    
    void processByte(uint8_t inByte);
    
    void setRS485SendByteHandler(RS485SendByte funcPtr);
    void setRS485SendBytesHandler(RS485SendBytes funcPtr);
    
    void setAuiIoState(uint16_t state); 
    uint16_t getAuiIoState(void);
    
    void setAuiIoBitState(uint8_t IoNum, bool bitState); 
    bool getAuiIoBitState(uint8_t IoNum); 

  private:
  	CAB_TYPE	cabType;
  	CAB_STATE	cabState;
  	uint8_t 	cabAddress;
  	bool	 	polled;
  	
  	uint16_t	aiuState;
  	
  	uint8_t		cmdBufferIndex;
  	uint8_t		cmdBufferExpectedLength;
  	uint8_t		cmdBuffer[CMD_LEN_MAX];
  	
  	void		send1ByteResponse(uint8_t byte0);
  	void		send2BytesResponse(uint8_t byte0, uint8_t byte1);
  	
  	RS485SendByte	func_RS485SendByte;
  	RS485SendBytes	func_RS485SendBytes;
  	
  	uint8_t getCmdLen(uint8_t cmd);
};