################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/internet/mf-model/ci-scheduler.cc \
../src/internet/mf-model/master-application.cc \
../src/internet/mf-model/mf-scheduler.cc \
../src/internet/mf-model/ni-scheduler.cc \
../src/internet/mf-model/tracker.cc 

CC_DEPS += \
./src/internet/mf-model/ci-scheduler.d \
./src/internet/mf-model/master-application.d \
./src/internet/mf-model/mf-scheduler.d \
./src/internet/mf-model/ni-scheduler.d \
./src/internet/mf-model/tracker.d 

OBJS += \
./src/internet/mf-model/ci-scheduler.o \
./src/internet/mf-model/master-application.o \
./src/internet/mf-model/mf-scheduler.o \
./src/internet/mf-model/ni-scheduler.o \
./src/internet/mf-model/tracker.o 


# Each subdirectory must supply rules for building sources it contributes
src/internet/mf-model/%.o: ../src/internet/mf-model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


