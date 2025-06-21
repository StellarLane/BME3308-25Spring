################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
include/helper.obj: ../include/helper.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/bin/cl430" -vmspx --abi=eabi -g --include_path="C:/ti_53/ccsv5/ccs_base/msp430/include" --include_path="C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/include" --advice:power=all --define=__MSP430F6638__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="include/helper.pp" --obj_directory="include" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

include/tft.obj: ../include/tft.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/bin/cl430" -vmspx --abi=eabi -g --include_path="C:/ti_53/ccsv5/ccs_base/msp430/include" --include_path="C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/include" --advice:power=all --define=__MSP430F6638__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="include/tft.pp" --obj_directory="include" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

include/tft_base.obj: ../include/tft_base.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/bin/cl430" -vmspx --abi=eabi -g --include_path="C:/ti_53/ccsv5/ccs_base/msp430/include" --include_path="C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/include" --advice:power=all --define=__MSP430F6638__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="include/tft_base.pp" --obj_directory="include" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

include/tft_obstacle.obj: ../include/tft_obstacle.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/bin/cl430" -vmspx --abi=eabi -g --include_path="C:/ti_53/ccsv5/ccs_base/msp430/include" --include_path="C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/include" --advice:power=all --define=__MSP430F6638__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="include/tft_obstacle.pp" --obj_directory="include" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

include/tft_plane.obj: ../include/tft_plane.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/bin/cl430" -vmspx --abi=eabi -g --include_path="C:/ti_53/ccsv5/ccs_base/msp430/include" --include_path="C:/ti_53/ccsv5/tools/compiler/msp430_4.1.2/include" --advice:power=all --define=__MSP430F6638__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="include/tft_plane.pp" --obj_directory="include" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


