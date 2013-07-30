TARGET    =$(OUTPUT_PATH)/unittest_rpc

INCLUDES += -I../../src/utils

DEP_OBJS   = ${wildcard ../../src/utils/*.o}
include ../default.mk

${TARGET}:${LOCAL_OBJS} ${DEP_OBJS}
	@echo ${DEP_OBJS}
	${CPP} -o $@ $^ $(ALL_LIBS)
