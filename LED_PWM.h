/**
 * @file LED_PWM.h
 * @author yewan
 * @brief 普通LED及PWM-LED的控制函数
 * @version 1.0.0
 * @date 2022-11-23
 *
 * @copyright North China University of Science and Technology
 *            Embedded Systems and Internet of Things Applications Lab
 *
 */

#ifndef __LED_PWM_H
#define __LED_PWM_H

#include "Arduino.h"

/*==============================================
    LED
==============================================*/

/* LED GPIO */
#define LED_PIN       (2)

/* LED PWM 相关参数 */
#define LED_PWM_FREQ              (1000)
#define LED_PWM_STEP              (1000)
#define LED_PWM_DUTY_MAX          (LED_PWM_STEP)
#define LED_PWM_DUTY_MIN          (LED_PWM_STEP * 0.9)
#define LED_PWM_DUTY_DIST         (LED_PWM_STEP)

#define LED_PWM_FLASH_CYCLE       (1000)
#define LED_PWM_FLASH_STEP        (LED_PWM_FLASH_CYCLE / (LED_PWM_DUTY_MAX - LED_PWM_DUTY_MIN))

typedef enum{
    LED_ON = HIGH,
    LED_OFF = LOW
}ledState;

/* LED */
typedef struct{
    uint8_t pin;
    uint8_t state;
    uint8_t GPIOmode;
}led_GPIOStruct;

/* LED PWM */
typedef struct{
    uint32_t pwmFreq;
    uint32_t pwmStep;
    uint8_t pin;
    uint8_t GPIOmode;
    uint8_t dir;
    uint16_t pwmDuty;
}ledPwmDutyDir;

/*==============================================
    函数声明
==============================================*/

/**
 * @fn void ledInit(led_GPIOStruct* led)
 * @brief 初始化数字LED
 *
 * @param [led] led_GPIOStruct*
 */
void ledInit(led_GPIOStruct* led);

/**
 * @fn void ledPwmInit(ledPwmDutyDir* led)
 * @brief 初始化模拟(PWM)LED
 *
 * @param [led] ledPwmDutyDir*
 */
void ledPwmInit(ledPwmDutyDir* led);

/**
 * @fn void ledFlash(led_GPIOStruct* led)
 * @brief 切换LED状态
 *
 * @param [led] led_GPIOStruct*
 */
void ledFlash(led_GPIOStruct* led);

/**
 * @fn void writeLedPwmDuty(ledPwmDutyDir* led)
 * @brief 更新模拟(PWM)LED状态
 * @details 该函数会先将传入的状态更新到LED，然后刷新led结构体中的参数
 *           因此会出现参数不对应状态的现象
 *
 * @param [led] ledPwmDutyDir*
 */
void writeLedPwmDuty(ledPwmDutyDir* led);

#endif /* __LED_PWM_H */
