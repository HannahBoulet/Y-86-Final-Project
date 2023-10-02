#lab5 makefile (complete)
BIN = ../bin

yess:
	(cd src && make $(BIN)/yess)
	(cd bin && ./run.sh)

runlab5:
	(cd unit && make runlab5)

runlab4:
	(cd unit && make runlab4)

clean:
	(cd bin && rm -f lab4 & rm -f lab5 & rm -f yess)
	(cd obj && rm -f *.o)
