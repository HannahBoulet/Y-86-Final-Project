#unit makefile (complete)

CC = g++
INC = ../inc
BIN = ../bin
SRC = ../src
OBJD = ../obj
CFLAGS = -g -c -Wall -std=c++11 -I $(INC)

#lab4 object files whose .C is in the SRC directory
LAB4SRCOBJS = $(OBJD)/Memory.o $(OBJD)/Tools.o $(OBJD)/RegisterFile.o $(OBJD)/ConditionCodes.o 

LAB4UNITOBJS = $(OBJD)/lab4.o $(OBJD)/MemoryTester.o $(OBJD)/RegisterFileTester.o \
$(OBJD)/ConditionCodesTester.o

$(OBJD)/%.o: %.C
	@mkdir -p $(OBJD)
	$(CC) $(CFLAGS) $< -o $@

help:
	@echo "make runlab4: compile/link all lab4 code and run"
	@echo "make compile: compile code in this directory"
	@echo "targets: runlab4 compile help $(BIN)/lab4 $(OBJS) clean"

runlab4:
	make $(LAB4UNITOBJS)
	(cd $(SRC) && make $(LAB4SRCOBJS))
	@mkdir -p $(BIN)
	$(CC) $(LAB4UNITOBJS) $(LAB4SRCOBJS) -o $(BIN)/lab4
	$(BIN)/lab4

compile: $(LAB4UNITOBJS)

$(OBJD)/lab4.o: $(INC)/MemoryTester.h $(INC)/Memory.h

$(OBJD)/MemoryTester.o: $(INC)/MemoryTester.h $(INC)/Memory.h

$(OBJD)/RegisterFileTester.o: $(INC)/RegisterFile.h $(INC)/RegisterFileTester.h

$(OBJD)/ConditionCodesTester.o: $(INC)/ConditionCodes.h $(INC)/ConditionCodesTester.h

clean:
	rm -f ../bin/lab4
	rm -f $(OBJS)

