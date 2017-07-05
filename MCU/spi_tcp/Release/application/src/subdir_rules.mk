################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
application/src/common_app.obj: ../application/src/common_app.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="application/src/common_app.d" --obj_directory="application/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

application/src/ring_buffer.obj: ../application/src/ring_buffer.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="application/src/ring_buffer.d" --obj_directory="application/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

application/src/user_main.obj: ../application/src/user_main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=fpalib -me -Ooff --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.2.LTS/include" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/inc" --include_path="C:/Users/max/programming/C/codeComposer/test_wlan/spi_tcp/application/src" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/include" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink/source" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/example/common" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/driverlib" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/inc" --include_path="C:/ti/cc3200_sdk/cc3200-sdk/simplelink_extlib/provisioninglib" --define=ccs --define=USER_INPUT_ENABLE --define=cc3200 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="application/src/user_main.d" --obj_directory="application/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


