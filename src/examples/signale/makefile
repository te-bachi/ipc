CMP=		g++
CMPFLAGS=	-g
EXENAM1=	signal_p.e
EXENAM2=	signal_c.e
LDFLAGS=	
LIBNAME=	

compile:	$(EXENAM1) $(EXENAM2)

$(EXENAM1):	signal_p.o
		$(CMP) signal_p.o $(LIBNAME) $(LDFLAGS) -o signal_p.e

$(EXENAM2):	signal_c.o
		$(CMP) signal_c.o $(LIBNAME) $(LDFLAGS) -o signal_c.e

signal_p.o:	signal_p.cc
		$(CMP) -c $(CMPFLAGS) signal_p.cc

signal_c.o:	signal_c.cc
		$(CMP) -c $(CMPFLAGS) signal_c.cc

ra:		
		touch *.cc

clean:
		rm *.o

purge:
		rm *.e
		rm *.o
