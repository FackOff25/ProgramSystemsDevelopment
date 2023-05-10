# 1 thread
mpiexec -n 1 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 64.856395
# 2 threads
mpiexec -n 2 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 35.865047
# 4 threads
mpiexec -n 4 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 17.749354
# 8 threads

mpiexec -n 8 ./mpi.out 2048 1 0

num_r_el 2048 

Time: 9.484734

