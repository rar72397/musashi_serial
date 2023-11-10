// Note:  only working with asynchronous mode, not HDLC or synchronous


/* ----------------------- TO DO ------------------------------------------------

1.  make functions that control reads/writes to control + status registers (do in forms of macros)
2.  circular buffer

(maybe: emulate interrupts)

*/
#include <ctype.h> // for toupper
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "serial.h"

void buffer_init(struct circular_buffer* buffer){
    buffer->full = 0;
    // initialize read and write pointers point to the index of the buffer
    buffer->readPointer = 0;
    buffer->writePointer = 0;
    // initialize buffer to 0
    for(int i=0; i<16; ++i){
        buffer->buffer[i] = 0;
    }
}

// circular buffer functions

// read from buffer at read pointer
unsigned char buffer_read(struct circular_buffer* buffer) {
    if(buffer->readPointer == buffer->writePointer && buffer->buffer[buffer->readPointer-1] == 0)
    {
        printf("Buffer is empty\n");
        return -1;
    } 
    // read everything in the buffer already so return the last read value
    else if (buffer->readPointer == buffer->writePointer && buffer->buffer[buffer->readPointer-1] != 0)
    {
        return buffer->buffer[buffer->readPointer-1];
    }
    // Read value from the current read pointer position
    unsigned char val = buffer->buffer[buffer->readPointer];
    // make sure to not read values that has not been written yet
    if(buffer->readPointer != buffer->writePointer){
        // Move the read pointer to the next position and handle wrapping
        buffer->readPointer = (buffer->readPointer + 1) % 16;
    }
    return val;
}


// write to buffer
void buffer_write(struct circular_buffer* buffer, unsigned char val) {
    // if buffer was full and spaces opened up
    if(buffer->full && buffer->readPointer != ((buffer->writePointer + 1) % 16)){
        buffer->full = 0;
        // Move the write pointer to the next position first since current position is occupied
        buffer->writePointer = (buffer->writePointer + 1) % 16;
        // Write value to the current write pointer position
        buffer->buffer[buffer->writePointer] = val;
        // update position again so write pointer is at an empty space
        buffer->writePointer = (buffer->writePointer + 1) % 16;
    } else if(buffer->readPointer != ((buffer->writePointer + 1) % 16)){ 
        // Write value to the current write pointer position
        buffer->buffer[buffer->writePointer] = val;
        // Move the write pointer to the next position and handle wrapping
        buffer->writePointer = (buffer->writePointer + 1) % 16;
    } else if (buffer->readPointer == ((buffer->writePointer + 1) % 16) && !buffer->full){
        // fill in the last position in the buffer
        buffer->buffer[buffer->writePointer] = val;
        buffer->full = 1;
    }
}


// print existing items in buffer, empty spaces are represented by 0 so don't print those
void buffer_print(struct circular_buffer* buffer){
    for(int i=0; i<16; ++i){
        if(buffer->buffer[i] != 0){
            printf("%c", buffer->buffer[i]);
        } else {
            break;
        }
    }
}

// Unlike C++ structs in C can't have member functions, so it will be a global variable
// that is called by functions below, also have to specify that serial_chip is struct in
// declaration
struct serial_chip chip;

// Instead of having to memorize all of the bits of each of the registers, I have decided to write a bunch of macros
// that set and clear the bits instead

// status: can be 's' or 'c' for set and clear respectively

// -------------------------------------------------------------------------------- CONTROL REGISTER MACROS ----------------------------------------------------------

// --------------- CONTROL REGISTER 0 ------------------------

// Register Pointers - Control Register 0, bits 0 through 2
// determines which register is going to be accessed for read/writes
// initialized to 0

#define REG_PTR(CHANNEL, NUMBER) \
    do { \
        if(CHANNEL == 'A'){ \
            chip.controlRegisterA[0] &= ~(7); \
            chip.controlRegisterA[0] |= NUMBER; \
        } \
        else if(CHANNEL == 'B'){ \
            chip.controlRegisterB[0] &= ~(7); \
            chip.controlRegisterB[0] |= NUMBER; \
        } \
    } while (0)


