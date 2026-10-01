/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct step_ctrl_t
{
    int s;     // 加速过程需要的步数
    int i;     // 当前步
    int steps; // 步数
    float vi;  // 当前速度
    float a;   // 加速度（脉冲/s2）
    uint dir;
} step_ctrl;

typedef struct stepper_channel_t
{
    TIM_HandleTypeDef *timer;
    TIM_TypeDef *timer_instance;
    uint32_t channel;
    GPIO_TypeDef *dir_port;
    uint16_t dir_pin;
    volatile int *busy_flag;
    volatile int *enable_flag;
} stepper_channel;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 全局变量
float speed_factor = 0.1f;
float accel_factor = 0.2f;
int finger_pos[2];
int exit_debug_mode = 0;
g_config cfg;
// flag0~3 和 allow 在定时器中断里修改、在主循环里轮询，必须是 volatile，
// 否则 -O3 下 stepper_wait_all_done() 只读一次标志位，第一次阻塞运动就会死循环。
volatile int flag0;
volatile int flag1;
volatile int flag2;
volatile int flag3;
int test;
int test1;
int test2;
int test3;
int test4 = 0;
volatile int allow[4] = {0, 0, 0, 0};

static step_ctrl stepper_ctrl[4] = {0};

///////////////////////////////////////////////////////////////////////////////////
// ------------------------------ 解析魔方还原步骤 ------------------------------- //
///////////////////////////////////////////////////////////////////////////////////

enum cube_stat {
    STAT_DF=0,STAT_DB,  STAT_DL,  STAT_DR,
    STAT_UF,  STAT_UB,  STAT_UL,  STAT_UR,
    STAT_FL,  STAT_FR,  STAT_FU,  STAT_FD,
    STAT_BL,  STAT_BR,  STAT_BU,  STAT_BD,
    STAT_LF,  STAT_LB,  STAT_LU,  STAT_LD,
    STAT_RF,  STAT_RB,  STAT_RU,  STAT_RD,
};

enum cube_flip {
    FLIP_90_LEFT_CW   = 0,
    FLIP_180_LEFT     = 1,
    FLIP_90_LEFT_CCW  = 2,
    FLIP_90_RIGHT_CW  = 3,
    FLIP_180_RIGHT    = 4,
    FLIP_90_RIGHT_CCW = 5,
    FLIP_NULL         = 7,
};
enum cube_twist {
    TWIST_90_LEFT_CW   = 0,
    TWIST_180_LEFT     = 1,
    TWIST_90_LEFT_CCW  = 2,
    TWIST_90_RIGHT_CW  = 3,
    TWIST_180_RIGHT    = 4,
    TWIST_90_RIGHT_CCW = 5,
    TWIST_NULL         = 7,
};
            
