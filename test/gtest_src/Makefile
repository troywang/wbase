.PHONY: clean

CPP = g++
OUTPUT_PATH = ../../bin/test

${shell mkdir -p ${OUTPUT_PATH} 2> /dev/null}

#
# Compile gtest library
#

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = ./

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h $(GTEST_DIR)/include/gtest/internal/*.h

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
GTEST_LIB   = ${OUTPUT_PATH}/libgtest.a

${GTEST_LIB} : ${GTEST_DIR}/gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

${GTEST_DIR}/gtest-all.o : $(GTEST_SRCS_)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GTEST_DIR)/include -c $(GTEST_DIR)/src/gtest-all.cc -o $@

clean:
	-rm -f ${GTEST_LIB}
	-rm -f ${GTEST_DIR}/gtest-all.o