// Command Bits - Control Register 0, bits 3 through 5
// intialized to 0, changes depending on what function needs to be called

// 000 - null (default)
// 001 - send abort
// 010 - reset external status interrupt
// 011 - channel reset
// 100 - enable interrupt on next character
// 101 - reset pending transmitter interrupt/DMA request
// 110 - error reset
// 111 - end of interrupt (channel A only)

#define COMMAND(CHANNEL, NUMBER)\
    do{ \
        if(CHANNEL == 'A'){ \
            chip.controlRegisterA[0] &= ~(7<<3); \
            chip.controlRegisterA[0] |= (NUMBER<<3); \
        } \
        else if(CHANNEL == 'B'){ \
            chip.controlRegisterB[0] &= ~(7<<3); \
            chip.controlRegisterB[0] |= (NUMBER<<3); \
        } \
    } while(0)


// CRC Control Commands - Control Register 0, bits 6 and 7
// will be unused in the code, setting all to 0

// --------------- CONTROL REGISTER 1 ------------------------

// External/Status Interrupt Enable - Control Register 1, bit 0
// will always be set to 0, since we are not uscing HDLC/sync modes

// Transmitter Interrupt Enable - Control Register 1, bit 1
// set to 0 by default
#define TRANSMITTER_INTERRUPT_ENABLE(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[1] |= (1<<1);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[1] &= ~(1<<1);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[1] |= (1<<1);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[1] &= ~(1<<1);\
        }\
    }

// Condition Affects Vector - Control Register 1, bit 2
// depends on whether or not we want to use our own interrupt vectors with this program
// setting to 0 by default
#define CONDITION_AFFECTS_VECTOR(CHANNEL, STATUS)\
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[1] |= (1<<2);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[1] &= ~(1<<2);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[1] |= (1<<2);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[1] &= ~(1<<2);\
        }\
    }

// Receiver Interrupt Mode - Control Register 1, bits 3 and 4
// a lot of the actual commands for this function will go unused since we aren't using HDLC, etc
#define RECEIVER_INT_MODE(CHANNEL, NUMBER)\
    do{ \
        if(CHANNEL == 'A'){ \
            chip.controlRegisterA[1] &= ~(3<<3); \
            chip.controlRegisterA[1] |= (NUMBER<<3); \
        }\
        else if(CHANNEL == 'B'){ \
            chip.controlRegisterB[1] &= ~(3<<3); \
            chip.controlRegisterB[1] |= (NUMBER<<3); \
        }\
    } while(0) \
    

// Wait on Receiver/Transmitter - Control Register 1, bit 5
#define WAIT_ON_RxTx(CHANNEL, STATUS)\
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[1] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[1] &= ~(1<<5);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[1] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[1] &= ~(1<<5);\
        }\
    }

// Tx Byte Count Enable - Control Register 1, bit 6
#define Tx_COUNT_ENABLE(CHANNEL, STATUS)\
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[1] |= (1<<6);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[1] &= ~(1<<6);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[1] |= (1<<6);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[1] &= ~(1<<6);\
        }\
    }

// Wait Function Enable - Control Register 1, bit 7
#define WAIT_FUNC_EN(CHANNEL, STATUS)\
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[1] |= (1<<7);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[1] &= ~(1<<7);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[1] |= (1<<7);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[1] &= ~(1<<7);\
        }\
    }

// --------------- CONTROL REGISTER 2 (A ONLY) ------------------
// Channel 2 is custom made

// DMA Mode Select - Control Register 2, bits 0 and 1
#define DMA_MODE_SEL(NUMBER)\
    chip.controlRegisterA[1] &= ~(3<<0); \
    chip.controlRegisterA[1] |= (NUMBER<<0); \