// orig|90,LEFT,CW|180,LEFT|90,LEFT,CCW|90,RIGHT,CW|180,RIGHT|90,RIGHT,CCW
// 使用cube_table.py自动生成
const char cube_status_tab[24][6]={
    // 90,LEFT,CW|180,LEFT|90,LEFT,CCW|90,RIGHT,CW|180,RIGHT|90,RIGHT,CCW
    {STAT_DL, STAT_DB, STAT_DR, STAT_RF, STAT_UF, STAT_LF}, // flip from STAT_DF
    {STAT_DR, STAT_DF, STAT_DL, STAT_LB, STAT_UB, STAT_RB}, // flip from STAT_DB
    {STAT_DB, STAT_DR, STAT_DF, STAT_FL, STAT_UL, STAT_BL}, // flip from STAT_DL
    {STAT_DF, STAT_DL, STAT_DB, STAT_BR, STAT_UR, STAT_FR}, // flip from STAT_DR
    {STAT_UR, STAT_UB, STAT_UL, STAT_LF, STAT_DF, STAT_RF}, // flip from STAT_UF
    {STAT_UL, STAT_UF, STAT_UR, STAT_RB, STAT_DB, STAT_LB}, // flip from STAT_UB
    {STAT_UF, STAT_UR, STAT_UB, STAT_BL, STAT_DL, STAT_FL}, // flip from STAT_UL
    {STAT_UB, STAT_UL, STAT_UF, STAT_FR, STAT_DR, STAT_BR}, // flip from STAT_UR
    {STAT_FD, STAT_FR, STAT_FU, STAT_UL, STAT_BL, STAT_DL}, // flip from STAT_FL
    {STAT_FU, STAT_FL, STAT_FD, STAT_DR, STAT_BR, STAT_UR}, // flip from STAT_FR
    {STAT_FL, STAT_FD, STAT_FR, STAT_RU, STAT_BU, STAT_LU}, // flip from STAT_FU
    {STAT_FR, STAT_FU, STAT_FL, STAT_LD, STAT_BD, STAT_RD}, // flip from STAT_FD
    {STAT_BU, STAT_BR, STAT_BD, STAT_DL, STAT_FL, STAT_UL}, // flip from STAT_BL
    {STAT_BD, STAT_BL, STAT_BU, STAT_UR, STAT_FR, STAT_DR}, // flip from STAT_BR
    {STAT_BR, STAT_BD, STAT_BL, STAT_LU, STAT_FU, STAT_RU}, // flip from STAT_BU
    {STAT_BL, STAT_BU, STAT_BR, STAT_RD, STAT_FD, STAT_LD}, // flip from STAT_BD
    {STAT_LU, STAT_LB, STAT_LD, STAT_DF, STAT_RF, STAT_UF}, // flip from STAT_LF
    {STAT_LD, STAT_LF, STAT_LU, STAT_UB, STAT_RB, STAT_DB}, // flip from STAT_LB
    {STAT_LB, STAT_LD, STAT_LF, STAT_FU, STAT_RU, STAT_BU}, // flip from STAT_LU
    {STAT_LF, STAT_LU, STAT_LB, STAT_BD, STAT_RD, STAT_FD}, // flip from STAT_LD
    {STAT_RD, STAT_RB, STAT_RU, STAT_UF, STAT_LF, STAT_DF}, // flip from STAT_RF
    {STAT_RU, STAT_RF, STAT_RD, STAT_DB, STAT_LB, STAT_UB}, // flip from STAT_RB
    {STAT_RF, STAT_RD, STAT_RB, STAT_BU, STAT_LU, STAT_FU}, // flip from STAT_RU
    {STAT_RB, STAT_RU, STAT_RF, STAT_FD, STAT_LD, STAT_BD}, // flip from STAT_RD
};

// a: 需要进行的操作，取值范围如下
// "U", "R", "F", "D", "L", "B"
// "U'", "R'", "F'", "D'", "L'", "B'"
// "U2", "R2", "F2", "D2", "L2", "B2"
// stat: 魔方的朝向，取值范围如下(0-23)
//    STAT_DF,STAT_DB, STAT_DL, STAT_DR,
//    STAT_UF, STAT_UB, STAT_UL, STAT_UR,
//    STAT_FL, STAT_FR, STAT_FU, STAT_FD,
//    STAT_BL, STAT_BR, STAT_BU, STAT_BD,
//    STAT_LF, STAT_LB, STAT_LU, STAT_LD,
//    STAT_RF, STAT_RB, STAT_RU, STAT_RD,
// last_lr: 最后移动过的机械臂，下次操作时，会尽量选择同侧的
// LEFT  0
// RIGHT 1

