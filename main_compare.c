#define _XTAL_FREQ 4000000
#define diode_freq 3
#define rattling   50
// CONFIG
#pragma config FOSC = INTOSCCLK // Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Detect (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)

#include <xc.h>
#include <pic.h>
#include <pic16f684.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

unsigned char y_flag = 0;
unsigned char d_flag = 1;
unsigned int tim0count, tim0count2 = 0;

unsigned char CheckButton(void) {
  unsigned char result = 0;
  unsigned int butcount = 0;
  while(!RC2) {
    if(butcount < rattling) {
      butcount++;
    }
    else {
      result = 1;
      break;
    }
  }
  return result;
}
 
void interrupt timer0(void) {
    tim0count++;
    INTCONbits.TMR0IF = 0;
    return;
} 

void main(void) {
    TRISA = 0b00000011;
    PORTA = 0b11111100;
    TRISC = 0b11111100;         
    PORTC = 0b11111100;               
    ANSEL = 0b00000011;        
    
    CMCON0 = 0b00000100; 
     
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    TMR0 = 0;
   
    OPTION_REGbits.T0CS = 0; 
    OPTION_REGbits.T0SE = 0; 
    OPTION_REGbits.PSA = 0; 
    OPTION_REGbits.PS0 = 1;
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS2 = 1;
    
    ei();
     __delay_us(100);
     PORTCbits.RC0 = 0;
     INTCONbits.TMR0IE = 0;
     TMR0 = 0;

    while(1) {
        if(tim0count > diode_freq) {
            y_flag = !y_flag;
            PORTCbits.RC0 = y_flag; 
            tim0count = 0;
            tim0count2 = 0;
        } 
        if(C1OUT == 1 && d_flag == 1) {
            tim0count2++;
            if(C1OUT == 1 && d_flag == 1 && tim0count2 == 3) {
            T0IF = 1; 
            INTCONbits.TMR0IE = 1;
            d_flag = 0;
            tim0count2 = 0;
            }
        }
        if(CheckButton()) {
            T0IF = 0;
            PORTCbits.RC0 = 0;
            INTCONbits.TMR0IE = 0;
            d_flag = 1;
        } 
  }
    return;
}