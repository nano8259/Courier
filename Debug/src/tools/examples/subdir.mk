################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/tools/examples/gnuplot-example.cc 

CC_DEPS += \
./src/tools/examples/gnuplot-example.d 

OBJS += \
./src/tools/examples/gnuplot-example.o 


# Each subdirectory must supply rules for building sources it contributes
src/tools/examples/%.o: ../src/tools/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


