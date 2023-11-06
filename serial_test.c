/* need functions to return and update the status registers
- read goes to status registers
- control goes to control registers
- need to support register pointer functionality
- fucniton to read the receive buffer and write to transmit buffer
- need to determine a vs b
- funciton to write to the control register
- function write status register
- decide how to encode a vs b
- decide how to encode control vs data
- 0 1 2 6 on luna?
- read data a, write data a, write control a, read status a
- same for b
- can simplify to just read and write that takes an address from 0 to 3
- data for each channel and control for each channel
- wrap in c++ and then figure out how to call those functions based on the address
- read and write to data read and write to status
- write the 8(4 functions for each a and b, can have a flag for a and b) then write the 2(call those previous 4 functions)
- 0 channel a data, 1 channel b data, 2 channel a control, 3 channel b control
- read comes from receive buffer
- write goes to transmit buffer

4 (transmit, receive, status, control, each with read write) go into -> 2 functions (1 read, 1 write) -> will be called by sid's framework
update read write functions so that they can update the bits


*/


// serial test code

// THINGS TO TEST
// 1. Basic reading and writing bits to all of the buffers (Channel A and B's receive and transmit buffers)
// 2. writing until the circular buffer is full

// declare chip as volatile global variable?

#include "serial.h"
#include "serial.c" //turn into a header instead
#include <stdio.h>
#include <assert.h>

void test_writes()
{
    printf("Testing transmit_write function...\n");

    // writing until the buffer is full, want to see if wrap around works
    for(char c='a'; c<='z'; ++c){
        transmit_write('A', c);
        transmit_write('B', c);
        receive_write('A', c);
        receive_write('B', c);
    }
   
    printf("At: ");
    TxRx_print('A', 't');
    printf("\n");
    printf("Ar: ");
    TxRx_print('A', 'r');
    printf("\n");
    printf("Bt: ");
    TxRx_print('B', 't');
    printf("\n");
    printf("Br: ");
    TxRx_print('B', 'r');
    printf("\n");
}

void test_reads()
{
    printf("Testing transmit_read function...\n");
    unsigned char At = transmit_read('A');
    unsigned char Ar = receive_read('A');
    unsigned char Bt = transmit_read('B');
    unsigned char Br = receive_read('B');

    printf("At: %c\n", At);
    printf("Ar: %c\n", Ar);
    printf("Bt: %c\n", Bt);
    printf("Br: %c\n", Br);
}

void test_wrap()
{
    printf("Testing buffer's ability to wrap around when full and everything in the beginning of the buffer has been read...\n");
    for(char c='a'; c<='z'; ++c){
        transmit_write('A', c);
        transmit_write('B', c);
        receive_write('A', c);
        receive_write('B', c);
        printf("At: %c\n", transmit_read('A'));
        printf("Ar: %c\n", receive_read('A'));
        printf("Bt: %c\n", transmit_read('B'));
        printf("Br: %c\n", receive_read('B'));
    }
    printf("At: ");
    TxRx_print('A', 't');
    printf("\n");
    printf("Ar: ");
    TxRx_print('A', 'r');
    printf("\n");
    printf("Bt: ");
    TxRx_print('B', 't');
    printf("\n");
    printf("Br: ");
    TxRx_print('B', 'r');
    printf("\n");
    printf("Finished testing wrapping ability\n");
    printf("\n");
}

void test_read2()
{
    printf("Testing buffer's ability to read up all the contents of the buffer when its not full...\n");
    transmit_write('A', 'a');
    transmit_write('A', 'b');
    printf("At: %c\n", transmit_read('A'));
    printf("At: %c\n", transmit_read('A'));
    printf("At: %c\n", transmit_read('A'));
    printf("At: %c\n", transmit_read('A'));
    printf("Finished testing reading all contents in buffer\n");
    printf("\n");
}

extern struct serial_chip chip;

int main()
{
    chip_init(); //sets up circular buffers and intial register values
    
    // printing out the bits of each of the registers after originally set up
    printf("CONTROL REGISTERS: \n");
    printf("\n");
    for(int j=0; j<8; ++j){
        char registerA = chip.controlRegisterA[j];
        char registerB = chip.controlRegisterB[j];

        for (int i = 7; i >= 0; i--){
            if(i == 7) printf("Register A%d: ", j);
            printf("%d", (registerA >> i) & 1);
            printf("\n");
            if(i == 7) printf("Register B%d: ", j);
            printf("%d", (registerB >> i) & 1);
            printf("\n");
        }
        printf("\n");
    }
    
    printf("-------------------\n");
    printf("STATUS REGISTERS: \n");
    for(int j=0; j<5; ++j){
        char registerA = chip.statusRegisterA[j];
        char registerB = chip.statusRegisterB[j];


        for (int i = 7; i >= 0; i--){
            if(i == 7) printf("Register A%d: ", j);
            printf("%d", (registerA >> i) & 1);
            printf("\n");
            if(i == 7) printf("Register A%d: ", j);
            printf("%d", (registerB >> i) & 1);
            printf("\n");
        }
        printf("\n");
    }

    // printf("Testing empty buffer");
    // test_reads();

    test_writes();
    printf("\n");
    test_reads();
    printf("TxA byte count: %d", chip.TxA_byte_count);
    printf("\n");
    printf("TxB byte count: %d", chip.TxB_byte_count);
    printf("\n");
    printf("\n");

    chip_init(); // reset chip
    test_read2();
    chip_init(); // reset chip
    test_wrap();

    return 0;
}
