################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/internet/mf-helper/switch-helper.cc \
../src/internet/mf-helper/tcp-flow-helper.cc \
../src/internet/mf-helper/topology-helper.cc 

CC_DEPS += \
./src/internet/mf-helper/switch-helper.d \
./src/internet/mf-helper/tcp-flow-helper.d \
./src/internet/mf-helper/topology-helper.d 

OBJS += \
./src/internet/mf-helper/switch-helper.o \
./src/internet/mf-helper/tcp-flow-helper.o \
./src/internet/mf-helper/topology-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/internet/mf-helper/%.o: ../src/internet/mf-helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


