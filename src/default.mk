.PHONY: clean all
.SUFFIXES: .cpp .d

CPP = g++
################################################################################################
# SOURCE Code directory and filenames
################################################################################################
OUTPUT_PATH = ../../bin
${shell mkdir -p ${OUTPUT_PATH} 2> /dev/null}
CURRENT_DIR = $(notdir $(PWD))

SOURCES += $(wildcard *.cpp)

LOCAL_OBJS	= $(SOURCES:.cpp=.o)
LOCAL_DEPS	= $(SOURCES:.cpp=.d)

################################################################################################
# Compile Flags
################################################################################################ 
INCLUDES += -I/usr/local/include \
			-I/usr/include \
			-I../ \

CPPFLAGS += -Wall -Wextra -Wno-unused-parameter -g -ggdb -O2 $(INCLUDES)

BOOST_LIBS = -lboost_thread \
			 -lboost_filesystem \
			 -lboost_system \
			 -lboost_date_time

LBITS = $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
LDFLAGS += -L/usr/local/lib \
		   -L/usr/lib64
else
LDFLAGS += -L/usr/local/lib \
		   -L/usr/lib
endif

LDFLAGS += -L$(OUTPUT_PATH)
		   

ALL_LIBS := $(LDFLAGS) $(LIBS) $(BOOST_LIBS)

################################################################################################
# Common Rules
################################################################################################
%.a: $(LOCAL_OBJS)
	ar rcu $@ $(LOCAL_OBJS) 

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
	-rm -f *.d
	-rm -f *.o

-include $(LOCAL_DEPS)