// 返回值：魔方的朝向，取值范围和stat一样
static int cube_tweak(int stat, const char *a, int *last_lr, char *flip_twist)
{
    int lr = LEFT;
    int flip = FLIP_NULL;
    int twist = TWIST_NULL;
    const char cube_face_code[6] = {'U', 'R', 'F', 'D', 'L', 'B'};
    const char cube_stat_decode[24] = {
        0x32,0x35,0x34,0x31,
        0x02,0x05,0x04,0x01,
        0x24,0x21,0x20,0x23,
        0x54,0x51,0x50,0x53,
        0x42,0x45,0x40,0x43,
        0x12,0x15,0x10,0x13,
    }; 
    for(int face = 0; face < 6; face ++)
    {
        if(cube_face_code[face] == a[0])
        {
            // 将取值范围0-23的stat转为cube_stat_decode中描述的形式，方便判断可操作的面
            char stat_decode = cube_stat_decode[stat];
            //printf("action == %c%c, status == %c%c\n", a[0], a[1], cube_face_code[stat_decode>>4], cube_face_code[stat_decode&0x0f]);
            if((stat_decode & 0xF0) == (face << 4))
            {
                // 拧左边
                lr = LEFT;
            }
            else if((stat_decode & 0x0F) == face)
            {
                // 拧右边
                lr = RIGHT;
            }
            else
            {
                // 需要调整魔方方向
                int start, end, add, i;
                // last_lr: 最后移动过的机械臂，翻转魔方操作时，尽量选择同侧的机械臂
                // 和上次动作同为90度时，可以节省一些操作步骤
                if(*last_lr == LEFT)
                {
                    start = 0;
                    end = 6;
                    add = 1;
                }
                else
                {
                    start = 5;
                    end = -1;
                    add = -1;
                }
                for(i=start; i != end; i+=add)
                {
                    int next = cube_status_tab[stat][i];
                    char next_stat_decode = cube_stat_decode[next];
                    if((next_stat_decode & 0xF0) == (face << 4))
                    {
                        lr = LEFT;
                        flip = i;
                        stat = next;
                        break;
                    }
                    else if((next_stat_decode & 0x0F) == face)
                    {
                        lr = RIGHT;
                        flip = i;
                        stat = next;
                        break;
                    }
                }
            }
            break;
        }
    }
    *last_lr = lr;
    // 拧魔方
    if('\'' == a[1])
    {
        if(LEFT == lr){
            twist = TWIST_90_LEFT_CCW;
        }else{
            twist = TWIST_90_RIGHT_CCW;
        }
    }
    else if('2' == a[1])
    {
        if(LEFT == lr){
            twist = TWIST_180_LEFT;
        }else{
            twist = TWIST_180_RIGHT;
        }
    }
    else
    {
        if(LEFT == lr){
            twist = TWIST_90_LEFT_CW;
        }else{
            twist = TWIST_90_RIGHT_CW;
        }
    }
    flip_twist[0] = (char)flip;
    flip_twist[1] = (char)twist;
    return stat;
};