// Priority - Control Register 2, bit 2
#define PRIORITY(STATUS)\
    if(STATUS == 's'){\
        chip.controlRegisterA[1] |= (1<<2);\
    }\
    else if(STATUS == 'c'){\
        chip.controlRegisterA[1] &= ~(1<<2);\
    }

// Interrupt Vector Mode - Control Register 2, bit 3 through 5
#define INT_VEC_MODE(NUMBER)\
    chip.controlRegisterA[1] &= ~(7<<3);\
    chip.controlRegisterA[1] |= (NUMBER<<3);\

// Rx Int Mask - Control Register 2, bit 6
#define Rx_INT_MASK(STATUS)\
    if(STATUS == 's'){\
        chip.controlRegisterA[1] |= (1<<6);\
    }\
    else if(STATUS == 'c'){\
        chip.controlRegisterA[1] &= ~(1<<6);\
    }

// Pin 10 !SYNCB/!RTSB Select - Control Register 2, bit 7
#define PIN10_SEL(STATUS)\
    do{ \
        if(STATUS == 's'){\
            chip.controlRegisterA[1] |= (1<<7);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[1] &= ~(1<<7);\
        }\
    }while(0)
    

// --------------- CONTROL REGISTER 3 ------------------------

// Receiver Enable - Control Register 3, bit 0
#define RECEIVE_ENABLE(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[3] |= (1<<0);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[3] &= ~(1<<0);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[3] |= (1<<0);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[3] &= ~(1<<0);\
        }\
    }

// Sync Character Load Inhibit - Control Register 3, bit 1
// ignore, we don't have to do anything with Sync

// Address Search Mode - Control Register 3, bit 2
// ignore, has to do with HDLC

// Receiver CRC Enable - Control Register 3, bit 3
// ignore, has to do with CRC

// Enter Hunt Phase - Control Register 3, bit 4
// ignore, has to do with HDLC/synch

// Auto Enables - Control Register 3, bit 5
#define AUTO_EN(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[3] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[3] &= ~(1<<5);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[3] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[3] &= ~(1<<5);\
        }\
    }

// Number of Received Bits per Character - Control Register 3, bits 6 and 7
#define R_BITS_PER_CHAR(CHANNEL, NUMBER)\
    do{ \
        if(CHANNEL == 'A'){ \
            chip.controlRegisterA[3] &= ~(3<<6); \
            chip.controlRegisterA[3] |= (NUMBER<<6); \
        } \
        else if(CHANNEL == 'B'){ \
            chip.controlRegisterB[3] &= ~(3<<6); \
            chip.controlRegisterB[3] |= (NUMBER<<6); \
        }\
    }while(0)
    

// ----------------------------------------------------------

// --------------- CONTROL REGISTER 5 -----------------

// Transmitter Enable - Control Register 5, bit 3

#define TRANSMITTER_ENABLE(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.controlRegisterA[5] |= (1<<3);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterA[5] &= ~(1<<3);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.controlRegisterB[5] |= (1<<3);\
        }\
        else if(STATUS == 'c'){\
            chip.controlRegisterB[5] &= ~(1<<3);\
        }\
    }


// ---------------------------------------------------

// ------------------------------------------------------------------------------ STATUS REGISTER MACROS ------------------------------------------------------------------------------------------------------------


// ----------------------- STATUS REGISTER 0 ---------------------------------------


// Receive Char Available - Status Register 0, bit 0 
#define RECEIVE_CHAR_AVAILABLE(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[0] |= (1<<0);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[0] &= ~(1<<0);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[0] |= (1<<0);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[0] &= ~(1<<0);\
        }\
    }


// Interrupt Pending - Status Register 0, bit 1 (CHANNEL A ONLY)
#define INTERRUPT_PENDING(STATUS) \
    if(STATUS == 's'){\
        chip.statusRegisterA[0] |= (1<<1);\
    }\
    else if(STATUS == 'c'){\
        chip.statusRegisterA[0] &= ~(1<<1);\
    }


