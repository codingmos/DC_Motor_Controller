/* INCLUDES ******************************************************************/
// http://www.gammon.com.au/forum/?id=11504
// ^good source for this task
#include "ses_common.h"
#include "ses_led.h"
#include "ses_lcd.h"
#include "ses_motorFrequency.h"

/* DEFINES & MACROS **********************************************************/

#define SREG1_BIT						   7
#define INT0_BIT						   0
#define ICS00_BIT						   0
#define ICS01_BIT						   1

motorFrequencyCallback rising_edge_ptr = NULL;
motorFrequencyCallback motor_frequency_ptr = NULL;

void motorFrequency_init() {

	//Initialize DDR register and ports
	DDRD &= ~(1 << DDD0); // Clear the PD2 pin - PD0 (PCINT0 pin) is now an input

	PORTD |= (1 << PORTD0); // turn On the Pull-up - PD0 is now an input with pull-up enabled

	//SREG |= (1 << SREG1_BIT);  // may not be needed here

	// Turn on INT0
	EIMSK |= (1 << INT0_BIT);

	//Writing a 1 to both bits in EICRA register enables rising edge interrupt request at INT0 bit
	EICRA |= ((1 << ICS00_BIT) | (1 << ICS01_BIT));

	// Enable interrupts
	sei();
}

void setRisingEdgeCallback(motorFrequencyCallback reCallback) {
	rising_edge_ptr = reCallback;
}

void setMotorFrequencyCallback(motorFrequencyCallback mfCallback) {
	motor_frequency_ptr = mfCallback;
}

//Interrupt Service Routine for INT0
ISR(INT0_vect) {
	if ((rising_edge_ptr != NULL)) {
		rising_edge_ptr();
	}
	if ((motor_frequency_ptr != NULL)) {
		motor_frequency_ptr();
	}
}
