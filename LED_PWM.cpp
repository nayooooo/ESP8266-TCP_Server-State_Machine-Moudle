/**
 * @file LED_PWM.cpp
 * @author yewan
 * @brief 普通LED及PWM-LED的控制函数
 * @version 1.0.0
 * @date 2022-11-23
 *
 * @copyright North China University of Science and Technology
 *            Embedded Systems and Internet of Things Applications Lab
 *
 */

#include "Arduino.h"
#include "LED_PWM.h"

/*==============================================
    控制函数
==============================================*/

void ledInit(led_GPIOstruct* led)
{
    delay(1000);
    pinMode(led->pin, led->GPIOmode);
}

void ledPwmInit(ledPwmDutyDir* led)
{
    delay(1000);
    pinMode(led->pin, led->GPIOmode);
    analogWriteFreq(led->pwmFreq);          // 1000Hz
    analogWriteRange(led->pwmStep);         // step = 1us
    analogWrite(led->pin, led->pwmDuty);
}

void ledFlash(led_GPIOstruct* led)
{
    if(led->state == LED_ON) led->state = LED_OFF;
    else led->state = LED_ON;
    digitalWrite(led->pin, led->state);
}

void writeLedPwmDuty(ledPwmDutyDir* led)
{
    analogWrite(led->pin, led->pwmDuty);
    if(led->dir) led->pwmDuty++;
    else led->pwmDuty--;
    if(led->pwmDuty <= LED_PWM_DUTY_MIN) led->dir = 1;
    if(led->pwmDuty >= LED_PWM_DUTY_MAX) led->dir = 0;
    // Serial.printf("led pwm duty: %.3f\r\n", 1.0 * (led->pwmStep - led->pwmDuty) / led->pwmStep);
}
