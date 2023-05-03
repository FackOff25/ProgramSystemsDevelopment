make: mpi.out

mpi.out:
	mpic++ main.c -o mpi.out -lm

start: mpi.out
	mpiexec -n $(threads) ./mpi.out $(elements) 1

plot: gnuScript.gnu result.plt
	gnuplot gnuScript.gnu -p

clean:
	rm -rf *.o *.out *.gnu *.plt *.btr
