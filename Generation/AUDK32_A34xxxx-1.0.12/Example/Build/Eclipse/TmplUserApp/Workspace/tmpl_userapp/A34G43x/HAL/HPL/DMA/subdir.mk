################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Platform/HAL/HPL/DMA/hpl_dma.c 

OBJS += \
./HAL/HPL/DMA/hpl_dma.o 

C_DEPS += \
./HAL/HPL/DMA/hpl_dma.d 


# Each subdirectory must supply rules for building sources it contributes
HAL/HPL/DMA/hpl_dma.o: C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Platform/HAL/HPL/DMA/hpl_dma.c HAL/HPL/DMA/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Middleware/ThirdParty/sensirion_sht4x" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Middleware/ThirdParty/lis2de12" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Tiremo_Hal/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Board" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Drivers/led/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Drivers/button/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Drivers/sht40/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Drivers/lis2de12/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Example/Source/TmplUserApp/Tiremo_Lib/Common/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


