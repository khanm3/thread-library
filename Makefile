CC=g++ -g -Wall -std=c++17

# List of source files for your thread library
THREAD_SOURCES=cpu.cpp types.cpp thread.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

# List of test files
TEST_SOURCES=$(sort $(wildcard test*.cpp))

# Generate the names of the test executables
TESTS=${TEST_SOURCES:.cpp=}

# Generate the runtest target name for all tests
RUNTESTS=$(subst test,runtest,${TESTS})

all: libthread.o alltests

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	./autotag.sh
	ld -r -o $@ ${THREAD_OBJS}

# Compile a test program
test%: test%.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

# Compile all test programs
alltests: ${TESTS}

# Run a test and diff check
runtest%: test% test%.out.correct
	./$< > $<.out
	diff -q $<.out $<.out.correct
	@echo

# Run a test
runtest%: test%
	./$< > $<.out
	@echo "no .out.correct file, not diffing"
	@echo

# Run all tests
runall: alltests ${RUNTESTS}

# Compile an application program
app: app.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${THREAD_OBJS} libthread.o app ${TESTS} *.out
