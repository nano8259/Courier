################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/tools/test/average-test-suite.cc \
../src/tools/test/event-garbage-collector-test-suite.cc 

CC_DEPS += \
./src/tools/test/average-test-suite.d \
./src/tools/test/event-garbage-collector-test-suite.d 

OBJS += \
./src/tools/test/average-test-suite.o \
./src/tools/test/event-garbage-collector-test-suite.o 


# Each subdirectory must supply rules for building sources it contributes
src/tools/test/%.o: ../src/tools/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


