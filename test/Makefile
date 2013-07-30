###############################################################################
# Author: Dian FAN
# Date:   2010/06/22
# Description:
#   ....
# Usage:
#   ....
###############################################################################
.PHONY: clean 

OUTPUT_PATH = ../bin/test

all: gtest gtest-main utils rpc

gtest:
	${MAKE} -C gtest_src

gtest-main: gtest
	${MAKE} -C core

utils: gtest gtest-main
	${MAKE} -C utils

rpc: gtest gtest-main
	${MAKE} -C rpc
	
clean:
	${MAKE} -C gtest_src clean
	${MAKE} -C core clean
	${MAKE} -C utils clean
	${MAKE} -C rpc clean