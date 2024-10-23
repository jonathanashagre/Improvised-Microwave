/*
 * File:   newavr-main.c
 * Author: jonathana.
 *
 * Created on September 5, 2024, 11:44 AM
 */
#define F_CPU 3333333
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
// Holds the latest ADC conversion result.
volatile uint16_t adc_result = 0;

///general counter
volatile int count = 0;

// Flag to toggle the colon on the display.
volatile int toggleColon = 1;

// Current minute value for the clock display.
// Updated every minute by the RTC.
volatile int min = 0;

// Current hour value for the clock display.
// updated every hour by the RTC.
volatile int hr = 12;

// Button states for four different buttons
volatile int button = 0;
volatile int button2 = 0;
volatile int button3 = 0;
volatile int button4 = 0;

// Timer variables for seconds and minutes in the main cooking timer.
// Used to track the countdown time.
volatile int timerSEC = 0;
volatile int timerMIN = 0;

// Flag indicating that the red button was pressed once.
volatile int redPressedOnce = 0;

// Current power level setting
volatile int powerLevel = 1;

// Flag indicating that ADC data is ready to be processed.
volatile int adc_data_ready = 0;

// Flags indicating state changes for switch case state machines, like the red button going into the cooking setup state and the yellow button going into the kitchen timer state.
volatile int stateChanged = 0;
volatile int stateChanged2 = 0;

// Flag indicating whether the ADC is enabled.
volatile int adc_enabled = 0;

// Countdown flag for the main timer and flag indicating PWM has started.(pwm unfortunately was not implemented fully)
volatile int countdown = 0;
volatile int pwm_started = 0;

// Flags controlling button press allowances and power cancellation.
// allowedToPress: Determines if button presses are currently accepted.
// powerCancel: Flags whether setting up the power was cancelled.
volatile int allowedToPress = 0;
volatile int powerCancel = 0;

// Flag to allow RTC to update the display, it'll continue keeping track of real time, but won't display it unless this flag is set.
volatile int allowRTC = 1; 

// Timer variables for specific display modes, initialized to 3 seconds each, one for cooking and one for the kitchen timer
volatile uint8_t displayFoodTimer = 3;
volatile uint8_t displayDoneTimer = 3;

// Flag set every one second by a timer ISR
volatile uint8_t oneSecondFlag = 0;

// Flags indicating whether specific tones have been started(400 FRQ AND 600 FRQ TONES FOR COOKING AND KITCHEN TIMER)
volatile int tone_started = 0;
volatile int tone_started2 = 0;

//counter for keeping track of a second.
volatile int cnt = 0;
volatile int cnt2 = 0;

// Flag indicating that the yellow button was pressed.
volatile int yellowPress = 0;

// Timer variables for the kitchen timer in minutes and seconds.
// Separates kitchen timer functionality from the main cooking timer.
volatile int timerM2 = 0;
volatile int timerS2 = 0;

//countdown flag for the kitchen timer to notify when the countdown has finished
volatile int countdown2 = 0;

// Flag to check the status of timers.
volatile int timerCheck = 0;

// Flags indicating whether the main timer and kitchen timer have started.
volatile int timerStart = 0;
volatile int timerStart2 = 0;

// Flag that indicates to start incrementing the kitchen timer using green or blue button.
volatile int kitchenT = 0;

// Flag to cancel timer setups.
volatile int timeCancel = 0;



typedef enum{
        RED = PIN4_bm,
        YELLOW = PIN5_bm,
        GREEN = PIN6_bm,
        BLUE = PIN7_bm,
        POWER_SETTING,
        STATE_COOKING,
        STATE_CLOCK,
        STATE_DISPLAY_FOOD,
        TIMER,
        TIMER_RUNNING,
       STATE_DISPLAY_DONE
    } color; 
    

