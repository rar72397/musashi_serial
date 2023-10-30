// serial test code

// THINGS TO TEST
// 1. Basic reading and writing bits to all of the buffers (Channel A and B's receive and transmit buffers)
// 2. writing until the circular buffer is full

// declare chip as volatile global variable?

#include "serial.h"
#include "serial.c"
#include <stdio.h>
#include <assert.h>

void test_writes()
{
    printf("Testing transmit_write function...\n");

    // writing until the buffer is full, want to see if wrap around works
    for (char c = 'a'; c <= 'r'; ++c)
    {
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

extern struct serial_chip chip;

int main()
{
    chip_init(); // sets up circular buffers and intial register values

    // printing out the bits of each of the CONTROL registers
    printf("----- CONTROL REGISTERS -----\n");
    for (int j = 0; j < 8; ++j)
    {
        printf("Register A%d: ", j);
        for (int i = 7; i >= 0; i--)
        {
            printf("%d", (chip.controlRegisterA[j] >> i) & 1);
        }
        printf("\n");

        printf("Register B%d: ", j);
        for (int i = 7; i >= 0; i--)
        {
            printf("%d", (chip.controlRegisterB[j] >> i) & 1);
        }
        printf("\n");
    }
    printf("----- END CONTROL REGISTERS -----\n");

    printf("\n");

    // printing out the bits of each of the STATUS registers
    printf("----- STATUS REGISTERS -----\n");
    for (int j = 0; j < 5; ++j)
    {
        printf("Status Register A%d: ", j);
        for (int i = 7; i >= 0; i--)
        {
            printf("%d", (chip.statusRegisterA[j] >> i) & 1);
        }
        printf("\n");

        printf("Status Register B%d: ", j);
        for (int i = 7; i >= 0; i--)
        {
            printf("%d", (chip.statusRegisterB[j] >> i) & 1);
        }
        printf("\n");
    }
    printf("----- END STATUS REGISTERS -----\n");

    printf("\n");

    test_writes();
    printf("\n");
    test_reads();
    printf("TxA byte count: %d\n", chip.TxA_byte_count);
    printf("TxB byte count: %d\n", chip.TxB_byte_count);

    return 0;
}
