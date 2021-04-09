################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/internet/mf-util/debug-tag.cc \
../src/internet/mf-util/global-log.cc \
../src/internet/mf-util/global-property.cc \
../src/internet/mf-util/machine-slots.cc \
../src/internet/mf-util/macroflow.cc \
../src/internet/mf-util/parallel-job.cc \
../src/internet/mf-util/token-bucket.cc \
../src/internet/mf-util/trace-reader.cc 

CC_DEPS += \
./src/internet/mf-util/debug-tag.d \
./src/internet/mf-util/global-log.d \
./src/internet/mf-util/global-property.d \
./src/internet/mf-util/machine-slots.d \
./src/internet/mf-util/macroflow.d \
./src/internet/mf-util/parallel-job.d \
./src/internet/mf-util/token-bucket.d \
./src/internet/mf-util/trace-reader.d 

OBJS += \
./src/internet/mf-util/debug-tag.o \
./src/internet/mf-util/global-log.o \
./src/internet/mf-util/global-property.o \
./src/internet/mf-util/machine-slots.o \
./src/internet/mf-util/macroflow.o \
./src/internet/mf-util/parallel-job.o \
./src/internet/mf-util/token-bucket.o \
./src/internet/mf-util/trace-reader.o 


# Each subdirectory must supply rules for building sources it contributes
src/internet/mf-util/%.o: ../src/internet/mf-util/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


