make: server.out client.out

protocol.o: protocol.c
	gcc -c protocol.c

server.out: server.o protocol.o
	gcc server.o protocol.o -o server.out

server.o: server.c
	gcc -c server.c

client.out: client.o protocol.o
	gcc client.o protocol.o -o client.out

client.o: client.c
	gcc -c client.c

clean:
	rm -rf *.o *.out

start_server: server.out
	./server.out $(port)

start_client: client.out
	./client.out $(port) $(serv_addr) $(serv_port)