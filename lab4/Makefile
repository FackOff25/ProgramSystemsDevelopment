make: mpi.out

mpi.out:
	mpicc main.c -o mpi.out -lm

start: mpi.out
	mpiexec -n 10 ./mpi.out 5000 1

plot: gnuScript.gnu result.plt
	gnuplot gnuScript.gnu -p

clean:
	rm -rf *.o *.out *.gnu *.plt *.btr
