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

#define _XTAL_FREQ 4000000
#define diode_freq 5
#define rattling   50
#define n_calibrate 10
#define deleyCelibrate 40000
#define countBlinkRes 3

unsigned char y_flag = 0;
unsigned char d_flag = 1;
unsigned int tim0count, adcData, level = 0;
unsigned long counter = 0;
short int sensitivity = 3;
bool interFlag = false;

void interrupt timer0(void) {
    tim0count++;
    INTCONbits.TMR0IF = 0;                        // clear flag
    return;
} 

int Calibrator(){
    int arrData[n_calibrate];
    for(int i = 0; i < n_calibrate; i++){
        ADCON0bits.GO = 1; 
        while(ADCON0bits.GO);
        adcData = (unsigned int)ADRESH << 8;
        adcData |= ADRESL;
        arrData[i] = adcData;                     // write 10-n values ADC
    }
    int maxvalueDat = arrData[0];
    for(int i = 0; i < n_calibrate; i++){         // find max value of array
        if(arrData[i] > maxvalueDat){
            maxvalueDat = arrData[i];
        }
    }
    return maxvalueDat;
}

void Blink(){
    for(int i = 0; i < countBlinkRes; i++){
    PORTCbits.RC1 = 1;
    __delay_ms(200);
    PORTCbits.RC1 = 0;
    __delay_ms(500);
    }
}

void resetCalibrator(){
     Blink();
     level = Calibrator();
     Blink();
}

void main(void){ 
     TRISA = 0b00000011;                         // A0-A1 analog input
     PORTA = 0b11111100; 
     TRISC = 0b11111100;                         // RC1-RC2 digital output 
     PORTC = 0b11111100;            
     ANSEL = 0b00000011;                         // choose analog - digital pins
    
     INTCONbits.GIE = 1;                         // enable all global interrupts
     INTCONbits.PEIE = 1;                        // enable periferal int
     INTCONbits.TMR0IE = 0;                      // enable timer interrupt
     INTCONbits.TMR0IF = 0;                      //celar timer overflow flag
     INTCONbits.T0IF = 0;                        // interrupt timer       
     TMR0 = 0;                                   // register data timer
   
     OPTION_REGbits.T0CS = 0;                    // T0CS - 0 - internal clock
     OPTION_REGbits.T0SE = 0;                    // T0SE - 0 - edge selection
     OPTION_REGbits.PSA = 0;                     // prescalar assigned to timer0
     OPTION_REGbits.PS0 = 1;                     // select prescaler 1:256 PS0-PS2
     OPTION_REGbits.PS1 = 1; 
     OPTION_REGbits.PS2 = 1; 
                                                // ADCON0bits.CHS0 - CHS2 = 0 (standart) CHANNEL A0
     ADCON0bits.VCFG = 1;                         // Vref on
     ADCON0bits.ADON = 1;                         // ADC enable bit
     ADCON0bits.ADFM = 1;                         // right justifield
     ADCON1bits.ADCS0 = 1;                        // ADC clock source - internal Frc
     ADCON1bits.ADCS1 = 1;                        // ADCS0 and ADCS1 = 1
   
      ei();
      __delay_us(1000);
      resetCalibrator();

      while(1) {
          ADCON0bits.GO = 1;                      // run ADC
          while(ADCON0bits.GO);                   // waiting conversion to finish
          adcData = (unsigned int)ADRESH << 8;    // shifting the values
          adcData |= ADRESL;                      // log AND, get the entire value in type int
          if (adcData > level + sensitivity){     // compare with the level
              if(!interFlag){                     // checking the presence of the interrupt tim0 flag= 1;                          
              INTCONbits.TMR0IE = 1;              // enter here only after pressing the button
              interFlag = true;                   
              }
          }
          if(tim0count > diode_freq){             // swap led state with frequency 
              y_flag = !y_flag;                   // diode_freq
              PORTCbits.RC0 = y_flag; 
              tim0count = 0;                      // reset counter
          } 
          while(!RC2){                            // button click processing
              counter++;
              if(counter == rattling){            // processing rattling button
                  INTCONbits.T0IF = 0;                       // resetting all parameters
                  PORTCbits.RC0 = 0;
                  PORTCbits.RC1 = 0;
                  INTCONbits.TMR0IE = 0;
                  interFlag = false;
              }
              if(counter > deleyCelibrate){       // processing button reset
                  resetCalibrator();              // calibration
              }
          }
          counter = 0;                            // reseting counter of press button
    }
return;
}