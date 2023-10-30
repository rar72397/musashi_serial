#ifndef SERIAL_H
#define SERIAL_H

// circular buffer struct
struct circular_buffer{
    unsigned char buffer[16];
    // read and write pointers point to the index of the buffer
    int readPointer;
    int writePointer;
};

// struct definition: represents each of the 4 registers that are used for serial communication
// most likely that we'll only be reading to one channel from the microprocessor, but I'm just keeping
// this in there
struct serial_chip{

    /* ---------- SIO BUFFERS ------------------------------------------
     
    these buffers should be circular buffers
    make sure that reading/writing only happen if there is space within buffer
    if read/write pointers overlap, buffer is full -> don't write anymore to buffer
    if read/write are one apart, buffer is empty

    ---------------------------------------------------------------------*/

    struct circular_buffer aReceive; 
    struct circular_buffer aTransmit;
    struct circular_buffer bReceive;
    struct circular_buffer bTransmit;

    //---------- CONTROL AND STATUS REGISTERS ------------------------------------- 
    // each channel has their own set of registers that act almost exactly the same
    // channel B one extra status register SR[2], whose contents are then used in control register CR[2]
    // channel A status register SR[2] is unused, however I'm keeping the sizes of the registers the same for
    // consistency

    unsigned char controlRegisterA[8];
    unsigned char statusRegisterA[5];

    unsigned char controlRegisterB[8];
    unsigned char statusRegisterB[5];

    unsigned int TxA_byte_count; // We will increment these counters when we read from the Tx buffers
    unsigned int TxB_byte_count;

};


void chip_init();

// buffer functions
void buffer_init(struct circular_buffer* buffer);
unsigned char buffer_read(struct circular_buffer* buffer);
void buffer_print(struct circular_buffer* buffer);

// Tx/Rx buffer functions
void transmit_write(char channel, char val);
unsigned char transmit_read(char channel);
void receive_write(char channel, char val);
unsigned char receive_read(char channel);
void TxRx_print(char channel, char type);

#endif