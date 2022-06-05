/*
 * HAW Hamburg IE
 * MCL_3
 * Task 1
 * @author: Thishan Warnakulasooriya
 * @date: 23.05.2022
 */

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>
#include <math.h>

void configPorts();
void wait(int value);
int_decimal_seperator(double voltage, twoNumber *number);

//structure to hold integer and decimal values of a 4-digit number e.g- (12.34)
typedef struct decimalAndInt {
    int intPart;
    int decimalPart;
} dec_int;


void main(void) {
    unsigned long v_dac;
    float vout;
    double lsb = 0.01953125;
    int firstDigit, secondDigit;
    //pointer to number
    dec_int *numberPtr, number;
    numberPtr = &number;

    //configuring ports
    configPorts();

    while (1) {
        v_dac = 0;
        int i;
        //here we use the weighting conversion method,more details on  how it works is in the document attached
        for (i = 7; i >= 0; i--) {
            //when the conversion is started ,set trigger pulse=1,set EN1 and EN0 to 1;
            GPIO_PORTL_DATA_R |= 0x07;
            //starting from 1000 0000 , all MSB set to 1. And keep setting the next highest significant bit to 1,
            //but clears the bit if the comparator outputs low. At the end of the loop the 8-bit number which is nearly
            // equal to the comparing voltage
            v_dac = v_dac | (1 << i);

            GPIO_PORTK_DATA_R = v_dac;
            wait(1000); //delay of 30 µs until comparator is stable

            if (GPIO_PORTD_AHB_DATA_R == 0x00) {
                v_dac &= ~(1 << i); // clear bit is comparator output is low
            }
        }
        GPIO_PORTL_DATA_R |= 0x03;
        //to get the true value at the end of the weighting conversion we have to multiply the end result by the lsb
        double voltage = v_dac * lsb;

        int_decimal_seperator(voltage, numberPtr);
        firstDigit = ((number.decimalPart) / 10);
        secondDigit = ((number.decimalPart) % 10);
        //displaying the decimal  value of the comparing voltage in the BCD display
        GPIO_PORTM_DATA_R = ((firstDigit << 4) | secondDigit);
        GPIO_PORTL_DATA_R = 0x03;
        wait(10000);
        firstDigit = ((number.intPart) / 10);
        secondDigit = ((number.intPart) % 10);
        GPIO_PORTL_DATA_R = 0x0A;
        //displaying the integer  value of the comparing voltage in the BCD display
        GPIO_PORTM_DATA_R = ((firstDigit << 4) | secondDigit);
        wait(10000);

        while (((GPIO_PORTD_AHB_DATA_R) & 0x02));
    }
}


void configPorts() {
    // Port M Configuration
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11; // clock enable for port M
    while (!(SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R11)); //check for ports ready
    GPIO_PORTM_DEN_R = 0xFF; // Enable PM(7:0)
    GPIO_PORTM_DIR_R = 0xFF; // Set PM(7:0) to output

    // Port L Configuration
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R10; // clock enable for port L
    while (!(SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R10)); //check for ports ready
    GPIO_PORTL_DEN_R |= 0x07; // Enable PL(2:0)
    GPIO_PORTL_DIR_R = 0x07; // Set PL(2:0) to output

    // Port K Configuration
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R9; // clock enable for port K
    while (!(SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R9)); //check for ports ready
    GPIO_PORTK_DEN_R = 0xFF; // Enable PM(7:0)
    GPIO_PORTK_DIR_R = 0xFF; // Set Pk(7:0) to output

    // Port D Configuration
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3; // clock enable for port D
    while (!(SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R3)); //check for ports ready
    GPIO_PORTD_AHB_DEN_R = 0x03; // Enable PD(2:0)
    GPIO_PORTD_AHB_DIR_R = 0x00; // PD(7:0) input be default
}

void wait(int value) {
    //delay function
    int counter;
    for (counter = 0; counter < value; counter++);
}

int_decimal_seperator(double voltage, dec_int *number) {
    //the function separates and returns the decimal and integer part of 4-digit double number.
    number->intPart = (int) voltage;
    number->decimalPart = (voltage - (int) voltage) * 100;
}


