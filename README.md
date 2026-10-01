# 001 STM32F427 魔方机械臂工程

这是一个基于 STM32CubeMX 生成的 STM32F427IIHx 工程，使用 Keil MDK-ARM 构建。当前主程序用于控制四路步进电机驱动的魔方机械臂，按预设魔方步骤执行翻转、旋转和演示动作。

## 目录结构

- `001.ioc`: STM32CubeMX 工程配置。
- `Core/Inc`: 应用和外设头文件。
- `Core/Src`: 应用入口、外设初始化和中断处理。
- `Drivers`: STM32 HAL、CMSIS 驱动库。
- `MDK-ARM`: Keil 工程文件（构建输出 `MDK-ARM/001/` 已被 `.gitignore` 忽略）。
- `tools`: nr_micro_shell 和 ANSI 辅助代码（参与编译，但 `main.c` 目前没有调用）。

## 主要入口

主入口在 `Core/Src/main.c`。

启动流程：

1. `HAL_Init()` 初始化 HAL。
2. `SystemClock_Config()` 配置 HSE + PLL 系统时钟。
3. `MX_GPIO_Init()`、`MX_TIMx_Init()`、`MX_USART6_UART_Init()` 初始化外设。
4. `app_init()` 加载默认运动参数、初始化运行状态、准备方向 GPIO。
5. `app_run()` 执行 `run_demo()`，随后停留在空循环。

## main.c 代码分区

- 魔方步骤解析：`cube_tweak()`、`cube_tweak_str()` 将魔方公式转换为机械动作。
- 脉冲控制：`stepper_move()`、`stepper_move_block()` 和 `HAL_TIM_PeriodElapsedCallback()` 负责四路步进电机的加减速脉冲输出。
- 运动控制：`move_finger_*()`、`move_arm*()`、`twist_cube_*()`、`flip_cube_*()` 封装机械臂动作。
- 应用初始化：`app_load_default_config()`、`app_init_state()`、`app_prepare_hardware()` 管理启动默认状态。

## 构建方式

推荐使用 Keil uVision 打开：

```text
MDK-ARM/001.uvprojx
```

然后选择工程目标并执行 Build。HAL 源码已包含在 `Drivers` 中，CMSIS Core 通过 RTE 引用，需要在 Pack Installer 中安装 `Keil::STM32F4xx_DFP` 和 `ARM::CMSIS`。

已验证环境：MDK 5.43、ARMCC 5.06 update 5、CMSIS 6.3.0、STM32F4xx_DFP 3.1.1。

## 注意事项

- `Core/Src/main.c` 保留了 CubeMX 的 `USER CODE` 区域，后续用 CubeMX 重新生成代码时，应确认用户代码没有被覆盖。
- 默认运动参数集中在 `app_load_default_config()` 中，调速度、加速度、夹爪行程时优先修改这里。
- 定时器与电机通道的对应关系集中在 `stepper_channels` 表中，修改硬件映射时优先检查该表和 `Core/Inc/main.h` 中的引脚宏。
- `flag0~3`、`allow[]` 在定时器中断里修改、在主循环里轮询，必须保持 `volatile`。工程使用 -O3，去掉后阻塞运动会卡死。
- 源码中文注释是 GBK 编码，与 Keil 的 code page 936 一致；用 VS Code 等编辑器打开时请选择 GBK。

## 参考项目

- 本工程参考了 [rubiks-cube-robot](https://gitee.com/hemn1990/rubiks-cube-robot)。
