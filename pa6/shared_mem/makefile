# makefile

all: dataserver client client_tester

KSemaphore.o: KSemaphore.h KSemaphore.cpp
	g++ -std=c++11 -c -g KSemaphore.cpp

reqchannel.o: reqchannel.h reqchannel.cpp KSemaphore.o
	g++ -std=c++11 -c -g reqchannel.cpp KSemaphore.o

bounded_buffer.o: bounded_buffer.h bounded_buffer.cpp
	g++ -std=c++11 -c -g bounded_buffer.cpp

client_tester.o: client_tester.cpp
	g++ -std=c++11 -c -g client_tester.cpp

dataserver: dataserver.cpp reqchannel.o KSemaphore.o
	g++ -std=c++11 -g -o dataserver dataserver.cpp reqchannel.o KSemaphore.o -lpthread -lrt

client: client.cpp reqchannel.o bounded_buffer.o KSemaphore.o
	g++ -std=c++11 -g -o client client.cpp reqchannel.o bounded_buffer.o KSemaphore.o -lpthread -lrt
	
client_tester: client_tester.o
	g++ -std=c++11 -g -o client_tester client_tester.o -lpthread

clean:
	rm -rf *.o fifo* dataserver client
	ipcrm -a