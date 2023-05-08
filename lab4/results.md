# 1 thread
mpiexec -n 1 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 87.651271
# 2 threads
mpiexec -n 2 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 34.688125
# 4 threads
mpiexec -n 4 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 15.767512
# 8 threads

mpiexec -n 8 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 7.489143
