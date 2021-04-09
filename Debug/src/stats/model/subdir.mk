################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/stats/model/data-calculator.cc \
../src/stats/model/data-collector.cc \
../src/stats/model/data-output-interface.cc \
../src/stats/model/omnet-data-output.cc \
../src/stats/model/packet-data-calculators.cc \
../src/stats/model/sqlite-data-output.cc \
../src/stats/model/time-data-calculators.cc 

CC_DEPS += \
./src/stats/model/data-calculator.d \
./src/stats/model/data-collector.d \
./src/stats/model/data-output-interface.d \
./src/stats/model/omnet-data-output.d \
./src/stats/model/packet-data-calculators.d \
./src/stats/model/sqlite-data-output.d \
./src/stats/model/time-data-calculators.d 

OBJS += \
./src/stats/model/data-calculator.o \
./src/stats/model/data-collector.o \
./src/stats/model/data-output-interface.o \
./src/stats/model/omnet-data-output.o \
./src/stats/model/packet-data-calculators.o \
./src/stats/model/sqlite-data-output.o \
./src/stats/model/time-data-calculators.o 


# Each subdirectory must supply rules for building sources it contributes
src/stats/model/%.o: ../src/stats/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


