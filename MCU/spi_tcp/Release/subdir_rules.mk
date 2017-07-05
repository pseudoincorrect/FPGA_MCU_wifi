################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
main.obj: ../main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="main.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

network_common.obj: C:/ti/cc3200_sdk/cc3200-sdk/example/common/network_common.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="network_common.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup_ccs.obj: C:/ti/cc3200_sdk/cc3200-sdk/example/common/startup_ccs.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="startup_ccs.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

uart_if.obj: C:/ti/cc3200_sdk/cc3200-sdk/example/common/uart_if.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="uart_if.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

udma_if.obj: C:/ti/cc3200_sdk/cc3200-sdk/example/common/udma_if.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="udma_if.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


