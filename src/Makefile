
CC      = g++
LD      = g++

CFLAGS  = -g -Wall

all: HSMain HSDisplay HSControl HSDataTx SensorDevices
	mkdir -p ../bin
	cp *.e ../bin

clean:
	rm -f *.o *.a *.e

HSMain: HSMain.o defs.h
	$(CC) -o HSMain.e HSMain.o

HSDisplay: HSDisplay.o defs.h
	$(CC) -o HSDisplay.e HSDisplay.o

HSControl: HSControl.o defs.h
	$(CC) -o HSControl.e HSControl.o

HSDataTx: HSDataTx.o defs.h
	$(CC) -o HSDataTx.e HSDataTx.o

SensorDevices: SensorDevices.o defs.h
	$(CC) -o SensorDevices.e SensorDevices.o

%.o: %.cc
	$(CC) -c -o $@ $(CFLAGS) $<
