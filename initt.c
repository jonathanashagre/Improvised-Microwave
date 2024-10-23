#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/cpufunc.h>
#include "initt.h"

void TCA_normal(){
     TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_NORMAL_gc; 
    
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTC_gc;
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.PER = 52082;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;
}
void TWIinit() {
    // TODO Fill in with your code from Lab 6
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
    TWI0.MSTATUS =  TWI_WIF_bm |  TWI_RIF_bm | TWI_RXACK_bm;
    
    TWI0.MBAUD = 10;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    TWI0.MCTRLA = TWI_ENABLE_bm;
}
void RTC_init(){
    // Step 1: Wait for the RTC to be ready before making changes
    while (RTC.STATUS > 0) {
        ;  // Wait until RTC is ready (no flags set in STATUS)
    }
    // Step 5: Enable overflow (OVF) and compare match (CMP) interrupts
    RTC.INTCTRL = RTC_OVF_bm | RTC_CMP_bm;  // Enable both interrupts

    // Step 2: Set the RTC period for overflow (e.g., 1024 for 1-second overflow with 32.768kHz / 32 prescaler)
    RTC.PER = 512;  // Overflow period (e.g., 1 second)

    // Step 3: Set the compare value for CMP interrupt to occur half as often as the overflow.
    RTC.CMP = 256;   // Compare match halfway through the period (e.g., 0.5 seconds)

    // Step 4: Select the internal 32.768 kHz oscillator as the clock source
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;

    

    // Step 6: Wait for synchronization to complete before enabling RTC
    while (RTC.STATUS > 0) {
  
    }

    // Step 7: Enable the RTC and set the prescaler
    RTC.CTRLA = RTC_PRESCALER_DIV32_gc   // Set prescaler to divide clock by 32
               | RTC_RTCEN_bm            // Enable RTC
               | RTC_RUNSTDBY_bm;        // Enable RTC in standby mode
}
void ADCInit(){
    PORTD.DIR &= ~PIN6_bm;
    
    
    /* Disable pull-up resistor */
    PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;
    PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    ADC0.CTRLA = ADC_RESSEL_10BIT_gc | ADC_FREERUN_bm;;
    ADC0.MUXPOS = ADC_MUXPOS_AIN6_gc;
    ADC0.CTRLC = ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV4_gc;
    ADC0.INTCTRL = ADC_RESRDY_bm;
}
//400 frequency
void TCA_freq() {
    PORTC.DIR |= PIN0_bm;
    TCA0.SINGLE.CTRLA = 0;

    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_FRQ_gc;

    TCA0.SINGLE.CMP0 = 7574;

    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTC_gc;
    
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
}
//600 frequency
void TCA_freq2(){
    PORTC.DIR |= PIN1_bm;
    TCA0.SINGLE.CTRLA = 0;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_FRQ_gc;
    TCA0.SINGLE.CMP0 = 5554;
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTC_gc;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
    
}
void TCA_pwm(){
    PORTC.DIR |= PIN1_bm;
    TCA0.SINGLE.CTRLA = 0;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    TCA0.SINGLE.CMP1 = 11110;
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTC_gc;
    
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
}