



-include src/subdir.mk





CFLAGS   += `root-config --cflags`

LFLAGS    += `root-config --libs --glibs` -lboost_system -lboost_filesystem

INCLUDES  += -I"./include"

CALI_OBJS += ./build/calibration.o
ANA_OBJS += ./build/analysis.o

all: clean build/calibration build/analysis


# Link Calibration
build/calibration: $(CALI_OBJS)
	@echo 'Linking target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Linker'
	g++ -o "build/calibration" $(CALI_OBJS) $(LFLAGS)
	@echo 'Finished linking: $@ from: $<'
	@echo ' '

# Link Analysis
build/analysis: $(ANA_OBJS)
	@echo 'Linking target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Linker'
	g++ -o "build/analysis" $(ANA_OBJS) $(LFLAGS)
	@echo 'Finished linking: $@ from: $<'
	@echo ' '

# Compile Calibration
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
