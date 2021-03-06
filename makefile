-include Scripts/scripts.mk
-include src/subdir.mk


ifeq ($(shell uname -s),Linux)
	#CFLAGS += -fopenmp `root-config --cflags` -I/remote/pcilc3/software/gperftools/include -g -rdynamic
	CFLAGS += -std=c++11 -fopenmp `root-config --cflags` -c -fmessage-length=0
	# -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
	LFLAGS += -fopenmp `root-config --libs --glibs` -lboost_system -lboost_filesystem -lboost_program_options -lgsl -lgslcblas
	#-lgsl -lgslcblas
	# -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
	CC = g++
endif
ifeq ($(shell uname -s),Darwin)
	#CFLAGS += -stdlib=libstdc++ `root-config --cflags` -I/opt/boost-1.63.0/include
	CFLAGS += `root-config --cflags`  -c -fmessage-length=0
	#CFLAGS += -pthread -std=c++11 -m64 -I/opt/root/root/include -I/opt/boost-1.63.0/include/
	# -pthread -std=c++11 -m64 -I/opt/root/root/include
	# CFLAGS += -fopenmp -std=c++1y -pthread -m64 -I/opt/root/root/include -g
	# -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
#	LFLAGS += `root-config --libs --glibs` -L/opt/boost/boost/bin.v2/libs/system/build/darwin-4.2.1/release/threading-multi -lboost_system -L/opt/boost/boost/bin.v2/libs/filesystem/build/darwin-4.2.1/release/threading-multi -lboost_filesystem
#	LFLAGS += `root-config --libs --glibs` -L/opt/boost-1.63.0/lib -lboost_system-mt -lboost_filesystem-mt
  	LFLAGS += `root-config --libs --glibs` -lboost_system -lboost_filesystem
	# -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
#	CC = g++-4.9
#	CC = /usr/local/opt/llvm/bin/clang++
	#CC = /usr/local/opt/llvm/bin/clang-3.9
 	CC = g++
endif

INCLUDES  += -I"./include"
INCLUDES  += -I"./Scripts"
CALI_OBJS += ./build/calibration.o
ANA_OBJS += ./build/analysis.o
TEST_OBJS += ./build/test.o

#all: monkey clean build/calibration build/analysis docs
all: debug

debug: CFLAGS += -pg -g -O0 -Wall -fno-inline-functions
#debug: monkey clean build/calibration build/analysis
debug: monkey clean build/calibration

release: CFLAGS += -O3 -g -Wall
#release: CFLAGS += -pg -Wall
#release: LFLAGS += -pg
# release: monkey clean build/calibration build/analysis
release: monkey clean build/calibration

analysis: CFLAGS += -g -O3 -Wall -fno-inline-functions
analysis: monkey clean build/analysis

#test: CFLAGS += -pg -g -O0 -Wall -fno-inline-functions
#test: CFLAGS += -pg -g -O0 -Wall
test: CFLAGS += -pg -g -O0 -Wall -fno-inline-functions
test: LFLAGS += -pg
test: monkey clean build/test

# Link Calibration
build/calibration: $(CALI_OBJS)
	@echo 'Linking target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Linker'
	$(CC) -o "build/calibration" $(CALI_OBJS) $(LFLAGS)
	@echo 'Finished linking: $@ from: $<'
	@echo ' '

# Link Analysis
build/analysis: $(ANA_OBJS)
	@echo 'Linking target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Linker'
	$(CC) -o "build/analysis" $(ANA_OBJS) $(LFLAGS)
	@echo 'Finished linking: $@ from: $<'
	@echo ' '

# Link Analysis
build/test: $(TEST_OBJS)
	@echo 'Linking target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Linker'
	$(CC) -o "build/test" $(TEST_OBJS) $(LFLAGS)
	@echo 'Finished linking: $@ from: $<'
	@echo ' '

# Compile Calibration
build/%.o: ./%.cxx
	@echo 'Building target: $@ from: $<'
	@mkdir -p './build/src'
	@echo 'Invoking: GCC C++ Compiler'
	$(CC) $(CFLAGS) $(INCLUDES) -o "$@" "$<"
	#-E"@:%.o=%.d" > main.i
	@echo 'Finished building: $@ from: $<'
	@echo ' '

docs:
	doxygen doxygen_config

clean:
	@echo 'Cleaning up!'
	rm -rf './build'

monkey:
	@echo ┈┈┈┈┈┈┈┈┈┈┈?????????????
	@echo ┈┈╱▔▔▔▔▔╲┈┈┈??????????
	@echo ┈╱┈┈╱▔╲╲╲▏┈┈┈?????┈
	@echo ╱┈┈╱━╱▔▔▔▔▔╲━╮┈┈
	@echo ▏┈▕┃▕╱▔╲╱▔╲▕╮┃┈┈
	@echo ▏┈▕╰━▏▊▕▕▋▕▕━╯┈┈
	@echo ╲┈┈╲╱▔╭╮▔▔┳╲╲┈┈┈
	@echo ┈╲┈┈▏╭━━━━╯▕▕┈┈┈
	@echo ┈┈╲┈╲▂▂▂▂▂▂╱╱┈┈┈
	@echo ┈┈┈┈▏┊┈┈┈┈┊┈┈┈╲┈
	@echo ┈┈┈┈▏┊┈┈┈┈┊▕╲┈┈╲
	@echo ┈╱▔╲▏┊┈┈┈┈┊▕╱▔╲▕
	@echo ┈▏┈┈┈╰┈┈┈┈╯┈┈┈▕▕
	@echo ┈╲┈┈┈╲┈┈┈┈╱┈┈┈╱┈╲
	@echo ┈┈╲┈┈▕▔▔▔▔▏┈┈╱╲╲╲▏
	@echo ┈╱▔┈┈▕┈┈┈┈▏┈┈▔╲▔▔
	@echo ┈╲▂▂▂╱┈┈┈┈╲▂▂▂╱┈
	@echo $(CC)