// 按照求解结果执行动作
static int cube_tweak_str(int stat, const char *str)
{
    const int MAX_STEP = 25;
    const char *p = str;
    int count = 0;// 步骤数量
    int last_lr = LEFT; // 最后移动过的机械臂，下次操作时，会尽量选择同侧的
    char motion_table[MAX_STEP][2];// {flip, twist}
    while(1)
    {
        if(*p == 'U' || *p == 'R' || *p == 'F' || *p == 'D' || *p == 'L' || *p == 'B'){
            stat = cube_tweak(stat, p, &last_lr, motion_table[count]);
            p++;
            count++;
            if(*p == 0 || count >= MAX_STEP){
                break;
            }
        }
        p++;
        if(*p == 0){
            break;
        }
    }
    ////printf("count=%d\n", count);
    for(int i=0; i<count-1; i++)
    {
        int twist = motion_table[i][1];
        int flip = motion_table[i+1][0];
        if((flip == FLIP_90_LEFT_CW || flip == FLIP_90_LEFT_CCW) && 
           (twist == TWIST_90_LEFT_CW || twist == TWIST_90_LEFT_CCW))
        {
            motion_table[i][1] |= 0x10;
            motion_table[i+1][0] |= 0x10;
        }
        if((flip == FLIP_90_RIGHT_CW || flip == FLIP_90_RIGHT_CCW) && 
           (twist == TWIST_90_RIGHT_CW || twist == TWIST_90_RIGHT_CCW))
        {
            motion_table[i][1] |= 0x10;
            motion_table[i+1][0] |= 0x10;
        }
    }
    for(int i=0; i<count; i++)
    {
        //printf("count = %d\n", i);
        // 翻转魔方动作的编号，例如FLIP_90_LEFT_CW
        int flip = motion_table[i][0] & 0x0F;
        // 拧魔方动作的编号，例如TWIST_90_LEFT_CW
        int twist = motion_table[i][1] & 0x0F;
        // 如果可以跳过手指张开，机械臂旋转90度回零的动作，取值16
        // 如果不能，取值0
        int flip_skip_back_step = motion_table[i][0] & 0x10;
        int twist_skip_back_step = motion_table[i][1] & 0x10;
        switch(flip)
        {
            case FLIP_90_LEFT_CW:
                flip_cube_90(LEFT, CW, flip_skip_back_step);
                break;
            case FLIP_180_LEFT:
                flip_cube_180(LEFT);
                break;
            case FLIP_90_LEFT_CCW:
                flip_cube_90(LEFT, CCW, flip_skip_back_step);
                break;
            case FLIP_90_RIGHT_CW:
                flip_cube_90(RIGHT, CW, flip_skip_back_step);
                break;
            case FLIP_180_RIGHT:
                flip_cube_180(RIGHT);
                break;
            case FLIP_90_RIGHT_CCW:
                flip_cube_90(RIGHT, CCW, flip_skip_back_step);
                break;
            default: // FLIP_NULL
                break;
        }
        switch(twist)
        {
            case TWIST_90_LEFT_CW:
                twist_cube_90(LEFT, CW, twist_skip_back_step);
                break;
            case TWIST_180_LEFT:
                twist_cube_180(LEFT);
                break;
            case TWIST_90_LEFT_CCW:
                twist_cube_90(LEFT, CCW, twist_skip_back_step);
                break;
            case TWIST_90_RIGHT_CW:
                twist_cube_90(RIGHT, CW, twist_skip_back_step);
                break;
            case TWIST_180_RIGHT:
                twist_cube_180(RIGHT);
                break;
            case TWIST_90_RIGHT_CCW:
                twist_cube_90(RIGHT, CCW, twist_skip_back_step);
                break;
            default: // TWIST_NULL
                break;
        }
    }
    return stat;
}

///////////////////////////////////////////////////////////////////////////////////
// ---------------------------------- 脉冲控制 ---------------------------------- //
///////////////////////////////////////////////////////////////////////////////////

static stepper_channel stepper_channels[4] = {
    {&ARM0_T, TIM8, ARM0_ch, PIN_DIR0, GPIO_DIR0, &flag0, &allow[0]},
    {&ARM1_T, TIM2, ARM1_ch, PIN_DIR1, GPIO_DIR1, &flag1, &allow[1]},
    {&ARM2_T, TIM9, ARM2_ch, PIN_DIR2, GPIO_DIR2, &flag2, &allow[2]},
    {&ARM3_T, TIM5, ARM3_ch, PIN_DIR3, GPIO_DIR3, &flag3, &allow[3]},
};

static void stepper_refresh_busy_count(void)
{
    test4 = flag0 + flag1 + flag2 + flag3;
}

static int stepper_is_valid(int sm)
{
    return sm >= 0 && sm < 4;
}

static void stepper_mark_busy(int sm)
{
    if(stepper_is_valid(sm))
    {
        *stepper_channels[sm].busy_flag = 1;
        stepper_refresh_busy_count();
    }
}

static void stepper_mark_done(int sm)
{
    if(stepper_is_valid(sm))
    {
        *stepper_channels[sm].busy_flag = 0;
        stepper_refresh_busy_count();
    }
}

static int stepper_any_busy(void)
{
    return flag0 || flag1 || flag2 || flag3;
}

static void stepper_start_channel(int sm)
{
    if(stepper_is_valid(sm))
    {
        stepper_channel *channel = &stepper_channels[sm];

        HAL_TIM_Base_Start_IT(channel->timer);
        HAL_TIM_PWM_Start(channel->timer, channel->channel);
        *channel->enable_flag = 1;
    }
}

static void stepper_stop_channel(int sm)
{
    if(stepper_is_valid(sm))
    {
        stepper_channel *channel = &stepper_channels[sm];

        HAL_TIM_PWM_Stop(channel->timer, channel->channel);
        HAL_TIM_Base_Start_IT(channel->timer);
    }
}

