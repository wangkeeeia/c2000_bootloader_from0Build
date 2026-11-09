################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
device/%.obj: ../device/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 -Ooff --include_path="D:/001work/07Bootloader_build/flash_kernel_ex3_sci_flash_kernel" --include_path="D:/001work/07Bootloader_build/flash_kernel_ex3_sci_flash_kernel/device" --include_path="C:/ti/C2000Ware_26_01_00_00/driverlib/f28e12x/driverlib" --include_path="C:/ti/C2000Ware_26_01_00_00/libraries/boot_rom/f28e12x/rev0/rom_sources/F28E12x_ROM/inc" --include_path="C:/ti/C2000Ware_26_01_00_00/libraries/flash_api/f28e12x/lib" --include_path="C:/ti/C2000Ware_26_01_00_00/libraries/flash_api/f28e12x/include/FlashAPI" --include_path="C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/include" --define=CPU1 --define=_LAUNCHXL_F28E12X --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="device/$(basename $(<F)).d_raw" --obj_directory="device" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


