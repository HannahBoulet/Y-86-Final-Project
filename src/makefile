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
OBJS := $(OBJD)/Memory.o $(OBJD)/RegisterFile.o $(OBJD)/Tools.o $(OBJD)/ConditionCodes.o $(OBJD)/Loader.o $(OBJD)/String.o $(OBJD)/yess.o $(OBJD)/CPU.o $(OBJD)/D.o $(OBJD)/E.o\
	$(OBJD)/F.o $(OBJD)/M.o $(OBJD)/W.o $(OBJD)/FetchStage.o $(OBJD)/DecodeStage.o\
	$(OBJD)/ExecuteStage.o $(OBJD)/MemoryStage.o $(OBJD)/WritebackStage.o $(OBJD)/PipeRegArray.o\
	$(OBJD)/PipeReg.o $(OBJD)/PipeRegField.o $(OBJD)/StageArray.o $(OBJD)/Stage.o

 # 2) need a target (with a name that doesn't match a file; for example, all:)
#    that is dependent upon the .o files. With this target, you can type:
#    make all
#    and all of the .C files will be compiled
yess: $(OBJS)
	$(CC) $(OBJS) -o $(BIN)/yess

help:
	@echo "make all to compile for lab4."
	@echo "Run make all to compile all the .C files"
	@echo "Run make yess to run phony"

all:$(OBJS)

# 3) need a .o target for each .C
#    Remember each of these is dependent upon the .h files included by the .C

$(OBJD)/ConditionCodes.o: $(INC)/ConditionCodes.h $(INC)/Tools.h

$(OBJD)/Memory.o: $(INC)/Memory.h $(INC)/Tools.h 

$(OBJD)/RegisterFile.o: $(INC)/RegisterFile.h

$(OBJD)/Tools.o: $(INC)/Tools.h

$(OBJD)/String.o: $(INC)/String.h

$(OBJD)/yess.o: $(INC)/Memory.h $(INC)/Loader.h

$(OBJD)/Loader.o: $(INC)/Memory.h $(INC)/String.h $(INC)/Loader.h

$(OBJD)/CPU.o: $(INC)/CPU.h $(INC)/PipeRegArray.h $(INC)/StageArray.h $(INC)/Memory.h\
 $(INC)/ConditionCodes.h $(INC)/RegisterFile.h

$(OBJD)/D.o: $(INC)/D.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/E.o: $(INC)/E.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/F.o: $(INC)/F.h $(INC)/PipeRegField.h

$(OBJD)/M.o: $(INC)/M.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/W.o: $(INC)/W.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/FetchStage.o: $(INC)/PipeRegArray.h $(INC)/PipeReg.h $(INC)/Memory.h \
 $(INC)/FetchStage.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/Status.h \
 $(INC)/F.h $(INC)/D.h $(INC)/M.h $(INC)/W.h $(INC)/Tools.h $(INC)/Stage.h 

$(OBJD)/DecodeStage.o: $(INC)/DecodeStage.h $(INC)/PipeRegArray.h $(INC)/E.h  \
 $(INC)/D.h $(INC)/M.h $(INC)/W.h $(INC)/Instruction.h $(INC)/Stage.h $(INC)/RegisterFile.h \
 $(INC)/ConditionCodes.h $(INC)/PipeRegField.h  $(INC)/PipeReg.h

$(OBJD)/ExecuteStage.o: $(INC)/ExecuteStage.h $(INC)/PipeRegArray.h $(INC)/Instruction.h \
 $(INC)/ConditionCodes.h $(INC)/Stage.h $(INC)/Tools.h $(INC)/E.h $(INC)/M.h $(INC)/Status.h \
 $(INC)/W.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h  $(INC)/PipeReg.h

$(OBJD)/MemoryStage.o: $(INC)/MemoryStage.h $(INC)/PipeRegArray.h $(INC)/RegisterFile.h \
 $(INC)/ConditionCodes.h $(INC)/Instruction.h $(INC)/Stage.h $(INC)/W.h $(INC)/M.h \
 $(INC)/Status.h $(INC)/PipeReg.h

$(OBJD)/WritebackStage.o: $(INC)/WritebackStage.h $(INC)/PipeRegArray.h $(INC)/Memory.h \
 $(INC)/W.h $(INC)/ConditionCodes.h $(INC)/Instruction.h $(INC)/PipeReg.h \
 $(INC)/RegisterFile.h $(INC)/Stage.h $(INC)/Status.h 

$(OBJD)/PipeRegArray.o: $(INC)/PipeReg.h $(INC)/PipeRegArray.h $(INC)/F.h $(INC)/D.h \
 $(INC)/E.h $(INC)/M.h $(INC)/W.h

$(OBJD)/PipeReg.o: $(INC)/PipeReg.h

$(OBJD)/PipeRegField.o: $(INC)/PipeRegField.h

$(OBJD)/StageArray.o: $(INC)/StageArray.h $(INC)/Stage.h $(INC)/FetchStage.h \
 $(INC)/DecodeStage.h $(INC)/ExecuteStage.h $(INC)/MemoryStage.h $(INC)/WritebackStage.h

$(OBJD)/Stage.o: $(INC)/Memory.h $(INC)/RegisterFile.h $(INC)/ConditionCodes.h \
 $(INC)/Stage.h

# 4) need a clean target to delete the .o files corresponding to
#    the .C files in this directory from the ../obj directory
clean:
	rm -f $(OBJD)/*