// Transmitter Buffer Empty - Status Register 0, bit 2
#define TRANSMITTER_BUFFER_EMPTY(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[0] |= (1<<2);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[0] &= ~(1<<2);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[0] |= (1<<2);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[0] &= ~(1<<2);\
        }\
    }


// DCD status - Status Register 0, bit 3

// DCD: Data Carrier Detect - input goes low to indicate the presence of valid serial data at RxD (in channels A or B)
// This one is kinda weird; Instead of saying just DCD, the datasheet says that it inverts the state of !DCD input. This is because the chip reads
// !DCD, and not DCD by itself). There are a lot of bits in the status register that behave like this. 

#define DCD_STATUS(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[0] |= (1<<3);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[0] &= ~(1<<3);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[0] |= (1<<3);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[0] &= ~(1<<3);\
        }\
    }


// Sync Status - Status Register 0, bit 4
// There are multiple sync modes that the serial chip can use: async, sync, monosync, bisync, HDLC. However, for this model, we will only be
// considering aysnc mode
// Like DCD, inverts state of !SYNC input, giving SYNC

#define SYNC_STATUS(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[0] |= (1<<4);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[0] &= ~(1<<4);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[0] |= (1<<4);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[0] &= ~(1<<4);\
        }\
    }


// CTS Status - Status Register 0 , bit 5
// Inverts !CTS input, giving CTS input
// when thisbit is flipped, causes an external/status interrupt request

#define CTS_STATUS(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[0] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[0] &= ~(1<<5);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[0] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[0] &= ~(1<<5);\
        }\
    }


// Idle/CRC - Status Register 0, bit 6
// only used in synchronous mode or HDLC mode, so we will be skipping the macro

// Break/Abort - Status Register 0, bit 7
// when this bit is flipped, causes an external/status interrupt
// set when break sequence is detected (null character plus framing error) when RxD is low, (spacing?), for
// more than one character at a time
// cleared when RxD returns to high

#define BREAK_STATUS(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[0] |= (1<<7);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[0] &= ~(1<<7);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[0] |= (1<<7);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[0] &= ~(1<<7);\
        }\
    }


// -------------------------------------------------------------------------------

// --------------------- STATUS REGISTER 1 ------------------------------------------

// All sent -  Status Register 1, bit 0
#define ALL_SENT(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[1] |= (1<<0);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[1] &= ~(1<<0);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[1] |= (1<<0);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[1] &= ~(1<<0);\
        }\
    }


// Residue Codes - Status Register 1, bits 1-3
// We can ignore these bits completely, since they are only used in an HDLC message :)

//--------------- Following bits have to do with special receive conditions, trigger an interrupt request -----------------

// Latched Bits will only be reset if a error reset command is sent, receiving new characters does not change its status

// Parity Error (LATCHED) - Status Register 1, bit 4 
#define PARITY_ERROR(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[1] |= (1<<4);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[1] &= ~(1<<4);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[1] |= (1<<4);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[1] &= ~(1<<4);\
        }\
    }


// Receiver Overrun Error (LATCHED) - Status Register 1, bit 5
// Occurs when receiver buffer already contains 3 characters and 4th is completely received, overwriting the last character in the buffer

// Likely that this will always remain at 0 in our code, since we are giving our buffers more memory, 16 characters

#define RECEIVER_OVERRUN(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[1] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[1] &= ~(1<<5);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[1] |= (1<<5);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[1] &= ~(1<<5);\
        }\
    }


// CRC/Framing Error - Status Register 1, bit 6
// CRC - Cyclic Redundancy Check
// set when no stop bit is detected at the end of a character, when this occurs, chip waits additional one-half bit 
// before resampling so framing error is not interpreted as new start bit
#define CRC_ERROR(CHANNEL, STATUS) \
    if(CHANNEL == 'A'){\
        if(STATUS == 's'){\
            chip.statusRegisterA[1] |= (1<<6);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterA[1] &= ~(1<<6);\
        }\
    }\
    else if(CHANNEL == 'B'){\
        if(STATUS == 's'){\
            chip.statusRegisterB[1] |= (1<<6);\
        }\
        else if(STATUS == 'c'){\
            chip.statusRegisterB[1] &= ~(1<<6);\
        }\
    }


