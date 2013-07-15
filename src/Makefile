.PHONY: all clean net rpc utils

OUTPUT_PATH = ../bin

all: net rpc utils

net:
	${MAKE} -C net

utils:
	${MAKE} -C utils

rpc: net
	${MAKE} -C rpc

clean:
	${MAKE} -C net clean
	${MAKE} -C rpc clean
	${MAKE} -C utils clean
