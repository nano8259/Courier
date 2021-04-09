################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/emu/model/emu-encode-decode.cc \
../src/emu/model/emu-net-device.cc \
../src/emu/model/emu-sock-creator.cc 

CC_DEPS += \
./src/emu/model/emu-encode-decode.d \
./src/emu/model/emu-net-device.d \
./src/emu/model/emu-sock-creator.d 

OBJS += \
./src/emu/model/emu-encode-decode.o \
./src/emu/model/emu-net-device.o \
./src/emu/model/emu-sock-creator.o 


# Each subdirectory must supply rules for building sources it contributes
src/emu/model/%.o: ../src/emu/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


