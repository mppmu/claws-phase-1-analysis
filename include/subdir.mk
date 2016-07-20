

INCLUDES +=
CFLAGS   +=

# Add inputs and outputs from these tool invocations to the build variables
CPP_SRCS += \
../src/Even.cpp

OBJS += \
./src/Event.o

CPP_DEPS += \
./src/EVENT.d

# Each subdirectory must supply rules for building sources it contributes

./build/src/%.o: ./src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ $(CFLAGS) $(INCLUDES) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
