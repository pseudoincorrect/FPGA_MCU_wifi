################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./application/main.obj" \
"./application/src/common_app.obj" \
"./application/src/gpio_app.obj" \
"./application/src/net_app.obj" \
"./application/src/ring_buffer.obj" \
"./application/src/spi_app.obj" \
"./application/src/timer_app.obj" \
"./application/src/uart_app.obj" \
"./application/src/watchdog_app.obj" \
"./linked_files/network_common.obj" \
"./linked_files/startup_ccs.obj" \
"./linked_files/timer_if.obj" \
"./linked_files/uart_if.obj" \
"./linked_files/udma_if.obj" \
"./linked_files/wdt_if.obj" \
"../cc3200v1p32.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \
-l"C:/ti/cc3200_sdk/cc3200-sdk/simplelink/ccs/NON_OS/simplelink.a" \
-l"C:/ti/cc3200_sdk/cc3200-sdk/driverlib/ccs/Release/driverlib.a" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include application/subdir_vars.mk
-include application/src/subdir_vars.mk
-include linked_files/subdir_vars.mk
-include subdir_rules.mk
-include application/subdir_rules.mk
-include application/src/subdir_rules.mk
-include linked_files/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Fpga_wifi_link.out \

EXE_OUTPUTS__QUOTED += \
"Fpga_wifi_link.out" \

BIN_OUTPUTS += \
Fpga_wifi_link.bin \

BIN_OUTPUTS__QUOTED += \
"Fpga_wifi_link.bin" \


# All Target
all: Fpga_wifi_link.out

# Tool invocations
Fpga_wifi_link.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"Fpga_wifi_link.map" --heap_size=40000 --stack_size=40000 --cinit_hold_wdt=off -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/lib" -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="Fpga_wifi_link_linkInfo.xml" --rom_model -o "Fpga_wifi_link.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

Fpga_wifi_link.bin: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armhex"  -o "Fpga_wifi_link.bin" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "application\main.obj" "application\src\common_app.obj" "application\src\gpio_app.obj" "application\src\net_app.obj" "application\src\ring_buffer.obj" "application\src\spi_app.obj" "application\src\timer_app.obj" "application\src\uart_app.obj" "application\src\watchdog_app.obj" "linked_files\network_common.obj" "linked_files\startup_ccs.obj" "linked_files\timer_if.obj" "linked_files\uart_if.obj" "linked_files\udma_if.obj" "linked_files\wdt_if.obj" 
	-$(RM) "application\main.d" "application\src\common_app.d" "application\src\gpio_app.d" "application\src\net_app.d" "application\src\ring_buffer.d" "application\src\spi_app.d" "application\src\timer_app.d" "application\src\uart_app.d" "application\src\watchdog_app.d" "linked_files\network_common.d" "linked_files\startup_ccs.d" "linked_files\timer_if.d" "linked_files\uart_if.d" "linked_files\udma_if.d" "linked_files\wdt_if.d" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-"C:/ti/ccsv7/utils/tiobj2bin/tiobj2bin" "Fpga_wifi_link.out" "Fpga_wifi_link.bin" "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armofd" "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armhex" "C:/ti/ccsv7/utils/tiobj2bin/mkhex4bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

