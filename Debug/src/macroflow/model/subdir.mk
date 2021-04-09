################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/macroflow/model/ci-scheduler.cc \
../src/macroflow/model/master-application.cc \
../src/macroflow/model/mf-scheduler-scf.cc \
../src/macroflow/model/mf-scheduler-smf.cc \
../src/macroflow/model/mf-scheduler.cc \
../src/macroflow/model/ni-scheduler.cc 

CC_DEPS += \
./src/macroflow/model/ci-scheduler.d \
./src/macroflow/model/master-application.d \
./src/macroflow/model/mf-scheduler-scf.d \
./src/macroflow/model/mf-scheduler-smf.d \
./src/macroflow/model/mf-scheduler.d \
./src/macroflow/model/ni-scheduler.d 

OBJS += \
./src/macroflow/model/ci-scheduler.o \
./src/macroflow/model/master-application.o \
./src/macroflow/model/mf-scheduler-scf.o \
./src/macroflow/model/mf-scheduler-smf.o \
./src/macroflow/model/mf-scheduler.o \
./src/macroflow/model/ni-scheduler.o 


# Each subdirectory must supply rules for building sources it contributes
src/macroflow/model/%.o: ../src/macroflow/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


