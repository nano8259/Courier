################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/internet/test/error-channel.cc \
../src/internet/test/error-net-device.cc \
../src/internet/test/global-route-manager-impl-test-suite.cc \
../src/internet/test/ipv4-address-generator-test-suite.cc \
../src/internet/test/ipv4-address-helper-test-suite.cc \
../src/internet/test/ipv4-fragmentation-test.cc \
../src/internet/test/ipv4-header-test.cc \
../src/internet/test/ipv4-list-routing-test-suite.cc \
../src/internet/test/ipv4-packet-info-tag-test-suite.cc \
../src/internet/test/ipv4-raw-test.cc \
../src/internet/test/ipv4-test.cc \
../src/internet/test/ipv6-address-generator-test-suite.cc \
../src/internet/test/ipv6-address-helper-test-suite.cc \
../src/internet/test/ipv6-dual-stack-test-suite.cc \
../src/internet/test/ipv6-extension-header-test-suite.cc \
../src/internet/test/ipv6-fragmentation-test.cc \
../src/internet/test/ipv6-list-routing-test-suite.cc \
../src/internet/test/ipv6-packet-info-tag-test-suite.cc \
../src/internet/test/ipv6-test.cc \
../src/internet/test/tcp-test.cc \
../src/internet/test/udp-test.cc 

CC_DEPS += \
./src/internet/test/error-channel.d \
./src/internet/test/error-net-device.d \
./src/internet/test/global-route-manager-impl-test-suite.d \
./src/internet/test/ipv4-address-generator-test-suite.d \
./src/internet/test/ipv4-address-helper-test-suite.d \
./src/internet/test/ipv4-fragmentation-test.d \
./src/internet/test/ipv4-header-test.d \
./src/internet/test/ipv4-list-routing-test-suite.d \
./src/internet/test/ipv4-packet-info-tag-test-suite.d \
./src/internet/test/ipv4-raw-test.d \
./src/internet/test/ipv4-test.d \
./src/internet/test/ipv6-address-generator-test-suite.d \
./src/internet/test/ipv6-address-helper-test-suite.d \
./src/internet/test/ipv6-dual-stack-test-suite.d \
./src/internet/test/ipv6-extension-header-test-suite.d \
./src/internet/test/ipv6-fragmentation-test.d \
./src/internet/test/ipv6-list-routing-test-suite.d \
./src/internet/test/ipv6-packet-info-tag-test-suite.d \
./src/internet/test/ipv6-test.d \
./src/internet/test/tcp-test.d \
./src/internet/test/udp-test.d 

OBJS += \
./src/internet/test/error-channel.o \
./src/internet/test/error-net-device.o \
./src/internet/test/global-route-manager-impl-test-suite.o \
./src/internet/test/ipv4-address-generator-test-suite.o \
./src/internet/test/ipv4-address-helper-test-suite.o \
./src/internet/test/ipv4-fragmentation-test.o \
./src/internet/test/ipv4-header-test.o \
./src/internet/test/ipv4-list-routing-test-suite.o \
./src/internet/test/ipv4-packet-info-tag-test-suite.o \
./src/internet/test/ipv4-raw-test.o \
./src/internet/test/ipv4-test.o \
./src/internet/test/ipv6-address-generator-test-suite.o \
./src/internet/test/ipv6-address-helper-test-suite.o \
./src/internet/test/ipv6-dual-stack-test-suite.o \
./src/internet/test/ipv6-extension-header-test-suite.o \
./src/internet/test/ipv6-fragmentation-test.o \
./src/internet/test/ipv6-list-routing-test-suite.o \
./src/internet/test/ipv6-packet-info-tag-test-suite.o \
./src/internet/test/ipv6-test.o \
./src/internet/test/tcp-test.o \
./src/internet/test/udp-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/internet/test/%.o: ../src/internet/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


