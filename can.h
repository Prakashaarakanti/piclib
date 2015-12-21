/* 
 * Main CAN interface. Provides methods for initializing the CAN protocol stack as well as sending CAN traffic over.
 * Implemented from PIC18F2XKXX datasheet, so could work for other chips too (assuming xc.h is included in your main file prior to this one)
 * 
 * File:   can.h
 * Author: pojd
 *
 * Created on October 31, 2015, 10:19 PM
 */

#ifndef CAN_H
#define	CAN_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "utils.h"

/**
 * Type of CanMessage
 */
typedef enum {
    NORMAL    = 0b000, // normal message is a message sent by the node to reveal some action performed
    HEARTBEAT = 0b001, // hearbeat message is also sent by this node, but only triggered by a timer
    CONFIG    = 0b010, // config would typically be sent by some master node in the network to setup this node (so this node would receive this message instead)
    COMPLEX   = 0b011  // complex message is allowing more complex commands to be transmitted (sort of extension of NORMAL))
} MessageType;
    
/**
 * CanHeader structure - holds the header information used to wrap information needed to generate can IDs.
 */
typedef struct {
     /** the message ID - 8 bits only in this application, other 3 as part of the final can ID are used to encode other information  */
     byte nodeID;
          
     /** type of the message to be sent/received  */
     MessageType messageType;
} CanHeader;

/**
 * CanMessage structure - main structure for sending CAN traffic over
 */
typedef struct {
     CanHeader *header;

     /*
      * The below will become part of the data payload sent as part of this CAN message (custom protocol)
      */
     
     /**
      * Boolean determining whether the switch is on or off. The semantics of this are up to the switch (e.g. on always in the case of lights,
      * while on/off in the case of reed switch)
      * By default the switch is on, e.g. heartbeat message will find out on its own though
      */
     boolean isSwitchOn;
 } CanMessage;

/** Modes of the controller - see datasheet */ 
 typedef enum {
    CONFIG_MODE   = 0b100,
    LOOPBACK_MODE = 0b010,
    NORMAL_MODE   = 0b000
} Mode;

/**
 * Code of result of sending a CAN message
 */
typedef enum {
    OK           = 0, 
    ERROR        = 1,
    SENDING      = 2,
    NOTHING_SENT = 3
} MessageStatusCode;

/**
 * MessageStatus holds time and status of the last CAN send
 */
typedef struct {
     MessageStatusCode statusCode;
     int timestamp; // timestamp in whatever unit the target application wishes to use
 } MessageStatus;
 
/** was a CAN message send? The can util will update only the SENDING status when a can send is requested, 
 *  but the application code is required to keep it updated after a message is sent (both timestamp and status) */
MessageStatus messageStatus;

/**
 * Sets up basic can settings (ports to starts with)
 */
void can_init();

/**
 * Sets operational mode of CAN chip
 * 
 * @param mode mode to set
 */
void can_setMode(volatile Mode mode);

/**
 * Sets up baud rate of CAN chip
 * 
 * @param baudRate the baud rate to use (in kbits per seond). Max is 500 (the implementation uses 16TQ for the bit sequencing)
 * @param cpuSpeed speed of the clock in MHz (mind that PLL settings in registers may affect this)
 */
void can_setupBaudRate(volatile int baudRate, volatile int cpuSpeed);

/** flag used to indicate whether a filter has already been setup. If so, the second filter would be used
 * (just 2 filters are supported at the moment, calling the below methods multiple times will result in overwriting
 * the second acceptance filter). Either will result in 1st receive buffer receiving the message */
boolean filterSetup = FALSE;

/**
 * Setup receive filter based on the in passed CanHeader - to receive only can messages for that header. This method will setup
 * a single mask only checking all bits to be equal.
 * 
 * @param header wrapper for the CanHeader - the filter will be setup to match the relevant can messages only
 */
void can_setupStrictReceiveFilter(CanHeader *header);
 
/**
 * Setup receive filter based on the in passed CanHeader - to receive only can messages for that header. This method will setup
 * a single mask only checking only 1st bit of the nodeID and all bits of the message type to match
 * 
 * @param header wrapper for the CanHeader - the filter will be setup to match the relevant can messages only (using just the first bit from the nodeID)
 */
void can_setupFirstBitIdReceiveFilter(CanHeader *header);

/**
 * Attempts to send the message using TXB0 register (not using any others right now)
 * 
 * @param canID: the can ID (standard, ie 11 bits) of the message to be sent
 * @param data: data to be sent over CAN (takes all 8 bytes)
 */
void can_send(CanMessage *canMessage);

#ifdef	__cplusplus
}
#endif

#endif	/* CAN_H */

