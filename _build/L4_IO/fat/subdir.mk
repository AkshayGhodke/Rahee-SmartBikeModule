################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../L4_IO/fat/fatfs_time.c \
../L4_IO/fat/ff.c 

OBJS += \
./L4_IO/fat/fatfs_time.o \
./L4_IO/fat/ff.o 

C_DEPS += \
./L4_IO/fat/fatfs_time.d \
./L4_IO/fat/ff.d 


# Each subdirectory must supply rules for building sources it contributes
L4_IO/fat/%.o: ../L4_IO/fat/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -ffunction-sections -fdata-sections -Wall -Wshadow -Wlogical-op -Wfloat-equal -DBUILD_CFG_MPU=0 -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\newlib" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L0_LowLevel" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L1_FreeRTOS" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L1_FreeRTOS\include" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L1_FreeRTOS\portable\no_mpu" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L2_Drivers" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L2_Drivers\base" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L3_Utils" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L3_Utils\tlm" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L4_IO" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L4_IO\fat" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L4_IO\wireless" -I"C:\STUDY\Sem2\Cmpe244\SJSU_Dev-R\projects\lpc1758_freertos\L5_Application" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


