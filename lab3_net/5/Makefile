make: server.out client.out

server.out: server.c
	gcc server.c -o server.out

client.out: client.c
	gcc client.c -o client.out

clean:
	rm -rf *.o *.out

start_server: server.out
	./server.out $(port)

start_client: client.out
	./client.out $(port) $(serv_addr) $(serv_port)