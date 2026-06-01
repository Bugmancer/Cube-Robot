/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
	#pragma anon_unions  
// 每圈200步（360/1.8），16细分，每一分钟60秒
#define RPM (200.0f * 16.0f / 60.0f)
#define FLASH_CONFIG_VERSION 0x01
	typedef	unsigned int	uint;
typedef union GCONFIG_STRUCT
{
  uint8_t date[256];
  struct
  {
    float SPEED_HOME;
    float SPEED_LOW;
    float ACCEL_HOME;
    float V_START_FINGER;
    float V_MAX_FINGER;
    float A_MAX_FINGER;
    float V_START_ARM;
    float V_MAX_ARM;
    float A_MAX_ARM;
    float V_START_ARM_L;
    float V_MAX_ARM_L;
    float A_MAX_ARM_L;
    int DELAY_US_AFTER_FINGER_LOCK;
    float SPEED_FACTOR_FULL;
    float SPEED_FACTOR_SLOW;
    // ARM_OFFSET 零点距离光电开关的角度，720 * (1.8 / 16) = 81度
    int ARM_OFFSET;

    // FINGER_OFFSET，每个微步代表8*1.5*2*pi/(200*16) = 3*pi/400 = 0.02356mm
    // 利用爪子的型变弹力，可以单臂夹紧魔方，不滑落的状态
    // 如果夹不紧，还原过程中容易松动，导致魔方滑落，改小
    // 如果夹的过紧，导致魔方滑落，容易丢步，改大
    int FINGER_OFFSET_LOCK;
    // 利用爪子的型变弹力，可以拧动魔方的状态
    int FINGER_OFFSET_SPIN;
    // 回零点后，需要手指移动到的位置，比较松，方便放置魔方
    int FINGER_OFFSET_INIT;
    // 允许的最远位置
    int FINGER_OFFSET_MAX;
    // IRUN IHOLD取值范围0-31
    // Irms = (CS + 1)/32*180/(100+20)/sqrt(2)
    // Ip   = (CS + 1)/32*180/(100+20)
    // 电机厂商标称的最大电流,通常为峰值
    int IRUN;
    int IHOLD;
    // SGTHRS 无限位归零阈值，数值越大，越容易触发
    // 如果回零时有较大响声，改大
    // 如果回零时提前停止，说明灵敏度过高，改小
    int SGTHRS;
    int DEBUG_MODE;
  };
} g_config;

/*
方向定义
FINGER_OFFSET_ZERO
DIR=0
  |
  |
  |
DIR=1
FINGER_OFFSET_MAX
*/

// 电机0 2
#define LEFT 0
// 电机1 3
#define RIGHT 1

#define CCW 0
#define CW 1

extern g_config cfg;
extern int exit_debug_mode;

void zero_point(void);
void move_finger_to_default(void);
void move_finger_to_init(void);
void move_finger_to_max(void);
void move_finger_to_lock(void);
void twist_cube_180(int lr);
void twist_cube_90(int lr, int cw_ccw, int skip_back_step);
void flip_cube_180(int lr);
void flip_cube_90(int lr, int cw_ccw, int skip_back_step);
void run_demo(void);

/*
flip_cube_90(LEFT, CCW, 0);
flip_cube_90(RIGHT, CCW, 0);
flip_cube_90(LEFT, CW, 0);
flip_cube_90(RIGHT, CW, 0);
twist_cube_180(LEFT);
twist_cube_180(RIGHT);
twist_cube_90(LEFT, CCW, 0);
twist_cube_90(LEFT, CW, 0);
twist_cube_90(RIGHT, CCW, 0);
twist_cube_90(RIGHT, CW, 0);
flip_cube_180(LEFT);
flip_cube_180(RIGHT);
*/
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#ifndef _PIN_DEF_H
#define _PIN_DEF_H

// pins
#define PIN_BUTTON_0 0
#define PIN_BUTTON_1 1
#define GPIO_BUTTON_0 0
#define GPIO_BUTTON_1 1


#define PIN_DIR2 GPIOI
#define PIN_STEP2 3
#define PIN_DIAG2 6
#define GPIO_DIR2 GPIO_PIN_6
#define GPIO_STEP2 3
#define GPIO_DIAG2 6
#define ARM2_T htim9
#define ARM2_ch TIM_CHANNEL_1


#define PIN_DIR3 GPIOI
#define PIN_STEP3 8
#define PIN_DIAG3 9
#define GPIO_DIR3 GPIO_PIN_0
#define GPIO_STEP3 8
#define GPIO_DIAG3 9
#define ARM3_T htim5
#define ARM3_ch TIM_CHANNEL_2

#define PIN_DIR1 GPIOI
#define PIN_STEP1 11
#define PIN_DIAG1 12
#define GPIO_DIR1 GPIO_PIN_5
#define GPIO_STEP1 11
#define GPIO_DIAG1 12
#define ARM1_T htim2
#define ARM1_ch TIM_CHANNEL_4

#define PIN_DIR0 GPIOI
#define PIN_STEP0 14
#define PIN_DIAG0 15
#define GPIO_DIR0 GPIO_PIN_7
#define GPIO_STEP0 14
#define GPIO_DIAG0 15
#define ARM0_T htim8
#define ARM0_ch TIM_CHANNEL_4


#define PIN_ITR9606_0 16
#define PIN_ITR9606_1 17
#define GPIO_ITR9606_0 16
#define GPIO_ITR9606_1 17

#define PIN_ENN 18
#define PIN_LED 19
#define GPIO_ENN 18
#define GPIO_LED 19

#define PIN_CAMERA_TX 28
#define PIN_CAMERA_RX 29
#define GPIO_CAMERA_TX 28
#define GPIO_CAMERA_RX 29




#define PIN_TMC2209_TX 4
#define PIN_TMC2209_RX 5
#define GPIO_TMC2209_TX 4
#define GPIO_TMC2209_RX 5













#define TMC2209_UART uart1
#define CAMERA_UART uart0

//// error code
//#define ERROR_TMC2209_CONFIG_0 0
//#define ERROR_TMC2209_CONFIG_1 1
//#define ERROR_TMC2209_CONFIG_2 2
//#define ERROR_TMC2209_CONFIG_3 3
//#define ERROR_ZERO_POINT 4

#endif

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
