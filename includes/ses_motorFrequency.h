#ifndef SES_MOTOR_FREQ_H_
#define SES_MOTOR_FREQ_H_

/* INCLUDES ******************************************************************/



/* FUNCTION PROTOTYPES *******************************************************/

typedef void (*motorFrequencyCallback)();

void motorFrequency_init();

void setRisingEdgeCallback(motorFrequencyCallback reCallback);

uint16_t motorFrequency_getRecent(int elapsed_t);

uint16_t motorFrequency_getMedian();

#endif /* SES_MOTOR_FREQ_H_ */
