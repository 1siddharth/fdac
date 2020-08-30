CXX = clang++ -std=c++20 -D_GNU_SOURCE -O3

INCLUDE_ALL = -Iinclude
CFLAGS = -Wall -Wextra -fstrict-aliasing -Wstrict-aliasing=2 $(INCLUDE_ALL) \
	 -Wconversion -Wsign-conversion -Wpedantic
CFLAGS_DEBUG = -ggdb -fsanitize=address,undefined
TEST_LIB = -lgtest

BUILDDIR = build
TESTSDIR = build/tests

OBJ_CXX = $(CXX) $(INCLUDE_ALL) $(CFLAGS) -c
BIN_CXX = $(CXX) $(INCLUDE_ALL) $(CFLAGS)

#FILE_LIST

default: setup fdcap

setup:
	mkdir -p build
	mkdir -p build/tests

#fdcap: setup
#	$(BIN_CXX) $(CFLAGS_DEBUG) -o build/kbot $(FILE_LIST)

tests: setup
	$(BIN_CXX) $(CFLAGS_DEBUG) $(TEST_LIB) -o $(TESTSDIR)/test_class_fdcap src/tests/test_class_fdcap.cc

all_tests_run: setup tests
	./$(TESTSDIR)/test_class_fdcap

clean:
	rm -rf build/
