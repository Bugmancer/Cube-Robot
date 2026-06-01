#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#include "nr_micro_shell.h"

#include "main.h"
extern g_config cfg;

//#include "pin_def.h"
#define LEFT  0
#define RIGHT 1
#define CCW   0
#define CW    1

#define GET_TIME_START()   uint64_t t_but_1 = uwTick;
#define GET_TIME_END()   uint64_t t_but_2 = uwTick;  printf("Time cost %.3f ms\n", t_but_2-t_but_1 /1000.0f);

//void shell_help_cmd(char argc, char *argv)
//{
//	unsigned int i = 0;
//	for (i = 0; nr_shell.static_cmd[i].fp != NULL; i++)
//	{
//		shell_printf("%s",nr_shell.static_cmd[i].cmd);
//		shell_printf("\n");
//	}
//}

//void shell_zero_point_cmd(char argc, char *argv)
//{
//	GET_TIME_START();
//	zero_point();
//	GET_TIME_END();
//}
//void shell_run_demo_cmd(char argc, char *argv)
//{
//	GET_TIME_START();
//	run_demo();
//	GET_TIME_END();
//}
//void shell_move_finger_to_default_cmd(char argc, char *argv)
//{
//	GET_TIME_START();
//	move_finger_to_default();
//	GET_TIME_END();
//}
//void shell_move_finger_to_init_cmd(char argc, char *argv)
//{
//	GET_TIME_START();
//	move_finger_to_init();
//	GET_TIME_END();
//}
//void shell_move_finger_to_max_cmd(char argc, char *argv)
//{
//	GET_TIME_START();
//	move_finger_to_max();
//	GET_TIME_END();
//}
//void shell_move_finger_to_lock_cmd(char argc, char *argv)
//{
//	GET_TIME_START();
//	move_finger_to_lock();
//	GET_TIME_END();
//}
//static int get_lr(char argc, char *argv, int *lr)
//{
//	int error = 1;
//	if(argc == 2)
//	{
//		char *str = &(argv[argv[1]]);
//		if(!strcmp(str, "left")){
//			*lr = LEFT;
//			error = 0;
//		}else if(!strcmp(str, "right")){
//			*lr = RIGHT;
//			error = 0;
//		}
//	}
//	return error;
//}
//static int get_lr_cw_skip(char argc, char *argv, int *lr, int *cw_ccw, int *skip_back_step)
//{
//	int error = 3;
//	if(argc == 4)
//	{
//		char *str = &(argv[argv[1]]);
//		if(!strcmp(str, "left")){
//			*lr = LEFT;
//			error--;
//		}else if(!strcmp(str, "right")){
//			*lr = RIGHT;
//			error--;
//		}
//		str = &(argv[argv[2]]);
//		if(!strcmp(str, "cw")){
//			*cw_ccw = CW;
//			error--;
//		}else if(!strcmp(str, "ccw")){
//			*cw_ccw = CCW;
//			error--;
//		}
//		str = &(argv[argv[3]]);
//		*skip_back_step = atoi(str);
//		if(*skip_back_step == 0 || *skip_back_step == 1 ){
//			error--;
//		}
//	}
//	return error;
//}
//void shell_twist_cube_180_cmd(char argc, char *argv)
//{
//	int lr = 0;
//	if(get_lr(argc, argv, &lr)){
//		shell_printf("Usage: twist_cube_180 left/right\n");
//	}else{
//		GET_TIME_START();
//		twist_cube_180(lr);
//		GET_TIME_END();
//	}
//}
//void shell_flip_cube_180_cmd(char argc, char *argv)
//{
//	int lr = 0;
//	if(get_lr(argc, argv, &lr)){
//		shell_printf("Usage: flip_cube_180 left/right\n");
//	}else{
//		GET_TIME_START();
//		flip_cube_180(lr);
//		GET_TIME_END();
//	}
//}
//void shell_twist_cube_90_cmd(char argc, char *argv)
//{
//	int lr=0, cw_ccw=0, skip_back_step=0;
//	if(get_lr_cw_skip(argc, argv, &lr, &cw_ccw, &skip_back_step)){
//		shell_printf("Usage: twist_cube_90 left/right cw/ccw 0/1\n");
//	}else{
//		GET_TIME_START();
//		twist_cube_90(lr, cw_ccw, skip_back_step);
//		GET_TIME_END();
//	}
//}
//void shell_flip_cube_90_cmd(char argc, char *argv)
//{
//	int lr=0, cw_ccw=0, skip_back_step=0;
//	if(get_lr_cw_skip(argc, argv, &lr, &cw_ccw, &skip_back_step)){
//		shell_printf("Usage: flip_cube_90 left/right cw/ccw 0/1\n");
//	}else{
//		GET_TIME_START();
//		flip_cube_90(lr, cw_ccw, skip_back_step);
//		GET_TIME_END();
//	}
//}
//void shell_reset_cmd(char argc, char *argv)
//{
//	watchdog_enable(10, 1);
//}
//void shell_exit_cmd(char argc, char *argv)
//{
//	exit_debug_mode = 1;
//}

