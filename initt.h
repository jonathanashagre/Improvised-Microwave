/* 
 * File:   initt.h
 * Author: jonathana.
 *
 * Created on October 22, 2024, 10:56 PM
 */
extern 
#ifndef INITT_H
#define	INITT_H

#ifdef	__cplusplus
extern "C" {
#endif
void TCA_normal();
void TWIinit();
void RTC_init();
void ADCInit();
void TCA_freq();
void TCA_freq2();
void TCA_pwm();

#ifdef	__cplusplus
}
#endif

#endif	/* INITT_H */

