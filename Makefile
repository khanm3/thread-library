CC=g++ -g -Wall -std=c++17

# List of source files for your thread library
THREAD_SOURCES=cpu.cpp types.cpp thread.cpp mutex.cpp mutexImpl.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

# Directory containing test source files
TEST_DIR=tests

# List of test files
TEST_SOURCES=$(sort $(wildcard ${TEST_DIR}/test*.cpp))

# List of test files without the base directory
TEST_SOURCES_NO_DIR=$(subst ${TEST_DIR}/,,${TEST_SOURCES})

# Generate the names of the test executables
TESTS=$(subst .cpp,,${TEST_SOURCES_NO_DIR})

# Generate the runtest target name for all tests
RUNTESTS=$(subst test,runtest,${TESTS})

all: libthread.o alltests

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	./autotag.sh
	ld -r -o $@ ${THREAD_OBJS}

# Compile a test program
# Generates the executable in the base project directory
test%: ${TEST_DIR}/test%.cpp libthread.o libcpu.o
	${CC} -o $@ $^ -ldl -pthread -I.

# Compile all test programs
alltests: ${TESTS}

# Run a test and diff check
# Generates the output file in the test directory
runtest%: test% ${TEST_DIR}/test%.out.correct
	./$< > ${TEST_DIR}/$<.out
	diff -q ${TEST_DIR}/$<.out ${TEST_DIR}/$<.out.correct
	@echo

# Run a test
# Generates the output file in the test directory
runtest%: test%
	./$< > ${TEST_DIR}/$<.out
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
	rm -f ${THREAD_OBJS} libthread.o app ${TESTS} ${TEST_DIR}/*.out