static void stepper_wait_all_done(void)
{
    while(stepper_any_busy())
    {
    }
}
// 电机加速控制
// gpio_mask 1<<GPIO编号 steps：步数，v0：初始速度（脉冲/s），v：最高速度（脉冲/s），a：加速度（脉冲/s2）
// steps可以为正数或者负数，符号表示方向，但是不能为0
static void stepper_move(int sm, int steps, float v0, float v, float a)
{
    if(!stepper_is_valid(sm))
    {
        return;
    }

    step_ctrl *p_step_ctrl = &stepper_ctrl[sm];

    if(steps < 0)
    {
        p_step_ctrl->dir = 0;
        p_step_ctrl->steps = -steps;
    }
    else
    {
        p_step_ctrl->dir = 1;
        p_step_ctrl->steps = steps;
    }

    p_step_ctrl->vi = v0 * speed_factor;
    p_step_ctrl->a = a * accel_factor;
    p_step_ctrl->s = (int)roundf((v * v - v0 * v0) / (2 * a)); // 计算加速过程需要的步数
    p_step_ctrl->i = 0;
    test1 = sm;

    stepper_start_channel(sm);
}

// 等待电机控制指令执行完毕
static void stepper_move_block(int sm, int steps, float v0, float v, float a)
{
    stepper_mark_busy(sm);
    stepper_move(sm, steps, v0, v, a);
    stepper_wait_all_done();
}

static uint32_t stepper_move_calc(step_ctrl *p_step_ctrl)
{
    int i = p_step_ctrl->i;
    int s = p_step_ctrl->s;
    int steps = p_step_ctrl->steps;
    float vi = p_step_ctrl->vi;
    float a = p_step_ctrl->a;
    float accel;

    test3 = i;

    if(i < s && i < steps / 2)
    {
        accel = a;
    }
    else if(i >= steps - s)
    {
        accel = -a;
    }
    else
    {
        accel = 0;
    }

    vi = sqrtf(vi * vi + 2.0f * accel);
    p_step_ctrl->vi = vi;
    p_step_ctrl->i = i + 1;

    vi = vi * 2;
    if(vi > 1995)
    {
        vi = 1995;
    }

    return (uint32_t)vi;
}

