################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/macroflow/util/global-log.cc \
../src/macroflow/util/global-property.cc \
../src/macroflow/util/macroflow.cc \
../src/macroflow/util/parallel-job.cc \
../src/macroflow/util/trace-reader.cc 

CC_DEPS += \
./src/macroflow/util/global-log.d \
./src/macroflow/util/global-property.d \
./src/macroflow/util/macroflow.d \
./src/macroflow/util/parallel-job.d \
./src/macroflow/util/trace-reader.d 

OBJS += \
./src/macroflow/util/global-log.o \
./src/macroflow/util/global-property.o \
./src/macroflow/util/macroflow.o \
./src/macroflow/util/parallel-job.o \
./src/macroflow/util/trace-reader.o 


# Each subdirectory must supply rules for building sources it contributes
src/macroflow/util/%.o: ../src/macroflow/util/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


