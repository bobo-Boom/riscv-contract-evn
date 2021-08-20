################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../init/startup.S 

OBJS += \
./init/startup.o 

S_UPPER_DEPS += \
./init/startup.d 


# Each subdirectory must supply rules for building sources it contributes
init/%.o: ../init/%.S init/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross Assembler'
	riscv64-unknown-elf-gcc -march=rv64imc -mabi=lp64 -msmall-data-limit=8 -mno-save-restore -O2 -fno-builtin  -g3 -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


