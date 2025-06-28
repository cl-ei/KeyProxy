################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/custom/inner_main.c \
../Core/Inc/custom/master.c \
../Core/Inc/custom/message.c \
../Core/Inc/custom/slave.c \
../Core/Inc/custom/spia.c \
../Core/Inc/custom/usba.c 

C_DEPS += \
./Core/Inc/custom/inner_main.d \
./Core/Inc/custom/master.d \
./Core/Inc/custom/message.d \
./Core/Inc/custom/slave.d \
./Core/Inc/custom/spia.d \
./Core/Inc/custom/usba.d 

OBJS += \
./Core/Inc/custom/inner_main.o \
./Core/Inc/custom/master.o \
./Core/Inc/custom/message.o \
./Core/Inc/custom/slave.o \
./Core/Inc/custom/spia.o \
./Core/Inc/custom/usba.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/custom/%.o Core/Inc/custom/%.su Core/Inc/custom/%.cyclo: ../Core/Inc/custom/%.c Core/Inc/custom/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/AUDIO/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MTP/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-custom

clean-Core-2f-Inc-2f-custom:
	-$(RM) ./Core/Inc/custom/inner_main.cyclo ./Core/Inc/custom/inner_main.d ./Core/Inc/custom/inner_main.o ./Core/Inc/custom/inner_main.su ./Core/Inc/custom/master.cyclo ./Core/Inc/custom/master.d ./Core/Inc/custom/master.o ./Core/Inc/custom/master.su ./Core/Inc/custom/message.cyclo ./Core/Inc/custom/message.d ./Core/Inc/custom/message.o ./Core/Inc/custom/message.su ./Core/Inc/custom/slave.cyclo ./Core/Inc/custom/slave.d ./Core/Inc/custom/slave.o ./Core/Inc/custom/slave.su ./Core/Inc/custom/spia.cyclo ./Core/Inc/custom/spia.d ./Core/Inc/custom/spia.o ./Core/Inc/custom/spia.su ./Core/Inc/custom/usba.cyclo ./Core/Inc/custom/usba.d ./Core/Inc/custom/usba.o ./Core/Inc/custom/usba.su

.PHONY: clean-Core-2f-Inc-2f-custom