static void stepper_handle_period_elapsed(int sm)
{
    stepper_channel *channel = &stepper_channels[sm];
    step_ctrl *ctrl = &stepper_ctrl[sm];
    uint32_t period = stepper_move_calc(ctrl);

    HAL_GPIO_WritePin(channel->dir_port, channel->dir_pin, ctrl->dir ? GPIO_PIN_RESET : GPIO_PIN_SET);
    channel->timer_instance->ARR = 1000000U / period - 1U;

    if(sm == 2)
    {
        test2 = period;
    }

    if(ctrl->i >= ctrl->steps)
    {
        stepper_mark_done(sm);
        stepper_stop_channel(sm);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    stepper_refresh_busy_count();

    for(int sm = 0; sm < 4; sm++)
    {
        if(htim == stepper_channels[sm].timer && *stepper_channels[sm].enable_flag == 1)
        {
            stepper_handle_period_elapsed(sm);
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////
// ---------------------------------- 运动控制 ---------------------------------- //
///////////////////////////////////////////////////////////////////////////////////

//void zero_point(void)
//{
//    
//     HAL_Delay(100); 
//    // stepper 2
//    stepper_move      (0, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);// 移动一步，由HOLD到RUN电流
//    int step = 0;
//    for (int i = 0; ;i++)
//    {
//        stepper_move_block(2, -1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        step++;
//        if(gpio_get(DIAG2) || i > 500)
//        {
//            break;
//        }
//    }
//     HAL_Delay(100);
//    step = 0;
//    for (int i = 0; ;i++)
//    {
//        stepper_move_block(2, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        step++;
//        if(gpio_get(DIAG2))
//        {
//            //printf("finger 2 home done. step=%d\n", step);
//            break;
//        }
//        if (i > 200 * 16)
//        {
//            puts("finger 2 home error.");
//            error_code(ERROR_ZERO_POINT);
//        }
//    }
//     HAL_Delay(100);
//    stepper_move_block(2, -cfg.FINGER_OFFSET_INIT, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//    // stepper 0
//    step = 0;
//    for (int i = 0; ;i++)
//    {
//        stepper_move      (0, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        stepper_move_block(2, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        step++;
//        if(gpio_get(ITR9606_0))
//        {
//            //printf("arm 0 home done. step=%d\n", step);
//            break;
//        }
//        if ( i > 200 * 16 || gpio_get(DIAG0))
//        {
//            puts("arm 0 home error.");
//            error_code(ERROR_ZERO_POINT);
//        }
//    }
//     HAL_Delay(100);
//    stepper_move      (0, cfg.ARM_OFFSET, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//    stepper_move_block(2, cfg.ARM_OFFSET, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//    // stepper 3
//    stepper_move      (1, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);// 移动一步，由HOLD到RUN电流
//    step = 0;
//    for (int i = 0; ;i++)
//    {
//        stepper_move_block(3, -1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        step++;
//        if(gpio_get(DIAG3) || i > 500)
//        {
//            break;
//        }
//    }
//     HAL_Delay(100);
//    step = 0;
//    for (int i = 0; ;i++)
//    {
//        stepper_move_block(3, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        step++;
//        if(gpio_get(DIAG3))
//        {
//            //printf("finger 3 home done. step=%d\n", step);
//            break;
//        }
//        if (i > 200 * 16)
//        {
//            puts("finger 3 home error.");
//            error_code(ERROR_ZERO_POINT);
//        }
//    }
//     HAL_Delay(100);
//    stepper_move_block(3, -cfg.FINGER_OFFSET_INIT, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//    // stepper 1
//    step = 0;
//    for (int i = 0; ;i++)
//    {
//        stepper_move      (1, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        stepper_move_block(3, 1, cfg.SPEED_HOME, cfg.SPEED_HOME, cfg.ACCEL_HOME);
//        step++;
//        if(gpio_get(ITR9606_1))
//        {
//            //printf("arm 1 home done. step=%d\n", step);
//            break;
//        }
//        if ( i > 200 * 16 || gpio_get(DIAG1))
//        {
//            puts("arm 1 home error.");
//            error_code(ERROR_ZERO_POINT);
//        }
//    }
//     HAL_Delay(100);
//    stepper_move      (1, cfg.ARM_OFFSET, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//    stepper_move_block(3, cfg.ARM_OFFSET, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//    finger_pos[0] = cfg.FINGER_OFFSET_INIT;
//    finger_pos[1] = cfg.FINGER_OFFSET_INIT;
//}

// LEFT 0 RIGHT 1
void move_finger(int lr, int to, float v0, float v, float a)
{
    stepper_move      (lr + 2, finger_pos[lr] - to, v0, v, a);
    finger_pos[lr] = to;
}
void move_finger_block(int lr, int to, float v0, float v, float a)
{
    stepper_move_block(lr + 2, finger_pos[lr] - to, v0, v, a);
    finger_pos[lr] = to;
}
void move_arm_block(int lr, int step, float v0, float v, float a)
{
    stepper_move      (lr, step, v0, v, a);
    stepper_move_block(lr + 2, step, v0, v, a);
}
void move_arm(int lr, int step, float v0, float v, float a)
{
    stepper_move(lr, step, v0, v, a);
    stepper_move(lr + 2, step, v0, v, a);
}
// 夹紧魔方
void move_finger_to_default(void)
{
    move_finger      (LEFT, cfg.FINGER_OFFSET_SPIN, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
    move_finger_block(RIGHT, cfg.FINGER_OFFSET_SPIN, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
}
void move_finger_to_lock(void)
{
    move_finger      (LEFT, cfg.FINGER_OFFSET_LOCK, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
    move_finger_block(RIGHT, cfg.FINGER_OFFSET_LOCK, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
}
// 松开魔方
void move_finger_to_init(void)
{
    move_finger      (LEFT, cfg.FINGER_OFFSET_INIT, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
    move_finger_block(RIGHT, cfg.FINGER_OFFSET_INIT, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
}
void move_finger_to_max(void)
{
    move_finger      (LEFT, cfg.FINGER_OFFSET_MAX, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
    move_finger_block(RIGHT, cfg.FINGER_OFFSET_MAX, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
}

//// 反复移动手指，改善润滑状态
//static void move_finger_repetition()
//{
//    for(int i=0;i<3; i++)
//    {
//        move_finger      (LEFT, cfg.FINGER_OFFSET_MAX, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//        move_finger_block(RIGHT, cfg.FINGER_OFFSET_MAX, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//         HAL_Delay(cfg.DELAY_US_AFTER_FINGER_LOCK);
//        move_finger      (LEFT, 32, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//        move_finger_block(RIGHT, 32, cfg.SPEED_HOME, cfg.SPEED_LOW, cfg.ACCEL_HOME);
//         HAL_Delay(cfg.DELAY_US_AFTER_FINGER_LOCK);
//    }
//    for(int i=0;i<3; i++)
//    {
//        move_arm_block(LEFT, 1600, cfg.V_START_ARM_L, cfg.V_MAX_ARM_L, cfg.A_MAX_ARM_L);
//        move_arm_block(RIGHT, 1600, cfg.V_START_ARM_L, cfg.V_MAX_ARM_L, cfg.A_MAX_ARM_L);
//    }
//}
// 拧180度
// twist_cube_180(LEFT); or twist_cube_180(RIGHT);
void twist_cube_180(int lr)
{
    move_arm_block(lr, 200, cfg.V_START_ARM_L, cfg.V_MAX_ARM_L, cfg.A_MAX_ARM_L);
}
// 拧90度
void twist_cube_90(int lr, int cw_ccw, int skip_back_step)
{
    int dir = cw_ccw ? -1 : 1;
    // 转90度 
    move_arm_block(lr, dir * 100, cfg.V_START_ARM_L, cfg.V_MAX_ARM_L, cfg.A_MAX_ARM_L);
    if(!skip_back_step)
    {
        // 对侧手指锁紧
        move_finger_block(!lr, cfg.FINGER_OFFSET_LOCK, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
         HAL_Delay(cfg.DELAY_US_AFTER_FINGER_LOCK);
        // 手指松开
        move_finger_block(lr, cfg.FINGER_OFFSET_MAX, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
        // 转90度
        move_arm_block(lr, dir * 100, cfg.V_START_ARM, cfg.V_MAX_ARM, cfg.A_MAX_ARM);
        // 手指缩回
        move_finger_block(lr, cfg.FINGER_OFFSET_SPIN, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
        // 对侧手指缩回
        move_finger_block(!lr, cfg.FINGER_OFFSET_SPIN, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
    }
}
void flip_cube_180(int lr)
{
    // 手指锁紧
    move_finger_block(lr, cfg.FINGER_OFFSET_LOCK, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
    HAL_Delay(cfg.DELAY_US_AFTER_FINGER_LOCK);
    // 对侧手指缩回
    move_finger_block(!lr, cfg.FINGER_OFFSET_MAX, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
    // 转180
    move_arm_block(lr, 200, cfg.V_START_ARM_L, cfg.V_MAX_ARM_L, cfg.A_MAX_ARM_L);
    // 手指归位
    move_finger_block(!lr, cfg.FINGER_OFFSET_SPIN, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
    move_finger_block(lr, cfg.FINGER_OFFSET_SPIN, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
}
void flip_cube_90(int lr, int cw_ccw, int skip_back_step)
{
    int dir = cw_ccw ? -1 : 1;
    // 手指锁紧
    move_finger_block(lr, cfg.FINGER_OFFSET_LOCK, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
     HAL_Delay(cfg.DELAY_US_AFTER_FINGER_LOCK);
    // 对侧手指松开
    move_finger_block(!lr, cfg.FINGER_OFFSET_MAX, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
    // 转90
    move_arm_block(lr, dir * 100, cfg.V_START_ARM_L, cfg.V_MAX_ARM_L, cfg.A_MAX_ARM_L);
    if(!skip_back_step)
    {
        // 手指归位
        move_finger_block(!lr, cfg.FINGER_OFFSET_SPIN, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
        move_finger_block(!lr, cfg.FINGER_OFFSET_LOCK, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
         HAL_Delay(cfg.DELAY_US_AFTER_FINGER_LOCK);
        move_finger_block(lr, cfg.FINGER_OFFSET_MAX, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
        // 转90度
        move_arm_block(lr, dir * 100, cfg.V_START_ARM, cfg.V_MAX_ARM, cfg.A_MAX_ARM);
    }
    // 手指缩回
    move_finger_block(lr, cfg.FINGER_OFFSET_SPIN, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
    // 对侧手指缩回
    move_finger_block(!lr, cfg.FINGER_OFFSET_SPIN, cfg.V_START_FINGER, cfg.V_MAX_FINGER, cfg.A_MAX_FINGER);
}

void run_demo(void)
{

    const char scramble_string_a[] = "D R2 U2 L2 B2 D F2 D L2 B2 D L2 F' U B R F2 U L D2 R2";
    const char scramble_string_b[] = "R2 D2 L' U' F2 R' B' U' F L2 D' B2 L2 D' F2 D' B2 L2 U2 R2 D'";
    int stat = STAT_DF;

    stat = cube_tweak_str(stat, scramble_string_a);
    HAL_Delay(1000);
    stat = cube_tweak_str(stat, scramble_string_b);
    HAL_Delay(1000);
    move_finger_to_init();
}

static void app_load_default_config(void)
{
    cfg.SPEED_HOME = 100.0f * RPM;
    cfg.SPEED_LOW = 200.0f * RPM;
    cfg.ACCEL_HOME = 1e5f;
    cfg.V_START_FINGER = 100.0f * RPM;
    cfg.V_MAX_FINGER = 100.0f * RPM;
    cfg.A_MAX_FINGER = 2.5e6f;
    cfg.V_START_ARM = 100.0f * RPM;
    cfg.V_MAX_ARM = 300.0f * RPM;
    cfg.A_MAX_ARM = 1e6f;
    cfg.V_START_ARM_L = 50.0f * RPM;
    cfg.V_MAX_ARM_L = 200.0f * RPM;
    cfg.A_MAX_ARM_L = 0.3e5f;
    cfg.DELAY_US_AFTER_FINGER_LOCK = 10;
    cfg.SPEED_FACTOR_FULL = 1.0f;
    cfg.SPEED_FACTOR_SLOW = 0.125f;
    cfg.ARM_OFFSET = 720;
    cfg.FINGER_OFFSET_LOCK = 230 - 8;
    cfg.FINGER_OFFSET_SPIN = 230 - 4;
    cfg.FINGER_OFFSET_INIT = 230;
    cfg.FINGER_OFFSET_MAX = 400;
    cfg.IRUN = 24;  // IRUN峰值1.17A
    cfg.IHOLD = 13; // IHOLD峰值0.66A
    cfg.SGTHRS = 105;
    cfg.DEBUG_MODE = 0;
    cfg.date[254] = FLASH_CONFIG_VERSION;
}

static void app_init_state(void)
{
    finger_pos[LEFT] = cfg.FINGER_OFFSET_INIT;
    finger_pos[RIGHT] = cfg.FINGER_OFFSET_INIT;

    flag0 = 0;
    flag1 = 0;
    flag2 = 0;
    flag3 = 0;
    stepper_refresh_busy_count();
}

static void app_prepare_hardware(void)
{
    HAL_GPIO_WritePin(PIN_DIR1, GPIO_DIR0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PIN_DIR3, GPIO_DIR2, GPIO_PIN_RESET);
    HAL_Delay(2000);
}

static void app_init(void)
{
    app_load_default_config();
    app_init_state();
    app_prepare_hardware();
}

static void app_run(void)
{
    run_demo();

    while(1)
    {
    }
}
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM8_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_TIM9_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  app_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    app_run();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: //printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
