################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/mobility/test/mobility-trace-test-suite.cc \
../src/mobility/test/ns2-mobility-helper-test-suite.cc \
../src/mobility/test/steady-state-random-waypoint-mobility-model-test.cc \
../src/mobility/test/waypoint-mobility-model-test.cc 

CC_DEPS += \
./src/mobility/test/mobility-trace-test-suite.d \
./src/mobility/test/ns2-mobility-helper-test-suite.d \
./src/mobility/test/steady-state-random-waypoint-mobility-model-test.d \
./src/mobility/test/waypoint-mobility-model-test.d 

OBJS += \
./src/mobility/test/mobility-trace-test-suite.o \
./src/mobility/test/ns2-mobility-helper-test-suite.o \
./src/mobility/test/steady-state-random-waypoint-mobility-model-test.o \
./src/mobility/test/waypoint-mobility-model-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/mobility/test/%.o: ../src/mobility/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


