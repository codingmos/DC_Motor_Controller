/*ses_pwm.c*/
#include "ses_pwm.h"
#include "ses_common.h"
#include "ses_adc.c"
#include "avr/io.h"

void pwm_init(void) {

	//Enable Timer0
	PRR0 &= ~(1 << PRTIM0);

	//Fast PWM Mode -->  WG00:WG02 = 1 1 1
	TCCR0A |= (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << WGM02);

	//Disable Prescaler For Timer0 : F_CPU = 16MHz
	TCCR0B &= ~((1 << CS01) | (1 << CS02));
	TCCR0B |= (1 << CS00);
	//final desired TCCR0B value is = 0B00001001;

	//Set PORTG5/OC0B Pin To Output
	DDRG |= (1 << DDG5);

	//Output Compare Match Timer0
	//COM0B0:1 = 1 0  /noninverting mode pg234 table17-3
	TCCR0A |= (1 << COM0B1);

	//Timer0 Output Compare Match B Interrupt Enable
	//sei();
	//TIMSK0 = (1 << OCIE0B);

	//Motor Stop At Initialization
	OCR0A = 255;
	pwm_setDutyCycle(0);
}

void pwm_setDutyCycle(uint8_t dutyCycle) {
	//set duty cycle
	if (dutyCycle >= 256 || dutyCycle < 0) {
		return; //dont start if given invalid values
	} else {
		OCR0B = dutyCycle;
	}
}