// End of HDLC Frame - Status Register 1, bit 7
// HDLC mode, ignore
// ---------------------------------------------------------------------------

// ------------- STATUS REGISTER 2B (CHANNEL A DOES NOT HAVE EQUIVALENT) -----------
// custom to the user who is implementing it, setting to 0 for now


// setting everything to 0
void chip_init(){
    // initialize the chip's registers to 0 by calling buffer_init on each of them
    buffer_init(&chip.aReceive);
    buffer_init(&chip.aTransmit);
    buffer_init(&chip.bReceive);
    buffer_init(&chip.bTransmit);

    // --------------- CONTROL INIT ----------------

    // CR0
    REG_PTR('A', 0);
    REG_PTR('B', 0);
    COMMAND('A', 0);
    COMMAND('B', 0);
    chip.controlRegisterA[0] &= ~(3<<6);  // bits 6 and 7
    chip.controlRegisterB[0] &= ~(3<<6);

    // CR1
    chip.controlRegisterA[1] &= ~(1<<0);
    chip.controlRegisterB[1] &= ~(1<<0);
    TRANSMITTER_INTERRUPT_ENABLE('A', 'c');
    TRANSMITTER_INTERRUPT_ENABLE('B', 'c');
    CONDITION_AFFECTS_VECTOR('A', 'c');
    CONDITION_AFFECTS_VECTOR('B', 'c');
    RECEIVER_INT_MODE('A', 0);
    RECEIVER_INT_MODE('B', 0);
    WAIT_ON_RxTx('A', 'c');
    WAIT_ON_RxTx('B', 'c');
    Tx_COUNT_ENABLE('A','c');
    Tx_COUNT_ENABLE('B', 'c');
    WAIT_FUNC_EN('A', 'c');
    WAIT_FUNC_EN('B', 'c'); 

    // CR2
    DMA_MODE_SEL(0);
    PRIORITY('c');
    INT_VEC_MODE(0);
    Rx_INT_MASK('c');
    PIN10_SEL('c');

    // Channel B is up to the User
    chip.controlRegisterB[2] = 0;

    // CR3
    RECEIVE_ENABLE('A', 'c');
    RECEIVE_ENABLE('B', 'c');
    
    for(int i=1; i<=4; ++i){
        chip.controlRegisterA[3] &= ~(1<<i);
        chip.controlRegisterB[3] &= ~(1<<i);
    }
    AUTO_EN('A', 'c');
    AUTO_EN('B', 'c');
    R_BITS_PER_CHAR('A', 0);
    R_BITS_PER_CHAR('B', 0);


    // CR5
    TRANSMITTER_ENABLE('A', 'c');
    TRANSMITTER_ENABLE('B', 'c');

    // CR6


    // CR7


    // -------------- STATUS INIT ----------------

    // SR0
    RECEIVE_CHAR_AVAILABLE('A', 'c');
    RECEIVE_CHAR_AVAILABLE('B', 'c');
    INTERRUPT_PENDING('A');
    INTERRUPT_PENDING('A');
    TRANSMITTER_BUFFER_EMPTY('A', 's');
    TRANSMITTER_BUFFER_EMPTY('B', 's');
    SYNC_STATUS('A', 'c');
    SYNC_STATUS('B', 'c');
    BREAK_STATUS('A', 'c');
    BREAK_STATUS('B', 'c');

    // SR1
    ALL_SENT('A', 'c');
    ALL_SENT('B', 'c');
    RECEIVER_OVERRUN('A', 'c');
    RECEIVER_OVERRUN('B', 'c');
    PARITY_ERROR('A', 'c');
    PARITY_ERROR('B', 'c');
    RECEIVER_OVERRUN('A', 'c');
    RECEIVER_OVERRUN('B', 'c');
    CRC_ERROR('A', 'c');
    CRC_ERROR('B', 'c');

    // SR2
    chip.statusRegisterB[2] = 0;

    // SR 3 and 4
    chip.statusRegisterA[3] = 0;
    chip.statusRegisterA[4] = 0;
    chip.statusRegisterB[3] = 0;
    chip.statusRegisterB[4] = 0;

    // unsigned int TxA_byte_count = 0;
    // unsigned int TxB_byte_count = 0;
}

