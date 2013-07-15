.PHONY: clean all
.SUFFIXES: .cpp .d

CPP = g++
################################################################################################
# SOURCE Code directory and filenames
################################################################################################
OUTPUT_PATH = ../../bin/test
${shell mkdir -p ${OUTPUT_PATH} 2> /dev/null}
CURRENT_DIR = $(notdir $(PWD))

SOURCES += $(wildcard *.cpp)

LOCAL_OBJS	= $(SOURCES:.cpp=.o)
LOCAL_DEPS	= $(SOURCES:.cpp=.d)

################################################################################################
# Compile Flags
################################################################################################
GTEST_DIR = ../gtest_src/

INCLUDES += -I/usr/local/include \
			-I/usr/include \
			-I../ \
			-I../../src/ \
			-I${GTEST_DIR}/include

CPPFLAGS += -Wall -Wextra -Wno-unused-parameter -g $(INCLUDES)

BOOST_LIBS = -lboost_thread \
			 -lboost_filesystem \
			 -lboost_system \
			 -lboost_date_time \

LIBS += -lgtest \
		-lgtest-main
		
GOOGLE_LIBS = -lgflags \
			  -lglog

GCC_LIBS = -lpthread

GCC_VERSION := $(shell g++ -dumpversion)
LBITS = $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
LDFLAGS += -L/usr/local/lib \
		   -L/usr/lib64
else
LDFLAGS += -L/usr/local/lib \
		   -L/usr/lib
endif
LDFLAGS += -L$(OUTPUT_PATH) \
		   -L$(OUTPUT_PATH)/../

ALL_LIBS = $(LDFLAGS) $(LIBS) ${GCC_LIBS} $(BOOST_LIBS) ${GOOGLE_LIBS}

################################################################################################
# Common Rules
################################################################################################
%.a: $(LOCAL_OBJS)
	ar rcu $@ $^

%.d:%.cpp
	@echo create $@
	@set -e; rm -f $@; \
	$(CPP) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

################################################################################################
# Real Target
################################################################################################
all: $(TARGET)


clean:
	-rm -f $(LOCAL_DEPS)
	-rm -f $(LOCAL_OBJS)
	-rm -f $(TARGET)
	-rm -f *.d.*

-include $(LOCAL_DEPS)