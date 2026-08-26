################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Tiremo_Lib/Tiremo_Hal/Source/tiremo_hal_systick.c \
../Tiremo_Lib/Tiremo_Hal/Source/tiremo_hal_uart.c 

OBJS += \
./Tiremo_Lib/Tiremo_Hal/Source/tiremo_hal_systick.o \
./Tiremo_Lib/Tiremo_Hal/Source/tiremo_hal_uart.o 

C_DEPS += \
./Tiremo_Lib/Tiremo_Hal/Source/tiremo_hal_systick.d \
./Tiremo_Lib/Tiremo_Hal/Source/tiremo_hal_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Tiremo_Lib/Tiremo_Hal/Source/%.o: ../Tiremo_Lib/Tiremo_Hal/Source/%.c Tiremo_Lib/Tiremo_Hal/Source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Tiremo_Lib/Common/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Tiremo_Lib/Board/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Tiremo_Lib/Drivers/led/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Tiremo_Lib/Drivers/button/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Tiremo_Lib/Drivers/sht40/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Tiremo_Lib/Tiremo_Hal/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


