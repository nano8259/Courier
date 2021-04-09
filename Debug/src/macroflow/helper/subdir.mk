################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/macroflow/helper/asym-point-to-point-helper.cc \
../src/macroflow/helper/switch-helper.cc \
../src/macroflow/helper/tcp-flow-helper.cc \
../src/macroflow/helper/topology-helper.cc 

CC_DEPS += \
./src/macroflow/helper/asym-point-to-point-helper.d \
./src/macroflow/helper/switch-helper.d \
./src/macroflow/helper/tcp-flow-helper.d \
./src/macroflow/helper/topology-helper.d 

OBJS += \
./src/macroflow/helper/asym-point-to-point-helper.o \
./src/macroflow/helper/switch-helper.o \
./src/macroflow/helper/tcp-flow-helper.o \
./src/macroflow/helper/topology-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/macroflow/helper/%.o: ../src/macroflow/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