void shell_get_cmd(char argc, char *argv)
{
	shell_printf("cfg.SPEED_HOME = %f;\n", 			cfg.SPEED_HOME);
	shell_printf("cfg.SPEED_LOW = %f;\n", 			cfg.SPEED_LOW);
	shell_printf("cfg.ACCEL_HOME = %f;\n", 			cfg.ACCEL_HOME);
	shell_printf("cfg.V_START_FINGER = %f;\n", 		cfg.V_START_FINGER);
	shell_printf("cfg.V_MAX_FINGER = %f;\n", 		cfg.V_MAX_FINGER);
	shell_printf("cfg.A_MAX_FINGER = %f;\n", 		cfg.A_MAX_FINGER);
	shell_printf("cfg.V_START_ARM = %f;\n", 		cfg.V_START_ARM);
	shell_printf("cfg.V_MAX_ARM = %f;\n", 			cfg.V_MAX_ARM);
	shell_printf("cfg.A_MAX_ARM = %f;\n", 			cfg.A_MAX_ARM);
	shell_printf("cfg.V_START_ARM_L = %f;\n", 		cfg.V_START_ARM_L);
	shell_printf("cfg.V_MAX_ARM_L = %f;\n", 		cfg.V_MAX_ARM_L);
	shell_printf("cfg.A_MAX_ARM_L = %f;\n", 		cfg.A_MAX_ARM_L);
	shell_printf("cfg.DELAY_US_AFTER_FINGER_LOCK = %d;\n", cfg.DELAY_US_AFTER_FINGER_LOCK);
	shell_printf("cfg.SPEED_FACTOR_FULL = %f;\n",	cfg.SPEED_FACTOR_FULL);
	shell_printf("cfg.SPEED_FACTOR_SLOW = %f;\n", 	cfg.SPEED_FACTOR_SLOW);
	shell_printf("cfg.ARM_OFFSET = %d;\n", 			cfg.ARM_OFFSET);
	shell_printf("cfg.FINGER_OFFSET_LOCK = %d;\n", 	cfg.FINGER_OFFSET_LOCK);
	shell_printf("cfg.FINGER_OFFSET_SPIN = %d;\n",	cfg.FINGER_OFFSET_SPIN);
	shell_printf("cfg.FINGER_OFFSET_INIT = %d;\n", 	cfg.FINGER_OFFSET_INIT);
	shell_printf("cfg.FINGER_OFFSET_MAX = %d;\n", 	cfg.FINGER_OFFSET_MAX);
	shell_printf("cfg.IRUN = %d;\n", 				cfg.IRUN);
	shell_printf("cfg.IHOLD = %d;\n", 				cfg.IHOLD);
	shell_printf("cfg.SGTHRS = %d;\n", 				cfg.SGTHRS);
	shell_printf("cfg.DEBUG_MODE = %d;\n", 			cfg.DEBUG_MODE);
}


