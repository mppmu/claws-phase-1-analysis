

ANA_OBJS += ./build/Scripts/plot_waveform.o


# CALI_CPP_DEPS += ./build/src/EVENT.d
# CALI_CPP_DEPS += ./build/src/Gloablsettings.d

# Each subdirectory must supply rules for building sources it contributes

./build/Scripts/%.o: ./Scripts/%.cxx
	@echo 'Building target: $@ from: $<'
	@mkdir -p './build/Scripts'
	@echo 'Invoking: GCC C++ Compiler'
	$(CC) $(CFLAGS) $(INCLUDES) -o "$@" "$<"
	@echo 'Finished building: $@ from: $<'
	@echo ' '



	#$(CC) $(CFLAGS) $(INCLUDES)  -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
