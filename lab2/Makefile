make: threadReader.out

threadReader.out: main.o reader.o
	gcc -pthread main.o reader.o -o threadReader.out

main.o: main.c
	gcc -c main.c

reader.o: reader.c
	gcc -c reader.c

clean:
	rm -rf *.o *.out

start: threadReader.out
	./threadReader.out $(file1) $(file2)
