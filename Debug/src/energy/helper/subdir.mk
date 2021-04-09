################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/energy/helper/basic-energy-source-helper.cc \
../src/energy/helper/energy-model-helper.cc \
../src/energy/helper/energy-source-container.cc \
../src/energy/helper/rv-battery-model-helper.cc \
../src/energy/helper/wifi-radio-energy-model-helper.cc 

CC_DEPS += \
./src/energy/helper/basic-energy-source-helper.d \
./src/energy/helper/energy-model-helper.d \
./src/energy/helper/energy-source-container.d \
./src/energy/helper/rv-battery-model-helper.d \
./src/energy/helper/wifi-radio-energy-model-helper.d 

OBJS += \
./src/energy/helper/basic-energy-source-helper.o \
./src/energy/helper/energy-model-helper.o \
./src/energy/helper/energy-source-container.o \
./src/energy/helper/rv-battery-model-helper.o \
./src/energy/helper/wifi-radio-energy-model-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/energy/helper/%.o: ../src/energy/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


