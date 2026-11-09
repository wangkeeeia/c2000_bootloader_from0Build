################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./dcsm.obj" \
"./f28e12x_flash_kernel_ex3_boot.obj" \
"./f28e12x_flash_kernel_ex3_erase.obj" \
"./f28e12x_flash_kernel_ex3_sci_boot.obj" \
"./f28e12x_flash_kernel_ex3_sci_flash_kernel.obj" \
"./f28e12x_flash_kernel_ex3_sci_get_function.obj" \
"./f28e12x_flash_kernel_ex3_verify.obj" \
"./flash_kernel_ex3_codestartbranch.obj" \
"./device/device.obj" \
"../28e12x_flash_kernel_ex3.cmd" \
"../FAPI_ROM_F28E12x_v5.00.00.lib" \
"C:/ti/C2000Ware_26_01_00_00/driverlib/f28e12x/driverlib/ccs/Debug/driverlib.lib" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include device/subdir_vars.mk
-include subdir_rules.mk
-include device/subdir_rules.mk
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
flash_kernel_ex3_sci_flash_kernel.out 

EXE_OUTPUTS__QUOTED += \
"flash_kernel_ex3_sci_flash_kernel.out" 

BIN_OUTPUTS += \
flash_kernel_ex3_sci_flash_kernel.txt 

BIN_OUTPUTS__QUOTED += \
"flash_kernel_ex3_sci_flash_kernel.txt" 


# All Target
all: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "flash_kernel_ex3_sci_flash_kernel.out" secondary-outputs

# Tool invocations
flash_kernel_ex3_sci_flash_kernel.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C2000 Linker'
	"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 -Ooff --define=CPU1 --define=_LAUNCHXL_F28E12X --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"flash_kernel_ex3_sci_flash_kernel.map" --heap_size=0x100 --stack_size=0x200 --warn_sections -i"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/lib" -i"C:/ti/C2000Ware_26_01_00_00/libraries/flash_api/f28e12x/lib" -i"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/include" --reread_libs --diag_wrap=off --display_error_number --xml_link_info="flash_kernel_ex3_sci_flash_kernel_linkInfo.xml" --entry_point=code_start --rom_model -o "flash_kernel_ex3_sci_flash_kernel.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

flash_kernel_ex3_sci_flash_kernel.txt: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: C2000 Hex Utility'
	"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/bin/hex2000" --diag_wrap=off --boot --sci8 --ascii -o "flash_kernel_ex3_sci_flash_kernel.txt" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "dcsm.obj" "f28e12x_flash_kernel_ex3_boot.obj" "f28e12x_flash_kernel_ex3_erase.obj" "f28e12x_flash_kernel_ex3_sci_boot.obj" "f28e12x_flash_kernel_ex3_sci_flash_kernel.obj" "f28e12x_flash_kernel_ex3_sci_get_function.obj" "f28e12x_flash_kernel_ex3_verify.obj" "flash_kernel_ex3_codestartbranch.obj" "device\device.obj" 
	-$(RM) "dcsm.d" "f28e12x_flash_kernel_ex3_boot.d" "f28e12x_flash_kernel_ex3_erase.d" "f28e12x_flash_kernel_ex3_sci_boot.d" "f28e12x_flash_kernel_ex3_sci_flash_kernel.d" "f28e12x_flash_kernel_ex3_sci_get_function.d" "f28e12x_flash_kernel_ex3_verify.d" "device\device.d" 
	-$(RM) "flash_kernel_ex3_codestartbranch.d" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/bin/hex2000" "flash_kernel_ex3_sci_flash_kernel.out" -boot -sci8 --boot_align_sect -a -o "flash_kernel_ex3_sci_flash_kernel.txt"
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

