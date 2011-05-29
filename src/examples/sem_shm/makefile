CMP=		g++
CMPFLAGS=	-g
LDFLAGS=	
EXENAME=	shm.e
LIBNAME=	

$(EXENAME):	shm.o semaphore.o shmem.o
		$(CMP) shm.o semaphore.o shmem.o $(LIBNAME) $(LDFLAGS) -o $(EXENAME)

shm.o:		shm.cc 
		$(CMP) -c $(CMPFLAGS) shm.cc

semaphore.o:	semaphore.cc semaphore.h
		$(CMP) -c $(CMPFLAGS) semaphore.cc

shmem.o:	shmem.cc shmem.h
		$(CMP) -c $(CMPFLAGS) shmem.cc

clean:
		rm -f *.o

purge:		
		rm -f *.e *.o

all:		
		make purge
		make shm.e
