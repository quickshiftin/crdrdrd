outputName := crdrdrd
cppExtension := .c
CPPFLAGS += -O2 -Wall -g -I../mxnLogger/includes -I./includes $(shell pkg-config --cflags glib-2.0)
LDFLAGS += -L/home/toad/progs/usbreader/buildDhcpListener/trunk/mxnLogger/ -lmxnLogger -lsqlite3 -lusb -lc -lm -lnsl -lc $(shell pkg-config --libs glib-2.0)

default : incremental

CXXFLAGS += -MMD
CXX := gcc

allCppMakes := ${patsubst %${strip ${cppExtension}},%.d,${wildcard *${cppExtension}}}
allCppObjects = ${allCppMakes:.d=.o}

${allCppMakes} : %.d : %${cppExtension}
	${CXX} ${CPPFLAGS} -MM $< > $@

ifneq (${MAKECMDGOALS},clean)
include ${allCppMakes} # include the generated make files, which make the object files
endif

.PHONY : clean incremental full


run : $(outputName)
	LD_LIBRARY_PATH="$LD_LIBRARY_PATH:../mxnLogger" ./$(outputName)
debug : $(outputName)
	rm -f vlog.*
#	LD_LIBRARY_PATH="$LD_LIBRARY_PATH:../mxnLogger" valgrind --log-file=vlog --tool=memcheck ./$(outputName)
	LD_LIBRARY_PATH="$LD_LIBRARY_PATH:../mxnLogger" valgrind --log-file=vlog --leak-check=full --show-reachable=yes --tool=memcheck ./$(outputName)

full : clean incremental

incremental : ${outputName}

clean :
	rm -f *.o *~ *.d $(outputName)

install:
	mkdir -p /mxn/daemon/crdrdrd
	cp $(outputName) /mxn/daemon/crdrdrd/crdrdrd
	cp external/etc/conf.d/crdrdrd /etc/conf.d/crdrdrd
	cp external/etc/init.d/crdrdrd /etc/init.d/crdrdrd


${outputName} : ${allCppObjects}
	${CXX}  ${LDFLAGS} -o $@ $^
