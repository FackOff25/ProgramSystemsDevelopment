make: mpi.out

mpi.out:
	mpicc main.c -o mpi.out

start: mpi.out
	mpirun -np 5 ./mpi.out