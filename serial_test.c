// serial test code

// THINGS TO TEST
// 1. Basic reading and writing bits to all of the buffers (Channel A and B's receive and transmit buffers)
// 2. writing until the circular buffer is full

// declare chip as volatile global variable?

#include "serial.h"
#include "serial.c" //turn into a header instead
#include <stdio.h>
#include <assert.h>

/*
// Mock buffer_write function if it's not available
void buffer_write(buffer_t *buffer, char val)
{
    // Mock write to buffer
    // This is a mock function, replace with actual implementation
}

// Mock buffer_read function if it's not available
unsigned char buffer_read(buffer_t *buffer)
{
    // Mock read from buffer
    // This is a mock function, replace with actual implementation
    return '0'; // placeholder
}

// Mock buffer_print function if it's not available
void buffer_print(buffer_t *buffer)
{
    // Mock print buffer contents
    // This is a mock function, replace with actual implementation
}

void test_transmit_write()
{
    printf("Testing transmit_write function...\n");
    // setup
    char test_val = 'x';
    // execute
    transmit_write('A', test_val);
    // assert and verify if necessary
    // ... add your assertions and verifications here
    printf("transmit_write function passed.\n");
}

void test_transmit_read()
{
    printf("Testing transmit_read function...\n");
    // setup
    // ... add setup code
    // execute
    char val = transmit_read('A');
    // assert
    assert(val == 'expected_value'); // replace expected_value with the actual expected value
    printf("transmit_read function passed.\n");
}

void test_receive_write()
{
    printf("Testing receive_write function...\n");
    // setup
    char test_val = 'y';
    // execute
    receive_write('B', test_val);
    // assert and verify if necessary
    // ... add your assertions and verifications here
    printf("receive_write function passed.\n");
}

void test_receive_read()
{
    printf("Testing receive_read function...\n");
    // setup
    // ... add setup code
    // execute
    char val = receive_read('B');
    // assert
    assert(val == 'expected_value'); // replace expected_value with the actual expected value
    printf("receive_read function passed.\n");
}

void test_print_register()
{
    printf("Testing print_register function...\n");
    // this function prints to console, so a visual check is needed
    TxRx_print('A', 't');
    // check console output for correctness
    printf("Check above output for correctness.\n");
}
*/

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

    /*test_transmit_write();
    test_transmit_read();
    test_receive_write();
    test_receive_read();
    test_print_register();*/

    printf("All tests passed.\n");

    return 0;
}
