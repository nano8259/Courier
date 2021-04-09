################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/tools/model/delay-jitter-estimation.cc \
../src/tools/model/event-garbage-collector.cc \
../src/tools/model/gnuplot.cc 

CC_DEPS += \
./src/tools/model/delay-jitter-estimation.d \
./src/tools/model/event-garbage-collector.d \
./src/tools/model/gnuplot.d 

OBJS += \
./src/tools/model/delay-jitter-estimation.o \
./src/tools/model/event-garbage-collector.o \
./src/tools/model/gnuplot.o 


# Each subdirectory must supply rules for building sources it contributes
src/tools/model/%.o: ../src/tools/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


