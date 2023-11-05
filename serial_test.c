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
