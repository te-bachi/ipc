
CC      = g++
LD      = g++

SOURCES = Utils.cc \
          Debug.cc \
          Socket.cc \
          ServerSocket.cc \
          Semaphore.cc \
          SharedMemory.cc \
          MessageQueue.cc \
          Exception.cc
          
OBJECTS = $(SOURCES:%.cc=%.o)
CFLAGS  = -g -Wall
LDFLAGS = -pthread

all: HSMain.e HSDisplay.e HSControl.e HSDataTx.e SensorDevices.e SensorTest.e
	

clean:
	rm -f *.o *.a *.e

HSMain.e: $(OBJECTS) HSMain.o
	$(CC) $(LDFLAGS) -o HSMain.e HSMain.o $(OBJECTS)

HSDisplay.e: $(OBJECTS) HSDisplay.o
	$(CC) -o HSDisplay.e HSDisplay.o $(OBJECTS)

HSControl.e: $(OBJECTS) HSControl.o
	$(CC) -o HSControl.e HSControl.o $(OBJECTS)

HSDataTx.e: $(OBJECTS) HSDataTx.o
	$(CC) -o HSDataTx.e HSDataTx.o $(OBJECTS)

SensorDevices.e: SensorDevices.o
	$(CC) -o SensorDevices.e SensorDevices.o

SensorTest.e: SensorTest.o
	$(CC) -o SensorTest.e SensorTest.o

%.o: ../src/%.cc
	$(CC) $(CFLAGS) -c -o $@ $<

