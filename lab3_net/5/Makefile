make: server.out client.out

protocol.o: protocol.c
	gcc -c protocol.c

marine.o: marine.c
	gcc -c marine.c

server.out: server.o protocol.o marine.o
	gcc server.o protocol.o marine.o -o server.out

server.o: server.c
	gcc -c server.c

client.out: client.o protocol.o  marine.o
	gcc client.o protocol.o marine.o -o client.out

client.o: client.c
	gcc -c client.c

clean:
	rm -rf *.o *.out

start_server: server.out
	./server.out $(port)

start_client: client.out
	./client.out $(port) $(serv_addr) $(serv_port)