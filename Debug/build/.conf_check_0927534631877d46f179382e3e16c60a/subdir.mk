################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/.conf_check_0927534631877d46f179382e3e16c60a/test.cpp 

OBJS += \
./build/.conf_check_0927534631877d46f179382e3e16c60a/test.o 

CPP_DEPS += \
./build/.conf_check_0927534631877d46f179382e3e16c60a/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_0927534631877d46f179382e3e16c60a/%.o: ../build/.conf_check_0927534631877d46f179382e3e16c60a/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


