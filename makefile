



-include src/subdir.mk





CFLAGS   += `root-config --cflags`

LFLAGS    += `root-config --libs --glibs` -lboost_system -lboost_filesystem

INCLUDES  += -I"./include"

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
