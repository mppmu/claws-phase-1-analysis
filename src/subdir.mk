


# Add inputs and outputs from these tool invocations to the build variables
# CALI_CPP_SRCS += ../src/Event.cpp
# CALI_CPP_SRCS += ../src/GlobalSettings.cxx

CALI_OBJS += ./build/src/Event.o
CALI_OBJS += ./build/src/GlobalSettings.o

ANA_OBJS += ./build/src/GlobalSettings.o

# CALI_CPP_DEPS += ./build/src/EVENT.d
# CALI_CPP_DEPS += ./build/src/Gloablsettings.d

# Each subdirectory must supply rules for building sources it contributes

./build/src/%.o: ./src/%.cxx
	@echo 'Building target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Compiler'
	g++ $(CFLAGS) $(INCLUDES) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $@ from: $<'
	@echo ' '