ISR(ADC0_RESRDY_vect){
    adc_result = ADC0.RES;
    //SET DUTY CYCLE TO BE 10% WHEN POWERLEVEL=1, 20% FOR POWERLEVEL=2, AND SO ON UNTIL 10.
    adc_data_ready = 1;
    PORTC.OUTTGL = PIN1_bm;
    ADC0.INTFLAGS = ADC_RESRDY_bm;
}
ISR(TCA0_CMP0_vect){
    //ASK FOR HELP FOR THE DUTY CYCLES
//    TCA0.SINGLE.CMP0 = (TCA0.SINGLE.CMP0+TCA0.SINGLE.CMP0)%52082;
//    TCA0.SINGLE.CMP0 = 0;
//   
    PORTC.OUT = PIN0_bm;
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP0_bm;
}
ISR(TCA0_OVF_vect){
    
        if (timerM2 == 0 && timerS2 == 0) {
                // Timer has finished
                timerStart = 0; // Stop the timer
                countdown2 = 1; // Set flag to indicate timer completion
               

            } else {
                // Decrement the timer
                if (timerS2 == 0 && timerM2 == 0) {
                        timerM2--;
                        timerS2 = 60;

                }
                 timerS2--;
//                 timerCheck++;
//                if(timerCheck < 5){
//                    writeDigits(timerM2, timerS2);
//                }
//                   
//                    if(writeOnce = 0){
//                        writeDisplay(hr, min);
//                        writeOnce = 1;
//                    }
//                // Update the display
                writeDigits(timerM2, timerS2);
      }
    if (timerMIN == 0 && timerSEC == 0) {
            PORTC.OUT &= ~PIN1_bm;            // Cooking is done
            ADCDisable();     // Ensure ADC is disabled
            countdown = 1;
            timerStart2 = 0;
            pwm_started = 0;  // Reset PWM started flag
            TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // Stop TCA0
            
        } else {
            // Decrement timer and update display
            if(timerMIN>0 && timerSEC == 0){
                timerMIN--;
                timerSEC = 60;
            }
        timerSEC--;
 
        writeDigits(timerMIN, timerSEC);

    }
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}
ISR(PORTA_PORT_vect){
    if(PORTA.INTFLAGS & PIN4_bm){
        button = 1;    
       
    }
    if(PORTA.INTFLAGS & PIN5_bm){
       button2 = 1; 
       
    }
    if(PORTA.INTFLAGS & PIN6_bm){
        button3 = 1;
       
    }
    if(PORTA.INTFLAGS & PIN7_bm){
       button4 = 1; 
       
    }
    
    PORTA.INTFLAGS = PIN4_bm;
    PORTA.INTFLAGS = PIN5_bm;
    PORTA.INTFLAGS = PIN6_bm;
    PORTA.INTFLAGS = PIN7_bm;
}
ISR(RTC_CNT_vect)
{
    //basically this CMP interrupt is used for the colon blinking, giving the user a more realistic feeling of a microwave clock
    if(RTC.INTFLAGS & RTC_CMP_bm){
        if (allowRTC == 1) {
        if(toggleColon == 1){
            turnOffColon();
            toggleColon = 0;
        }
        else if(toggleColon == 0){
            turnOnColon();
            toggleColon = 1;
        }
        }
        //count basically keeps track of how long until we should increment the time
        count++;
        RTC.INTFLAGS = RTC_CMP_bm;
    }
    
    else if(RTC.INTFLAGS & RTC_OVF_bm){
        cnt++;
        //overflow interrupt, once the count is 128 we know a minute has passed, so we can increment the minute
         

       
        if (cnt >= 2) {
            cnt = 0; 
            oneSecondFlag = 1;   
        }
        if(count == 128){
            count = 0;
            min++;
             if(min == 60){
                incrementHour();
                min = 0;
              
        }
            if(allowRTC){
                writeDisplay(hr, min);
            }
        }
        //depending on what the minute is, we increment the hour.
        RTC.INTFLAGS = RTC_OVF_bm;
    }
    
}
void clearDisplay() {
    // Send the clear display command (0x76) to the display
    TWI0.MADDR = (0x71 << 1) | 0;  // Set the TWI address for your display
    while (!(TWI0.MSTATUS & TWI_WIF_bm));  // Wait for the TWI interface to be ready
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
        return;  // Exit if error occurred
    }
    // Send the clear display command (0x76)
    TWI0.MDATA = 0x76;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));  // Wait for the data to be transmitted
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
        return;  // Exit if error occurred
    }

    // Send stop condition
    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
    _delay_ms(5); 
}
void pauseRTC() {
    RTC.CTRLA &= ~RTC_RTCEN_bm;
    while (RTC.STATUS > 0) {
    }
}
void incrementHour() {
    hr = (hr % 12) + 1;
}
uint8_t mapCharTo7Seg(char character) {
    switch(character) {
        case '0': return 0;  // 0 on 7-segment display
        case '1': return 1; // 1 on 7-segment display
        case '2': return 2; // 2 on 7-segment display
        case '3': return 3; // 3 on 7-segment display
        case '4': return 4; // 4 on 7-segment display
        case '5': return 5;  // 5 on 7-segment display
        case '6': return 6;// 6 on 7-segment display
        case '7': return 7;  // 7 on 7-segment display
        case '8': return 8;  // 8 on 7-segment display
        case '9': return 9;  // 9 on 7-segment display
        case 'F': return 0x46;
        case 'O': return 0x4F;
        case 'D': return 0x44;
        case 'N': return 0x4E;
        case 'E': return 0x45;
    }
}
void writeDisplay(int hour, int minutes){
    char display[5];
    clearDisplay();
    _delay_ms(10);
    if(hour < 10){
    sprintf(display,"%02d%02d", hour, minutes);
    }
    else{
        sprintf(display,"%d%02d", hour, minutes);
    }
//    
    for (uint8_t i = 0; i < 4; i++) {
        // Send each character of the formatted string to the display
        sendCharacterToDisplay(display[i]);
    }
}
void displayString(const char* str) {
    // Assuming you have a 4-digit display
    clearDisplay();
    for (int i = 0; i < 4; i++) {
        sendCharacterToDisplay(str[i]);
    }
}

