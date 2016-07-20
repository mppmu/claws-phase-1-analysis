



-include src/subdir.mk





CFLAGS   += `root-config --cflags`

LFLAGS    += `root-config --libs --glibs`
LFLAGS   +=

INCLUDES  += -I"./include"
LIBS 		+= -L/remote/pcilc3/software/root-install/root-v6.04.02/lib/root
CPP_SRCS += \./main.cxx

OBJS += ./build/main.o

all: build/claws

build/claws: $(OBJS)
	@echo 'Building target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Linker'
	g++ -o "build/claws" $(OBJS) $(LFLAGS)
	@echo 'Finished linking: $@ from: $<'
	@echo ' '


# Tool invocations
build/%.o: ./%.cxx
	@echo 'Building target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Compiler'
	g++ $(CFLAGS) $(INCLUDES) -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $@ from: $<'
	@echo ' '


clean:
	@echo 'Cleaning up!'
	rm -rf './build'


# compile: $(CPP_SRCS)
	# g++ -c $(INCLUDES)

	# claws: $(OBJS) $(USER_OBJS)
	# 	@echo 'Building target: $@'
	# 	@echo 'Invoking: GCC C++ Linker'
	# 	g++ -o "claws" $(OBJS) $(USER_OBJS) $(LIBS)  -L/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -Wl,-rpath,/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lm -ldl -rdynamic -L/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lGui -pthread -std=c++11 -Wno-deprecated-declarations -m64 -I/remote/pcilc3/software/root-install/root-v6.04.02/include/root -m64 -pthread -Wl,-rpath,/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lm -ldl -rdynamic -pthread -std=c++11 -Wno-deprecated-declarations -m64
	# 	@echo 'Finished building target: $@'
	# 	@echo ' '

# 	# Add inputs and outputs from these tool invocations to the build variables
# 	CPP_SRCS += \
# 	../src/claws.cpp
#
# 	OBJS += \
# 	./src/claws.o
#
# 	CPP_DEPS += \
# 	./src/claws.d
#
#
# # Each subdirectory must supply rules for building sources it contributes
# src/%.o: ../src/%.cpp
# 	@echo 'Building file: $<'
# 	@echo 'Invoking: GCC C++ Compiler'
# 	g++ -std=c++11 -I"/remote/pcilc3/software/root-install/root-v6.04.02" -I"/remote/pcilc3/software/root-install/root-v6.04.02/include/root" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
# 	@echo 'Finished building: $<'
# 	@echo ' '


# -include ../makefile.init
#
# RM := rm -rf
#
# # All of the sources participating in the build are defined here
# -include sources.mk
# -include src/subdir.mk
# -include subdir.mk
# -include objects.mk
#
# ifneq ($(MAKECMDGOALS),clean)
# ifneq ($(strip $(C++_DEPS)),)
# -include $(C++_DEPS)
# endif
# ifneq ($(strip $(C_DEPS)),)
# -include $(C_DEPS)
# endif
# ifneq ($(strip $(CC_DEPS)),)
# -include $(CC_DEPS)
# endif
# ifneq ($(strip $(CPP_DEPS)),)
# -include $(CPP_DEPS)
# endif
# ifneq ($(strip $(CXX_DEPS)),)
# -include $(CXX_DEPS)
# endif
# ifneq ($(strip $(C_UPPER_DEPS)),)
# -include $(C_UPPER_DEPS)
# endif
# endif
#
# -include ../makefile.defs
#
# # Add inputs and outputs from these tool invocations to the build variables
#
# # All Target
# all: claws
#
# # Tool invocations
# claws: $(OBJS) $(USER_OBJS)
# 	@echo 'Building target: $@'
# 	@echo 'Invoking: GCC C++ Linker'
# 	g++ -o "claws" $(OBJS) $(USER_OBJS) $(LIBS)  -L/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -Wl,-rpath,/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lm -ldl -rdynamic -L/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lGui -pthread -std=c++11 -Wno-deprecated-declarations -m64 -I/remote/pcilc3/software/root-install/root-v6.04.02/include/root -m64 -pthread -Wl,-rpath,/remote/pcilc3/software/root-install/root-v6.04.02/lib/root -lm -ldl -rdynamic -pthread -std=c++11 -Wno-deprecated-declarations -m64
# 	@echo 'Finished building target: $@'
# 	@echo ' '
#
# # Other Targets
# clean:
# 	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) claws
# 	-@echo ' '
#
# .PHONY: all clean dependents
# .SECONDARY:
#
# -include ../makefile.targets
