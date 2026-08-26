################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Main/Tiremo_Lib/Tiremo_Hal/Src/subdir.mk
-include Main/Tiremo_Lib/Middleware/ThirdParty/sensirion_sht4x/subdir.mk
-include Main/Tiremo_Lib/Middleware/ThirdParty/lis2de12/subdir.mk
-include Main/Tiremo_Lib/Drivers/sht40/Src/subdir.mk
-include Main/Tiremo_Lib/Drivers/lis2de12/Src/subdir.mk
-include Main/Tiremo_Lib/Drivers/led/Src/subdir.mk
-include Main/Tiremo_Lib/Drivers/button/Src/subdir.mk
-include Main/Tiremo_Lib/Common/Src/subdir.mk
-include Main/subdir.mk
-include HAL/WDT/subdir.mk
-include HAL/VREFBUF/subdir.mk
-include HAL/USART/subdir.mk
-include HAL/UART/subdir.mk
-include HAL/Timer1/subdir.mk
-include HAL/SPI/subdir.mk
-include HAL/SCU/subdir.mk
-include HAL/RTC/subdir.mk
-include HAL/RNG/subdir.mk
-include HAL/QSPI/subdir.mk
-include HAL/QEI/subdir.mk
-include HAL/PWM/subdir.mk
-include HAL/PGA/subdir.mk
-include HAL/PCU/subdir.mk
-include HAL/MPWM/subdir.mk
-include HAL/LPUART/subdir.mk
-include HAL/LCD/subdir.mk
-include HAL/I2C/subdir.mk
-include HAL/HPL/NMI/subdir.mk
-include HAL/HPL/DMA/subdir.mk
-include HAL/FRT/subdir.mk
-include HAL/FMC/subdir.mk
-include HAL/DAC/subdir.mk
-include HAL/CRC/subdir.mk
-include HAL/CMP/subdir.mk
-include HAL/AES/subdir.mk
-include HAL/ADC/subdir.mk
-include Debug/Source/subdir.mk
-include ABOV/Source/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(SX_DEPS)),)
-include $(SX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Tiremo_Secure_IoT_Sw
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
Tiremo_Secure_IoT_Sw.hex \

SECONDARY_SIZE += \
Tiremo_Secure_IoT_Sw.siz \


# All Target
all: main-build

# Main-build Target
main-build: Tiremo_Secure_IoT_Sw.elf secondary-outputs

# Tool invocations
Tiremo_Secure_IoT_Sw.elf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU Arm Cross C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -T "C:/Users/sinan/Desktop/Tiremo_Secure_IoT_Sw/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Source/GCC/gcc_a34g43x.ld" -Xlinker --gc-sections -Wl,-Map,"Tiremo_Secure_IoT_Sw.map" --specs=nano.specs --specs=nosys.specs -Wl,--no-warn-rwx-segment -o "Tiremo_Secure_IoT_Sw.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Tiremo_Secure_IoT_Sw.hex: Tiremo_Secure_IoT_Sw.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU Arm Cross Create Flash Image'
	arm-none-eabi-objcopy -O ihex "Tiremo_Secure_IoT_Sw.elf"  "Tiremo_Secure_IoT_Sw.hex"
	@echo 'Finished building: $@'
	@echo ' '

Tiremo_Secure_IoT_Sw.siz: Tiremo_Secure_IoT_Sw.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU Arm Cross Print Size'
	arm-none-eabi-size --format=berkeley "Tiremo_Secure_IoT_Sw.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SX_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) Tiremo_Secure_IoT_Sw.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build

-include ../makefile.targets
