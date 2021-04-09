################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../examples/energy/energy-model-example.cc 

CC_DEPS += \
./examples/energy/energy-model-example.d 

OBJS += \
./examples/energy/energy-model-example.o 


# Each subdirectory must supply rules for building sources it contributes
examples/energy/%.o: ../examples/energy/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


