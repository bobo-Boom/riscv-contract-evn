################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../confident_contract/hal.c \
../confident_contract/main.c \
../confident_contract/test.c 

OBJS += \
./confident_contract/hal.o \
./confident_contract/main.o \
./confident_contract/test.o 

C_DEPS += \
./confident_contract/hal.d \
./confident_contract/main.d \
./confident_contract/test.d 


# Each subdirectory must supply rules for building sources it contributes
confident_contract/%.o: ../confident_contract/%.c confident_contract/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv64-unknown-elf-gcc -march=rv64imc -mabi=lp64 -msmall-data-limit=8 -mno-save-restore -O2 -fno-builtin  -g3 -I/home/lzhen/sdk/contract_demo/ContractDemo/lib -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