// FUNCTION PARAMETERS:
// channel = decide between a and b channels
// val = new val to be written into said index

// function should be called only when Tx sent is equal to the number of bytes in the Tx buffer

// ---------- transmit: processor (m68k) -> serial chip's transmit buffer -> other separate devices [one byte at a time]----------------------------

void transmit_write(char channel, char val){
    if(toupper(channel) == 'A'){
        buffer_write(&chip.aTransmit, val);
    }
    else if(toupper(channel) == 'B'){
        buffer_write(&chip.bTransmit, val);
    }
    else{ // Only allowed to look at A and B channels
        printf("Invalid channel name. Use either 'A' or 'B'");
    }
}

unsigned char transmit_read(char channel){
    if(toupper(channel) == 'A'){
        chip.TxA_byte_count++;
        return buffer_read(&chip.aTransmit);
    }
    else if(toupper(channel) == 'B'){
        chip.TxB_byte_count++;
        return buffer_read(&chip.bTransmit);
    }
    else{ // Only allowed to look at A and B channels
        printf("Invalid channel name. Use either 'A' or 'B'");
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------

// -------------- RECEIVE: other separate devices -> serial chip's receive buffer -> processor (m68k) [one byte at a time] ---------------------
void receive_write(char channel, char val){
    if(toupper(channel) == 'A'){
        buffer_write(&chip.aReceive, val);
        RECEIVE_CHAR_AVAILABLE('A', 's');
    }
    else if(toupper(channel) == 'B'){
        buffer_write(&chip.bReceive, val);
        RECEIVE_CHAR_AVAILABLE('B', 's');
    }
    else{ // Only allowed to look at A and B channels
        printf("Invalid channel name. Use either 'A' or 'B'");
    }
}

unsigned char receive_read(char channel){
    if(toupper(channel) == 'A'){
        return buffer_read(&chip.aReceive);
    }
    else if(toupper(channel) == 'B'){
        return buffer_read(&chip.bReceive);
    }
    else{ // Only allowed to look at A and B channels
        printf("Invalid channel name. Use either 'A' or 'B'");
    }
    return 0;
}

// -------------------------------------------------------------------------------------------------------------------------------------

// for output purposes
void TxRx_print(char channel, char type){
    if(toupper(channel) == 'A'){
        if(type == 't'){
            buffer_print(&chip.aTransmit);
        }
        else if(type == 'r'){
            buffer_print(&chip.aReceive);
        }
        else{
            printf("Invalid channel type");
        }
    }
    else if(toupper(channel) == 'B'){
        if(type == 't'){
            buffer_print(&chip.bTransmit);
        }
        else if(type == 'r'){
            buffer_print(&chip.bReceive);
        }
        else{
            printf("Invalid channel type. Use either 't' (transmit) or 'r' (receive)");
        }
    }  
    else{
        printf("Invalid channel name. Use either A or B");
    }
}

// Jacob
void write_control(char channel, int register, int bit){

}

// Amy
unsigned char read_status(char channel, int register, int bit){
    return
}

// channel: 'A' or 'B'
// location: 'control' or 'buffer' 
// val: 0 or 1
void write(char channel, char location, int reg, int bit){
    if(location == 'status'){
        
    }
    else if(location == 'buffer'){
        write_control(channel, reg, bit);
    }
}

// channel: 'A' or 'B'
// location: 'status' or 'buffer'
// bit: 0-7
unsigned char read(char channel, char location, int reg, int bit){
    if(location == 'buffer'){
        return ;
    }
    else if(location == 'status'){
        return read_status(channel, reg, bit);
    }
}