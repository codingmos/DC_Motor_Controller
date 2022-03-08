#include "ses_led.h"
#include "ses_lcd.h"
#include "ses_button.h"
#include "ses_button.c"
#include "ses_timer.h"
#include "ses_timer.c"
#include "util/delay.h"
#include "avr/io.h"
#include "ses_common.h"
#include "ses_scheduler.h"
#include "ses_motorFrequency.h"
#include "ses_motorFrequency.c"
#include "ses_uart.h"
#include "util/atomic.h"
#include "ses_pwm.h"
#include "ses_pwm.c"

/* DEFINES & MACROS **********************************************************/

#define N  				100  // Optimized value for accurate median calculations

// Flags & counters for internal timer
uint16_t timer_count = 0;
uint16_t timer_hundreds = 0;
uint16_t rising_edge_count = 0;

// Array & index counter for median calc
uint16_t median_index = 0;
uint16_t median[N];

// Counter for time to revolutions
uint16_t rev_time = 0;

// Flags for if motor is moving or stopped
bool pause = true;

// Flag for joystick button press
static int press_count = 1;

void leds_init() {
	led_greenInit();
	led_redInit();
	led_yellowInit();
	led_redOff();
	led_greenOff();
	led_yellowOff();
}

// this may need edit... check hardware for timer overflow
void timeCount() {
	timer_count++;
	if (timer_count == 32760) {
		timer_count = 0;
		led_redOn();
		lcd_setCursor(0, 3);
		fprintf(uartout, "Timer Overflow...");
		fprintf(lcdout, "Timer Overflow...");
	}
}

void risingEdgeCounter() {
	rising_edge_count++;
	TCNT5 = 0; // reset timer5 counter so that the callback does not pause RPM outputs
	pause = false;  // turn off pause on getRecent and getMedian RPM outputs
	if (rising_edge_count >= 6) {
		rev_time = timer_count;
		timer_count = 0;
		rising_edge_count = 0;
	}
}

void getRPM() {
	if ((pause == false)) {
		if ((60000 / rev_time) < 32660) {
			uint16_t rpm = 60000 / rev_time; // convert milliseconds to RPM
			// Output recent to lcd
			lcd_setCursor(0, 0);
			fprintf(uartout, "%6.2d RPM (recent)", rpm);
			fprintf(lcdout, "%6.2d RPM (recent)", rpm);

			// Clear stopped motor output if present
			lcd_setCursor(0, 2);
			fprintf(uartout, " Motor moving");
			fprintf(lcdout, " Motor moving");

			//initialize and count median array
			median_index++;
			if (median_index < N) {
				median[median_index] = rpm;
			}
		} else {
			// Do Nothing
		}
	} else {
		// Do Nothing
	}
}

void getMedianOutput() {
	if (median_index >= N) {
		//Sort median vector numerically
		int miniPos;
		for (int i = 0; i < N; i++) {
			miniPos = i;
			for (int j = i + 1; j < N; j++) {
				if (median[j] < median[miniPos]) {
					miniPos = j;
				}
			}
			uint16_t temp = median[miniPos];
			median[miniPos] = median[i];
			median[i] = temp;
		}

		// Determine median value
		uint16_t median_rpm;
		if (N % 2 == 0) {
			median_rpm = (median[(N / 2) - 1] + median[N / 2]) / 2;
		} else {
			median_rpm = median[N / 2];
		}

		// Output median to lcd
		lcd_setCursor(0, 1);
		fprintf(uartout, "%6.2d RPM (median)", median_rpm);
		fprintf(lcdout, "%6.2d RPM (median)", median_rpm);
		median_index = 0;
	}
}

void motorStopped() {
	pause = true;
	int paused_rpm = 0;
	lcd_setCursor(0, 0);
	fprintf(uartout, "%6.2d RPM (recent)", paused_rpm);
	fprintf(lcdout, "%6.2d RPM (recent)", paused_rpm);
	lcd_setCursor(0, 1);
	fprintf(uartout, "%6.2d RPM (median)", paused_rpm);
	fprintf(lcdout, "%6.2d RPM (median)", paused_rpm);
	lcd_setCursor(0, 2);
	fprintf(uartout, "Motor Stopped");
	fprintf(lcdout, "Motor Stopped");
	timer_count = 0;
}

void motorJoystickButton() {
	if (press_count == 0) {
		press_count = 1;
		pwm_setDutyCycle(0);
	} else {
		press_count = 0;
		pwm_setDutyCycle(170);
	}
}

void timer5Callback() {
	timer5_setCallback(&motorStopped);
}

int main(void) {

	//Initialization functions
	uart_init(57600);
	lcd_init();
	button_init(true);
	leds_init();
	scheduler_init();
	motorFrequency_init();
	pwm_init();

	// Set callbacks
	setRisingEdgeCallback(&led_yellowToggle); // toggle yellow led at rising edge (motor turn)
	setMotorFrequencyCallback(&risingEdgeCounter); // counter for rising edges

	// start timer 5 for motor stop functions
	timer5_start();

	// Task 5.1 - Toggle yellow led on rising edge
	button_setJoystickButtonCallback(&motorJoystickButton);

	// Tasks 5.2 and 5.3
	// Initialize debouncer function
	taskDescriptor debouncer;
	// Set initial period for execution
	debouncer.expire = 0;
	// Set param variable null
	debouncer.param = NULL;
	// Set periodic repeat time to 5 ms
	debouncer.period = 5;
	// Point to correct function task for debouncing
	debouncer.task = &button_checkState;

	// Initialize timing counter every 1 ms
	taskDescriptor timeCounter;
	// Set initial period for execution
	timeCounter.expire = 0;
	// Set param variable null
	timeCounter.param = NULL;
	// Set periodic repeat time to 1 s
	timeCounter.period = 1;
	// Point to correct function task for counting
	timeCounter.task = &timeCount;

	// Initialize setRecentRPM
	taskDescriptor setRecentRPM;
	// Set initial period for execution
	setRecentRPM.expire = 100;
	// Set param variable null
	setRecentRPM.param = NULL;
	// Set periodic repeat time to 10 Hz = 100 ms
	setRecentRPM.period = 10;
	// Point to correct function task for counting
	setRecentRPM.task = &getRPM;

	// Initialize setMedianRPM
	taskDescriptor setMedianRPM;
	// Set initial period for execution
	setMedianRPM.expire = 0;
	// Set param variable null
	setMedianRPM.param = NULL;
	// Set periodic repeat time to 1 ms
	setMedianRPM.period = 1;
	// Point to correct function task for counting
	setMedianRPM.task = &getMedianOutput;

	// Initialize setMedianRPM
	taskDescriptor setMotorStopped;
	// Set initial period for execution
	setMotorStopped.expire = 0;
	// Set param variable null
	setMotorStopped.param = NULL;
	// Set periodic repeat time to 1 ms
	setMotorStopped.period = 1;
	// Point to correct function task for counting
	setMotorStopped.task = &timer5Callback;

	// Debounce joystick button
	scheduler_add(&debouncer);
	// Start counter
	scheduler_add(&timeCounter);
	// Start pole for recent RPM output
	scheduler_add(&setRecentRPM);
	// Start pole for median RPM output
	scheduler_add(&setMedianRPM);
	// Start pole for median RPM output
	scheduler_add(&setMotorStopped);

	while (1) {
		scheduler_run();
	}

	return 0;
}
