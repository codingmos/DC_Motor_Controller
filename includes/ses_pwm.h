/*
 * ses_pwm.h
 *
 *  Created on: May 28, 2019
 *      Author: mosta
 */

#ifndef SES_PWM_H_
#define SES_PWM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ses_common.h"

void pwm_init(void);
void pwm_setDutyCycle(uint8_t dC);

#endif /* SES_PWM_H_ */
