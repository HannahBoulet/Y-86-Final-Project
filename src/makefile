#makefile to compile the .C files in this directory
#and write the .o output to the ../obj directory
CC = g++
INC = ../inc
BIN = ../bin
OBJD = ../obj
CFLAGS = -g -c -Wall -std=c++11 -I $(INC)

#rule to match .C with object in another directory
$(OBJD)/%.o: %.C 
	@mkdir -p $(OBJD)
	$(CC) $(CFLAGS) $< -o $(OBJD)/$@

# 1) need a macro for the .o files corresponding to the .C 
#    files in this directory (note they will be stored in the ../obj
#    directory: OBJS = ....
OBJS := $(OBJD)/Memory.o $(OBJD)/RegisterFile.o $(OBJD)/Tools.o $(OBJD)/ConditionCodes.o $(OBJD)/Loader.o $(OBJD)/String.o $(OBJD)/yess.o
 # 2) need a target (with a name that doesn't match a file; for example, all:)
#    that is dependent upon the .o files. With this target, you can type:
#    make all
#    and all of the .C files will be compiled
help:
	@echo "make all to compile for lab4."
	@echo "Run make all to compile all the .C files"
	@echp "Run make yess to run phony"

all:$(OBJS)

yess: $(OBJS)
	$(CC) $(OBJS) -o $(BIN)/yess
# 3) need a .o target for each .C
#    Remember each of these is dependent upon the .h files included by the .C

$(OBJD)/ConditionCodes.o: $(INC)/ConditionCodes.h $(INC)/Tools.h

$(OBJD)/Memory.o: $(INC)/Memory.h

$(OBJD)/RegisterFile.o: $(INC)/RegisterFile.h

$(OBJD)/Tools.o: $(INC)/Tools.h

$(OBJD)/String.o: $(INC)/String.h

$(OBJD)/yess.o: $(INC)/Memory.h $(INC)/Loader.h

$(OBJD)/Loader.o: $(INC)/Memory.h $(INC)/String.h $(INC)/Loader.h


# 4) need a clean target to delete the .o files corresponding to
#    the .C files in this directory from the ../obj directory
clean:
	rm -f $(OBJD)/*
