################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/.conf_check_39b3cecfac3989b2ed760243cef109e6/test.cpp 

OBJS += \
./build/.conf_check_39b3cecfac3989b2ed760243cef109e6/test.o 

CPP_DEPS += \
./build/.conf_check_39b3cecfac3989b2ed760243cef109e6/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_39b3cecfac3989b2ed760243cef109e6/%.o: ../build/.conf_check_39b3cecfac3989b2ed760243cef109e6/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


