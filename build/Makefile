
CC      = g++
LD      = g++

CFLAGS  = -g -Wall
LDFLAGS = -pthread

all: HSMain.e HSDisplay.e HSControl.e HSDataTx.e SensorDevices.e
	

clean:
	rm -f *.o *.a *.e

HSMain.e: HSMain.o Utils.o ../src/Utils.h ../src/defs.h
	$(CC) $(LDFLAGS) -o HSMain.e HSMain.o Utils.o

HSDisplay.e: HSDisplay.o Utils.o ../src/Utils.h ../src/defs.h
	$(CC) -o HSDisplay.e HSDisplay.o Utils.o

HSControl.e: HSControl.o Utils.o ../src/Utils.h ../src/defs.h
	$(CC) -o HSControl.e HSControl.o Utils.o

HSDataTx.e: HSDataTx.o Utils.o ../src/Utils.h ../src/defs.h
	$(CC) -o HSDataTx.e HSDataTx.o Utils.o

SensorDevices.e: SensorDevices.o ../src/defs.h
	$(CC) -o SensorDevices.e SensorDevices.o

%.o: ../src/%.cc
	$(CC) -c -o $@ $(CFLAGS) $<

