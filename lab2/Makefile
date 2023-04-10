make: threadReader.out

threadReader.out: main.o reader.o
	gcc main.o reader.o

main.o: main.c
	gcc -c main.c

reader.o: reader.c
	gcc -c reader.c

clean:
	rm -rf *.o *.out

start: threadReader.out
	./threadReader.out