void shell_set_cmd(char argc, char *argv)
{
	int is_float = 1;
	if(argc == 3)
	{
		char *str = &(argv[argv[1]]);
		char *num = &(argv[argv[2]]);
		if(!strcmp(str, "SPEED_HOME")){
			cfg.SPEED_HOME = atof(num);
		}else if(!strcmp(str, "SPEED_LOW")){
			cfg.SPEED_LOW = atof(num);
		}else if(!strcmp(str, "ACCEL_HOME")){
			cfg.ACCEL_HOME = atof(num);
		}else if(!strcmp(str, "V_START_FINGER")){
			cfg.V_START_FINGER = atof(num);
		}else if(!strcmp(str, "V_MAX_FINGER")){
			cfg.V_MAX_FINGER = atof(num);
		}else if(!strcmp(str, "A_MAX_FINGER")){
			cfg.A_MAX_FINGER = atof(num);
		}else if(!strcmp(str, "V_START_ARM")){
			cfg.V_START_ARM = atof(num);
		}else if(!strcmp(str, "V_MAX_ARM")){
			cfg.V_MAX_ARM = atof(num);
		}else if(!strcmp(str, "A_MAX_ARM")){
			cfg.A_MAX_ARM = atof(num);
		}else if(!strcmp(str, "V_START_ARM_L")){
			cfg.V_START_ARM_L = atof(num);
		}else if(!strcmp(str, "V_MAX_ARM_L")){
			cfg.V_MAX_ARM_L = atof(num);
		}else if(!strcmp(str, "A_MAX_ARM_L")){
			cfg.A_MAX_ARM_L = atof(num);
		}else if(!strcmp(str, "DELAY_US_AFTER_FINGER_LOCK")){
			cfg.DELAY_US_AFTER_FINGER_LOCK = atoi(num);
		}else if(!strcmp(str, "SPEED_FACTOR_FULL")){
			cfg.SPEED_FACTOR_FULL = atof(num);
		}else if(!strcmp(str, "SPEED_FACTOR_SLOW")){
			cfg.SPEED_FACTOR_SLOW = atof(num);
		}else if(!strcmp(str, "ARM_OFFSET")){
			cfg.ARM_OFFSET = atoi(num);
		}else if(!strcmp(str, "FINGER_OFFSET_LOCK")){
			cfg.FINGER_OFFSET_LOCK = atoi(num);
		}else if(!strcmp(str, "FINGER_OFFSET_SPIN")){
			cfg.FINGER_OFFSET_SPIN = atoi(num);
		}else if(!strcmp(str, "FINGER_OFFSET_INIT")){
			cfg.FINGER_OFFSET_INIT = atoi(num);
		}else if(!strcmp(str, "FINGER_OFFSET_MAX")){
			cfg.FINGER_OFFSET_MAX = atoi(num);
		}else if(!strcmp(str, "IRUN")){
			cfg.IRUN = atoi(num);
		}else if(!strcmp(str, "IHOLD")){
			cfg.IHOLD = atoi(num);
		}else if(!strcmp(str, "SGTHRS")){
			cfg.SGTHRS = atoi(num);
		}else if(!strcmp(str, "DEBUG_MODE")){
			cfg.DEBUG_MODE = atoi(num);
		}
	}else{
		shell_printf("Usage: set_cfg SPEED_HOME 5000\n");
	}
}

const static_cmd_st static_cmd[] = {
//	{"exit", shell_exit_cmd},
//	{"help", shell_help_cmd},
//	{"zero_point", shell_zero_point_cmd},
//	{"run_demo", shell_run_demo_cmd},
//	{"move_finger_to_default", shell_move_finger_to_default_cmd},
//	{"move_finger_to_init", shell_move_finger_to_init_cmd},
//	{"move_finger_to_max", shell_move_finger_to_max_cmd},
//	{"move_finger_to_lock", shell_move_finger_to_lock_cmd},
//	{"twist_cube_180", shell_twist_cube_180_cmd},
//	{"twist_cube_90", shell_twist_cube_90_cmd},
//	{"flip_cube_180", shell_flip_cube_180_cmd},
//	{"flip_cube_90", shell_flip_cube_90_cmd},
//	{"reset", shell_reset_cmd},
//	{"get", shell_get_cmd},
//	{"set", shell_set_cmd},
{"\0", NULL}
};
