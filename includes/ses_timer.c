/* INCLUDES ******************************************************************/
#include "ses_timer.h"

/* DEFINES & MACROS **********************************************************/
#define TIMER1_CYC_FOR_5MILLISEC         155 //???? 1240
#define TIMER2_CYC_FOR_1MILLISEC	     248 // 248
#define TIMER5_CYC_FOR_10HERTZ	   	   24999 // 250*x - 1 for x ms ==> 100 ms for 10 Hz

//Timer 5
#define OCIE5A_BIT                         1
#define OCF5A_BIT                          1
#define WGM50_BIT                          0
#define WGM51_BIT                          1
#define WGM52_BIT                          3
#define WGM53_BIT                          4
#define CS50_BIT                           0
#define CS51_BIT                           1
#define CS52_BIT                           2

//Timer 2
#define OCIE2A_BIT                         1
#define OCF2A_BIT                          1
#define WGM21_BIT                          1
#define CS2_BIT                            2
#define SREG1_BIT						   7

//Timer 1
#define OCIE1A_BIT                         1
#define OCF1A_BIT                          1
#define CS1_BIT                            2
#define WGM12_BIT                          3

#define SREG1_BIT						   7

pTimerCallback timer5_ptr = NULL;
pTimerCallback timer2_ptr = NULL;
pTimerCallback timer1_ptr = NULL;

static uint32_t currentTime = 0;
uint32_t overflowTime = 0;

/*FUNCTION DEFINITION ********************************************************/

void timer5_setCallback(pTimerCallback cb) {
	timer5_ptr = cb;
}

void timer5_start() {
	/*
	 *  Use CTC mode
	 */
	PRR1 &= ~(1 << PRTIM5);

	//Writing 0x04 to WGM5(0-3) bits enables CTC mode
	TCCR5B = (1 << WGM52_BIT);

	//Set a prescaler of 64
	TCCR5B |= ((1 << CS50_BIT) | (1 << CS52_BIT));      //0x05

	SREG |= (1 << SREG1_BIT);

	// enable compare interrupt
	TIMSK5 |= (1 << OCIE5A_BIT);

	/* Clear interrupt flag
	 * TIFR5 : Timer 5 interrupt flag register
	 * OCF5A bit : Output compare flag 5 A
	 *         1 : interrupt flag cleared
	 */
	TIFR5 |= (1 << OCF5A_BIT);

	OCR5A = TIMER5_CYC_FOR_10HERTZ;
}

void timer5_stop() {
	// stop timer: write 0  to the CS bits in TCCR5B
	TCCR5B &= ((0 << CS50_BIT) | (0 << CS51_BIT) | (0 << CS52_BIT));
}

void timer2_setCallback(pTimerCallback cb) {
	timer2_ptr = cb;
}

void timer2_start() {
	/*  Use CTC mode
	 *  WGM 22:20
	 *  So WGM21 = 1 as 010 = 0x2 is CTC mode
	 */

	TCCR2A = (1 << WGM21_BIT);     //0x02

	//Set a prescaler of 64
	TCCR2B |= (1 << CS2_BIT);      //0x04

	/* TIMSK2 : Timer 2 interrupt mask register
	 * //TODO: Set interrupt mask register for Compare A
	 *   OCIE2A bit: Timer 2 output compare match A interrupt enable
	 *   OCIE2A bit --> 1
	 */
	SREG |= (1 << SREG1_BIT);

	TIMSK2 |= (1 << OCIE2A_BIT);

	/* Clear interrupt flag
	 * TIFR2 : Timer 2 interrupt flag register
	 * OCF2A bit : Output compare flag 2 A
	 *         1 : interrupt flag cleared
	 */
	TIFR2 |= (1 << OCF2A_BIT);

	/*  Formula on page 316 --> OCRnx => OCR2A
	 * 	OCR2A = 124 in decimals for a period of 1ms
	 */
	OCR2A = TIMER2_CYC_FOR_1MILLISEC;                   //0x7C
	sei();
}

void timer2_stop() {
	// stop timer: write 0  to the CS22 bit in TCCR2B
	TCCR2B &= (0 << CS2_BIT);
	currentTime = 0;
}

uint32_t timer2_getTime(){
	return currentTime;
}

void timer2_setTime(uint32_t time){
	currentTime = time;
}
void timer1_setCallback(pTimerCallback cb) {
	timer1_ptr = cb;
}

void timer1_start() {
	/*  Enable CTC mode
	 *  WGM13:10
	 *  So WGM12 = 1 as 0100 = 0x04 => CTC mode
	 *  WGM12 is bit 3 in TCCR1B
	 */
	TCCR1B |= (1 << WGM12_BIT);

	// Set prescaler of 256 to obtain a vaild OCRnA value
	TCCR1B |= (1 << CS1_BIT);

	/* TIMSK1 : Timer 1 interrupt mask register
	 * //TODO: Set interrupt mask register for Compare A
	 *   OCIE1A bit: Timer 1 output compare match A interrupt enable
	 *   OCIE1A bit --> 1
	 */
	SREG |= (1 << SREG1_BIT);

	TIMSK1 |= (1 << OCIE1A_BIT);

	/* Clear interrupt flag
	 * TIFR1 : Timer 1 interrupt flag register
	 * OCF1A bit : Output compare flag 1 A
	 *         1 : interrupt flag cleared
	 */
	TIFR1 |= (1 << OCF1A_BIT);

	/*  OCR1A
	 *  OCR1A = 155 in decimals for a period of 5ms
	 */
	OCR1AL = TIMER1_CYC_FOR_5MILLISEC;
	sei();
}

void timer1_stop() {
	//Stop timer1
	TCCR1B &= (0 << CS1_BIT);
}

ISR(TIMER1_COMPA_vect) {
	if (timer1_ptr != NULL) {
		timer1_ptr();
	}
}

ISR(TIMER2_COMPA_vect) {
	if (timer2_ptr != NULL) {
		timer2_ptr();

		//increment current time every 1 ms when an interrupt is triggered.
		currentTime++;

		//calculating the time until the timer overflows
		int n = 0;
		if( (TIFR2 & (1 << TOV2)) == (1 << TOV2)){
			overflowTime =  currentTime - n*overflowTime;
			n++;
		}
	}
}

ISR (TIMER5_COMPA_vect) {
	if (timer5_ptr != NULL) {
		timer5_ptr();
	}
}