void sendCharacterToDisplay(char c){
    uint8_t segment;
    
    segment = mapCharTo7Seg(c);
    TWI0.MADDR = (0x71 << 1) | 0;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        // Handle error: NACK received
        TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
        return;  // Exit if error occurred
    }
    TWI0.MDATA = segment;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
}
void writeDigits(int hour, int minutes) {
    char display[5];
    if(hour < 10){
    sprintf(display,"%02d%02d", hour, minutes);
    }
    else{
        sprintf(display,"%d%02d", hour, minutes);
    }
//    
    for (uint8_t i = 0; i < 4; i++) {
        // Send each character of the formatted string to the display
        sendCharacterToDisplay(display[i]);
    }
}  
void turnOffColon() {
     TWI0.MADDR = (0x71 << 1) | 0;  // Set the TWI address for your display

    // Wait for the TWI interface to be ready
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    
    // Send the decimal control command (0x77)
    TWI0.MDATA = 0x77;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));

    // Send the data byte to turn off the colon (0x00)
    TWI0.MDATA = 0x00;  // No bits set
    while (!(TWI0.MSTATUS & TWI_WIF_bm));

    // Send stop condition
    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
}
void turnOnColon(){

    
    TWI0.MADDR = (0x71 << 1) | 0; 
     while (!(TWI0.MSTATUS & TWI_WIF_bm));
     if (TWI0.MSTATUS & TWI_RXACK_bm) {
        // Handle error: NACK received
        TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
        return;  
    }
    TWI0.MDATA = 0x77;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    TWI0.MDATA = 0x10;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
        
      

    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
}

void ADCEnable() {
    ADC0.CTRLA |= ADC_ENABLE_bm;
    ADC0.COMMAND = ADC_STCONV_bm;
}
void ADCDisable(){
    ADC0.CTRLA &= ~ADC_ENABLE_bm;
}
void update_power_level(){

    // Scale the ADC result to a value between 1-10 for power level
    powerLevel = (((adc_result-1)*9)/1022)+1;
    if (powerLevel < 1) powerLevel = 1;
    if (powerLevel > 10) powerLevel = 10;
    // Display the power level
    writeDigits(0, powerLevel);  // Assuming writeDisplay sends data to the display
    // Wait for the button press to confirm the power level
}
void update_LED_brightness(int powerLevel) {
    // Map powerLevel (1-10) to duty cycle (10% to 100%)
    TCA0.SINGLE.CMP0BUF = powerLevel * 17;
    
}

