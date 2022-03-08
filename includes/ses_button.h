#ifndef SES_BUTTON_H_
#define SES_BUTTON_H_

/* INCLUDES ******************************************************************/

#include "ses_common.h"

//Pin change interrupt enable 0 in PCICR register
#define INTERRUPT_DDR				PCICR
#define PIN_MASK_DDR			   PCMSK0
#define PCIE0_BIT                       0
#define PCIE1_BIT						1
#define PCIE2_BIT						2

/* FUNCTION PROTOTYPES *******************************************************/

/**
 * Initializes rotary encoder and joystick button
 */
void button_init(bool debouncing);

/** 
 * Get the state of the joystick button.
 */
bool button_isJoystickPressed(void);

/** 
 * Get the state of the rotary button.
 */
bool button_isRotaryPressed(void);

// Exercise 3.1: Extending button.h
typedef void (*pButtonCallback)();
void button_setRotaryButtonCallback(pButtonCallback callback);
void button_setJoystickButtonCallback(pButtonCallback callback);
bool getRotaryDebounced();
void button_checkState();

#endif /* SES_BUTTON_H_ */
