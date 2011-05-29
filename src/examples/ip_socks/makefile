CMP=		g++
CMPFLAGS=	-g
LDFLAGS=	
EXENAMES=	ip_server.e
EXENAMEC=	ip_client.e
LIBNAME=	

compile:	$(EXENAMES) $(EXENAMEC)

$(EXENAMES):	ip_server.o
		$(CMP) ip_server.o $(LIBNAME) $(LDFLAGS) -o $(EXENAMES)

$(EXENAMEC):	ip_client.o
		$(CMP) ip_client.o $(LIBNAME) $(LDFLAGS) -o $(EXENAMEC)

.cc.o:		lsocks.h
		$(CMP) -c $(CMPFLAGS) $<

clean:
		rm *.o
purge:
		rm -f *.e
		rm -f *.o

all:		
		make purge
		make