int main() {
   
    
    PORTA.DIR &= ~PIN4_bm;
    PORTA.DIR &= ~PIN5_bm;
    PORTA.DIR &= ~PIN6_bm;
    PORTA.DIR &= ~PIN7_bm;
    PORTC.DIR |= PIN1_bm;
//    PORTC.DIR |= PIN0_bm;
    
    PORTA.PIN4CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_RISING_gc);
    PORTA.PIN5CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_RISING_gc);
    PORTA.PIN6CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_RISING_gc);
    PORTA.PIN7CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_RISING_gc);
    TWIinit();
    RTC_init();
    ADCInit();
    turnOnColon();
    TCA_normal();
    

    writeDisplay(hr,min);
    color check = RED;
    sei();
    while(1){  
       
        switch(check){
            case RED:
                //writeDisplay(hr, min); 
                stateChanged = 0;
                if(button == 1){
                    if(powerCancel == 1){
                        timerMIN = 0;
                        timerSEC = 0;
                        powerCancel = 0;
                    }
                    if(timeCancel == 1)
                    if(button2 == 1){
                        allowRTC = 1;
                        writeDisplay(hr, min);  // Return to hour:minute display
                        yellowPress = 0;  
                        allowedToPress = 0;
                        check = STATE_CLOCK;
                    }
                    if (redPressedOnce == 0) {
                        
                        // First RED press: Pause RTC, turn on colon, update the display
                        //pauseRTC();
                        allowRTC = 0;
                        turnOnColon();
                        writeDisplay(0, 0);  // Example: Display 00:00 or similar
                        redPressedOnce = 1;  // Mark that RED was pressed once
                        allowedToPress = 1;
                    
                    }else{
                    if (timerMIN > 0 || timerSEC > 0) {
                        //proceed to setup power level
                         redPressedOnce = 0;
                         check = POWER_SETTING; 
                         stateChanged = 1;
                         allowedToPress = 0;
                         
                    } else{
                    // Second RED press: Cancel operation and return to hr:min display
                    //RTC.CTRLA |= RTC_RTCEN_bm;  // Re-enable RTC
                        allowRTC = 1;
                        writeDisplay(hr, min);  // Return to hour:minute display
                        redPressedOnce = 0;  // Reset the flag
                        allowedToPress = 0;
                    }
                    }
                }
               
                button = 0;
                if (stateChanged == 0) {
                    check = YELLOW;
                }
                break;
            case YELLOW:
                stateChanged2 = 0;
                if(button2 == 1){
                    if(timeCancel == 1){
                        timerM2 = 0;
                        timerS2 = 0;
                        timeCancel = 0;
                    }
                    if(yellowPress == 0){
                        allowRTC = 0;
                        
                        turnOnColon();
                        writeDisplay(0, 0);
                        allowedToPress = 1; 
                        yellowPress = 1;
                        kitchenT = 1;
                        
                }
                else{
                    if (timerM2 > 0 || timerS2 > 0) {
                        //proceed to setup Timer Preview
                         yellowPress = 0;
                         
                         timerStart = 1;
                         stateChanged2 = 1;
                         allowedToPress = 0;
                         check = TIMER; 
                         
                    } else{
                    // Second RED press: Cancel operation and return to hr:min display
                    //RTC.CTRLA |= RTC_RTCEN_bm;  // Re-enable RTC
                        allowRTC = 1;
                        writeDisplay(hr, min);  // Return to hour:minute display
                        yellowPress = 0;  // Reset the flag
                        allowedToPress = 0;
                    }
                }
                }
                
                if(button == 1){
                    allowRTC = 1;
                    writeDisplay(hr, min);  // Return to hour:minute display
                    redPressedOnce = 0;  // Reset the flag
                    allowedToPress = 0;
                    check = STATE_CLOCK;    // Return to clock state
                    button = 0;             // Reset the button flag
                    kitchenT = 0;
                    break; 
                }
                button2 = 0;
                if (stateChanged2 == 0) {
                    check = GREEN;
                }
                //check = GREEN;
                break;
            case GREEN:
                if (allowedToPress == 1) {
                    if(button3 == 1){
                        if(kitchenT == 1){
                           timerS2 += 1;
                            if(timerS2%60 == 0){
                                timerM2 += 1;
                                timerS2 = 0;
                               
                            }
                           
                           writeDisplay(timerM2, timerS2);
                           
                            }
                    else{
                        timerSEC += 1;
                        if(timerSEC%60 == 0){
                            timerMIN += 1;
                            timerSEC = 0;
                        }
                    
                        writeDisplay(timerMIN, timerSEC);
                        }
                    }
                }
                button3 = 0;
                check = BLUE;
                
                break;
            case BLUE:
                if (allowedToPress == 1) {
                if(button4 == 1){
                    if(kitchenT == 1){
                           timerS2 += 1;
                            if(timerS2%60 == 0){
                                timerM2 += 1;
                                timerS2 = 0;
                            }
                          
                           writeDisplay(timerM2, timerS2);
                            }
                    else{
                    timerSEC += 5;
                    if(timerSEC%60 == 0){
                        timerMIN += 1;
                        timerSEC = 0;
                    }
                    writeDisplay(timerMIN,timerSEC);
                }
                }
                }
                button4 = 0;
                check = RED;
                break;
            case POWER_SETTING:
                // Enable ADC to read potentiometer data
                if (!adc_enabled) {
                    ADCEnable();
                    adc_enabled = 1;
                }
                if(adc_data_ready){
                    adc_data_ready = 0;             // Reset the flag
                    update_power_level(); 
                    _delay_ms(100); 
                }
                if (button2 == 1){
                    ADCDisable();        // Disable ADC
                    adc_enabled = 0;
                    check = GREEN;  // Transition to the next state (e.g., cooking state)
                    button2 = 0; 
                }
                // User confirms power level
                if (button == 1) {
                    ADCDisable();        // Disable ADC
                    adc_enabled = 0;
                    button = 0;          // Reset button flag
                    timerStart2 = 1;
                    check = STATE_COOKING;  // Transition to the next state (e.g., cooking state)
                    
                    
                }      
                break;
            case TIMER:
//                TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_b
               
                TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
                if(countdown2 == 1){
                    displayString("DONE");
                    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
                    TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc;
                    TCA_freq2(); // Initialize TCA0 in Frequency Generation mode
                    displayDoneTimer = 3; 
                    countdown2 = 0;
                    check = STATE_DISPLAY_DONE; // Transition to a new state
                    
                }
                if(button2 == 1){
                    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // Stop TCA0
                    check = STATE_CLOCK;
                    timerM2 = 0;
                    timerS2 = 0;
                    timeCancel = 1;
                }
               // Check for RED button press to cancel cooking
               if (button == 1) {
              
                   TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // Stop TCA0
                   check = STATE_CLOCK; // Return to clock state
                   button = 0;
                   timeCancel = 1;
               }
            
               
                break;
            case STATE_COOKING:
                allowRTC = 0;
                TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
                   if (!pwm_started) {
                       update_LED_brightness(powerLevel); // Set LED brightness
                       pwm_started = 1;
                   }
                //indicating the countdown has finished
                if(countdown == 1){
                    displayString("FOOD");
                    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
                    TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc;
                    TCA_freq(); // Initialize TCA0 in Frequency Generation mode
                    displayFoodTimer = 3; 
                    
                    check = STATE_DISPLAY_FOOD; // Transition to a new state
                    countdown = 0;
                }
                //when pressing yellow button, cancel timer and go do normal clock
                if(button2 == 1){
                    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // Stop TCA0
                    check = STATE_CLOCK;
                    timerMIN = 0;
                    timerSEC = 0;
                }
               // Check for RED button press to cancel cooking
               if (button == 1) {
                   ADCDisable();     // Ensure ADC is disabled
                   ADC0.INTCTRL = 0; // Disable ADC interrupts
                   pwm_started = 0;  // Reset PWM started flag
                   TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; // Stop TCA0
                   check = STATE_CLOCK; // Return to clock state
                   button = 0;
                   powerCancel = 1;
               }

               break;
            case STATE_DISPLAY_DONE:
                allowRTC = 0;
                countdown2 = 0;
                
                if(!tone_started2){
                    TCA_freq2();
                    tone_started2 = 1;
                }
                if (oneSecondFlag) {
                    oneSecondFlag = 0;
                    if (displayDoneTimer > 0) {
                        displayDoneTimer--;
                    }
                    if (displayDoneTimer == 0) {
                        // Stop the tone and reset TCA0
                        TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
                        TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc;
                        timerCheck = 0;
                        allowRTC = 1; 
                        tone_started2 = 0;
                        check = STATE_CLOCK; // Return to clock state
                    }
                }
            case STATE_DISPLAY_FOOD:
               allowRTC = 0;
               countdown = 0;
                // Start the tone if not already started
                if (!tone_started) {
                    TCA_freq(); // Initialize TCA0 in Frequency Generation mode
                    tone_started = 1; // Flag to indicate tone has started
               }
                
                // Decrement the display timer every second
                if (oneSecondFlag) {
                    oneSecondFlag = 0;
                    if (displayFoodTimer > 0) {
                        displayFoodTimer--;
                    }
                    if (displayFoodTimer == 0) {
                        // Stop the tone and reset TCA0
                        TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
                        TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc;
                        TCA_normal(); 
                        allowRTC = 1; 
                        tone_started = 0;
                        check = STATE_CLOCK; // Return to clock state
                    }
                }

                // Check for RED button press to skip the display
                if (button == 1) {
                    // Stop the tone and reset TCA0
                    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
                    TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc;
                    TCA_normal();
                    allowRTC = 1; 
                    tone_started = 0;
                    check = STATE_CLOCK;
                    button = 0;
                }

            break;
           case STATE_CLOCK:
               clearDisplay();
               writeDisplay(hr, min); 
               allowRTC = 1;
               //RTC.CTRLA |= RTC_RTCEN_bm; 
               if (button == 1) {
                   button = 0;
                   check = RED;             
               }else if(button2 == 1){
                   check = YELLOW;
               }
               
               check = RED;
               break;

           default:
               check = RED;  // Default to RED state
               break;

       }
        
}
}