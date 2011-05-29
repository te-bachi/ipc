
CC      = g++
LD      = g++
SRC     = ../src
CFLAGS  = -g -Wall

all: HSMain HSDisplay HSControl HSDataTx SensorDevices
	

clean:
	rm -f *.o *.a *.e

HSMain: HSMain.o $(SRC)/defs.h
	$(CC) -o HSMain.e HSMain.o

HSDisplay: HSDisplay.o $(SRC)/defs.h
	$(CC) -o HSDisplay.e HSDisplay.o

HSControl: HSControl.o $(SRC)/defs.h
	$(CC) -o HSControl.e HSControl.o

HSDataTx: HSDataTx.o $(SRC)/defs.h
	$(CC) -o HSDataTx.e HSDataTx.o

SensorDevices: SensorDevices.o $(SRC)/defs.h
	$(CC) -o SensorDevices.e SensorDevices.o

%.o: ../src/%.cc
	$(CC) -c -o $@ $(CFLAGS) $<

