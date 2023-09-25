#lab4 makefile (complete)

#unit tests
UNIT = unit
#object directory
OBJ = obj
#source directory
SRC = src
#bin directory
BIN = bin
#executables
EXE = lab4

runlab4:
	(cd $(UNIT) && make runlab4)

clean:
	(cd $(OBJ) && rm -f *.o)
	(cd $(BIN) && rm -f $(EXE))
