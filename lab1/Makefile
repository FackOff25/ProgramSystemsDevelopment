make: elevators.out

elevators.out: main.o printer.o elevators.o
	gcc main.o printer.o elevators.o -o elevators.out

main.o: main.c
	gcc -c main.c

printer.o: printer.c
	gcc -c printer.c

elevators.o: elevators.c
	gcc -c elevators.c

clean:
	rm -rf *.o *.out

start: elevators.out
	./elevators.out