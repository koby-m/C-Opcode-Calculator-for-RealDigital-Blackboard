/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        Program Name:           Keypad Op-Code Calculator

        Contributor(s):         Koby Miller

        Date last modified:     February 15th, 2024

        Description:            An extremely basic calculator. On-board switches are used to determine operation to perform.
                                The four right-most switches are used to input 4-bit ARM Assembly op-codes. 
                                
                                Only 4-digit numbers may be entered, though it operates in two modes, hexadecimal and decimal. The mode is
                                determined by the position of the 12th (left-most) switch. Digits are appended to the right-most digit of 
                                the number, shifting the number over. Operand is confirmed when any pushbutton is pressed, moving to
                                the next operand, then the solution based on the operation.

                                Should the solution of an operation exceed what is able to be displayed on the seven-segment display, 
                                four LEDs will come on to alert that the number is not displayed correctly on the seven-segment due to the
                                four digit limitation. This warning triggers when a negative number is produced also.

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        INCLUDE

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
#include "klib-io.h"
#include "klib-pfrl.h"
#include <sys/_intsup.h> 
#include <sys/unistd.h>

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        MACROS

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
// Op-codes
#define ADDITION_OPCODE 0b0000
#define SUBTRACTION_OPCODE 0b0001
#define REVERSE_SUBTRACTION_OPCODE 0b0010
#define MULTIPLICATION_OPCODE 0b0011
#define MULT_ACCUM_OPCODE 0b0100
#define TEST_EQUIVALENCE_OPCODE 0b101
#define SHIFT_LEFT_OPCODE 0b0110
#define SHIFT_RIGHT_OPCODE 0b0111
#define BITWISE_AND_OPCODE 0b1000
#define BITWISE_OR_OPCODE 0b1001
#define BITWISE_XOR_OPCODE 0b1010
#define BITWISE_CLEAR_OPCODE 0b1011
#define NOT_OPCODE 0b1100
#define COUNT_LEADING_ZEROES_OPCODE 0b1101
#define STORE_OPCODE 0b1110
#define LOAD_OPCODE 0b1111

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        PROTOTYPES

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
int waitForKey();
void performOp(unsigned int* sol, unsigned int op1, unsigned int op2, unsigned int* store, bool mode);
void getInput(unsigned int* val, bool mode);

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        MAIN
        Last modified: February 15th, 2024

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
int main() {
    
    int op[4] = {0,0,0,0}; // {Solution, Operand1, Operand2, Store} respectively
    byte mode = 0; // used to determine what mode of operation (hexadecimal or decimal)

    outputToSevSegCustom("TYPE"); // initial power on

    while(true) { 

        while(getButtonStates() == 0) { // wait until button press
            mode = bitIndex(getSwitchStates(),11); // set mode of operation (hexadecimal or decimal) from twelfth switch
            if(bitIndex(getSwitchStates(), 10) == 1) mode = 2; // override if in binary mode (thirteenth switch)
        }

        outputToSevSegCustom("____"); 
        outputToLEDs(0); // clear previous warning

        while(getButtonStates() != 0); // wait until button release

        getInput(&op[1],mode); // get first operand from keypad

        getInput(&op[2],mode); // get second operand from keypad

        performOp(&op[0], op[1], op[2], &op[3], mode); // perform operation
        
    }    

    return 0;
}

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        DEFINITIONS

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        getInput();
		
        unsigned int val*       Variable to store to from keypad (Must pass variable by reference)
        bool mode               Determines whether to operate in hexadecimal mode (0) or decimal mode (1)
            
            Waits for user to press button keypad, then stores the pressed key to the passed variable.
            The passed variable must be a pointer, or a variable passed by reference.
            
        Written by Koby Miller
        Last modified: February 13th, 2024
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
void getInput(unsigned int* val, bool mode) {

    int key;

    *val = 0; // ensure value passed is empty

    while(true) {

        key = waitForKey(); // wait for a key, then store it
        
        if(key != -1) { // if keypad is not skipped over by pushbuttons
        
            // shift over and add digits
            switch(mode) {
                case 0:  // hexadecimal mode
                    *val *= 0x10;
                    *val %= 0x10000;
                    break;
                case 1: // decimal mode
                    *val *= 10; 
                    *val %= 10000;
                    break;
                case 2: // binary mode
                    *val *= 0b10; 
                    *val %= 0b10000;
                    break;
            }

            *val += key;
        }

        outputToSevSeg(*val, mode);

        if(getButtonStates() != 0) break; // pressing pushbutton confirms number
    }

    outputToSevSegCustom("----");

    while(getButtonStates() != 0); // wait for pushbutton release

    return;
}

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        waitForKey();
            
            Waits for a keypad button to be pressed, and once pressed, returns the value of the key.
            Pressing any one of the four pushbuttons on the board will make the function stop waiting
            for a keypad button press, and will return a -1.
            
        Written by Koby Miller
        Last modified: February 13th, 2024
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
int waitForKey() {

    int val = -1; // default to not pushed

    while(getKeypad() == -1 && getButtonStates() == 0) { // wait until pushed
        usleep(10000); // slight delay
        outputToLEDs(getSwitchStates()&0b1111); // display operation on LEDs
    }

    val = getKeypad(); // set return value based on keypad button

    while(getKeypad() != -1) { // wait until let go
        usleep(10000); // slight delay
    }

    return val;
}

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        performOp();
            
        unsigned int* sol       Variable to store the solution in (pass pointer or variable by reference)
        unsigned int op1        1st operand to use
        unsigned int op2        2nd operand to use
        unsigned int* store     Additional STORE variable (pass pointer or variable by reference)
        bool mode               Determines whether to operate in hexadecimal mode (0) or decimal mode (1)

            Performs an operation using op-codes which are entered on the on-board switches. Solution
            will be stored in pointer or variable passed by reference. 

            EXAMPLES:
                Switches are set to perform ADD
                performOp(&num,a,b,&stor,1);

                Code translation:
                    num = a + b; // in decimal mode

                Switches are set to perform MULTIPLY CUMULATIVE
                performOp(&num,x,y,&stor,1);

                Code translation:
                    num = a * b + stor; // in decimal mode

        Written by Koby Miller
        Last modified: February 13th, 2024
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
void performOp(unsigned int* sol, unsigned int op1, unsigned int op2, unsigned int* store, bool mode) {
    
    byte c; // iterator, only used for counting leading zeroes operation

    // determine operation from switches
    // unlabeled functions are straightforward
    switch(getSwitchStates() & 0xF) { // mask to ignore other switches possibly being switched
        case ADDITION_OPCODE: 
            *sol = op1 + op2;
            break;

        case SUBTRACTION_OPCODE: 
            *sol = op1 - op2;
            break;

        case REVERSE_SUBTRACTION_OPCODE:
            *sol = op2 - op1;             
            break;

        case MULTIPLICATION_OPCODE:
            *sol = (op1) * (op2);
            break;

        case MULT_ACCUM_OPCODE: 
            *sol = (op1 * op2) + *store;
            break;

        case TEST_EQUIVALENCE_OPCODE:
            *sol = (op1 == op2);
            break;

        case SHIFT_LEFT_OPCODE:
            *sol = op1 << op2;
            break;

        case SHIFT_RIGHT_OPCODE:
            *sol = op1 >> op2;
            break;
            
        case BITWISE_AND_OPCODE:
            *sol = op1 & op2;
            break;

        case BITWISE_OR_OPCODE:
            *sol = op1 | op2;
            break;

        case BITWISE_XOR_OPCODE:
            *sol = op1 ^ op2;
            break;

        case BITWISE_CLEAR_OPCODE:
            *sol = op1 & ~op2;
            break;

        case NOT_OPCODE:
            *sol = ~op1;
            break;

        case COUNT_LEADING_ZEROES_OPCODE: 
            *sol = 0; // only instance where solution is reset beforehand
            for(c = 0; c < 31; c++) { // max amount a 32-bit number can be shifted is 31 times
                if( (((op1) >> (31 - c)) & 1) == 0 ) { // increment if bit is zero
                    (*sol)++; 
                } else { // otherwise max the iterator
                    c = 31;
                }
            }
            break;

        case STORE_OPCODE:
            *store = *sol;
            break;

        case LOAD_OPCODE:
            *sol = *store;            
            break;
    }

    // warn if over max number able to be displayed 
    switch(mode) {
        case 0: // hexadecimal mode
            if(*sol > 0xFFFF) outputToLEDs(0b111111111);
            break;
        case 1: // decimal mode
            if(*sol > 9999) outputToLEDs(0b111111111);
            break;
        case 2: // binary mode
            if(*sol > 0b1111) outputToLEDs(0b111111111);
            break;
    }
    
    outputToSevSeg(*sol, mode);

    return;
}