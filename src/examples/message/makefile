CMP=		g++
CMPFLAGS=	-g
LDFLAGS=	
EXENAME=	msg_bsp.e
LIBNAME=	

$(EXENAME):	msg_bsp.o 
		$(CMP) msg_bsp.o $(LIBNAME) $(LDFLAGS) -o $(EXENAME)

msg_bsp.o:	msg_bsp.cc
		$(CMP) -c $(CMPFLAGS) msg_bsp.cc
ra:		
		touch *.cc
		make msg_bsp.e

clean:
		rm *.o

purge:
		rm *.e
		rm *.o
