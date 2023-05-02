#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#define DEF_ROW_PROC 4
#define Ia 10.0
#define R 1000.0
#define C 1e-5

#define scriptFile "gnuScript.gnu"
#define resultsFile "result.plt"

int time_interval;
int num_of_elements;
int num_of_rows;
int num_of_process;
int time_moment = 0;
double time_step = R * C / 10;

struct timeval tv1, tv2, dtv;
struct timezone tz;

void makeGNUscript(char* scriptFilename, char* resultFilename, int elNum){
    elNum = elNum * elNum;
    FILE *script = fopen(scriptFilename, "w");
    fprintf(script, "filedata = '%s'\n", resultFilename);
    fprintf(script, "n = system(sprintf('cat %%s | wc -l', filedata))\n");
    fprintf(script, "set hidden3d\nset dgrid3d 50,50 qnorm 2\nset zrange [0:%f]\n", time_interval / time_step);
    fprintf(script, "do for [i=%d+1:n:%d] {\n", elNum, elNum);
    fprintf(script, "set title 'time '.i/%d\n", elNum);
    fprintf(script, "splot filedata every ::(i-%d)::i w l ls 1 \npause 0.1\n}\n", elNum);
    fclose(script);
}

void startAnimation(char* scriptFilename){
    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    fprintf(gnuplotPipe, "load \"%s\"\n", scriptFilename);
    fflush(gnuplotPipe);
}

int main(int argc, char **argv) {
    double *F_matrix, *f_matrix;
    double *N_matrix, *n_matrix;
    double *p;
    double *p_n_matrix, *el_prev, *el_last;
    int intBuf[4];
    int i, j, row_in_proc, row_elems;
    int myrank, total; //Параметры MPI
    int first, last;
    double time;
    time_step = R * C / 10;

    FILE *res = fopen(resultsFile, "w");
    MPI_Status *status1;
    MPI_Status *status2;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &total);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    if (!myrank) { //Только основной процесс исполнит этот блок
        if (argc >= 3) {
            num_of_elements = atoi(argv[1]);
            time_interval = atoi(argv[2]);
        }
        num_of_process = total;
        if ((num_of_elements % num_of_process != 0) || (num_of_elements < num_of_process)) {
            printf("number of processes is not multuple to elems\n");
            exit(0);
        }
        num_of_rows = num_of_elements;
        row_elems = num_of_elements;
        printf("num_r_el %d \n", row_elems);
        intBuf[0] = row_elems; //количество элементов в строке
        intBuf[1] = num_of_rows / num_of_process; //количество строк на процесс
        intBuf[2] = num_of_process; //количество процессов
        intBuf[3] = time_interval; //время работы
        N_matrix = (double *) calloc(num_of_elements * num_of_elements, sizeof(double)); //матрица i+1 момента
        F_matrix = (double *) calloc(num_of_elements * num_of_elements, sizeof(double)); //матрица i момента
        int max = time_interval / time_step;
        for(int j = 0; j < num_of_elements; ++j){
            F_matrix[j] = max;
            F_matrix[j*row_elems] = max;
            F_matrix[j*row_elems + row_elems - 1] = max;
            F_matrix[(num_of_rows-1)*row_elems + j] = max;
        }
    }

    MPI_Bcast((void *) intBuf, 4, MPI_INT, 0, MPI_COMM_WORLD);
    int n = intBuf[0];
    int m = intBuf[1];
    int kol = intBuf[2];
    time_interval = intBuf[3];
    int in1, in2, in;
    
    el_prev = (double *) malloc(n * sizeof(double)); //элементы предыдущей строки
    el_last = (double *) malloc(n * sizeof(double)); //элементы следующей строки
    f_matrix = (double *) malloc(n * m * sizeof(double)); //матрица текущего момента
    n_matrix = (double *) malloc(n * m * sizeof(double)); //матрица следующего момента
    

    gettimeofday(&tv1, &tz);
    MPI_Scatter((void *) F_matrix, n * m, MPI_DOUBLE, (void *) f_matrix, n * m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for (time = 0; time < time_interval / time_step; ++time) {
        status1 = (MPI_Status *) malloc(sizeof(MPI_Status));
        status2 = (MPI_Status *) malloc(sizeof(MPI_Status));

        in1 = (myrank == kol - 1) ? kol - 1 : myrank + 1;
        in2 = (!myrank) ? 0 : myrank - 1;

        if (myrank != kol - 1)MPI_Send(f_matrix + n * (m - 1), n, MPI_DOUBLE, in1, 1, MPI_COMM_WORLD);
        if (myrank)MPI_Send(f_matrix, n, MPI_DOUBLE, in2, 2, MPI_COMM_WORLD);
        if (myrank)MPI_Recv(el_prev, n, MPI_DOUBLE, in2, 1, MPI_COMM_WORLD, status1);
        if (myrank != kol - 1) MPI_Recv(el_last, n, MPI_DOUBLE, in1, 2, MPI_COMM_WORLD, status2);

        for (i = 0; i < m; ++i)
            for (j = 0; j < n; ++j) {
                if ((!myrank && (!i)) || ((myrank == kol - 1) && (i == m - 1)) || (!j) ||
                    (j == n - 1)) { //если на границе
                    n_matrix[i * n + j] = (time <= (time_interval / (10 * time_step))) ? Ia * R : 0.0;
                } else if (i == 0) //если на границе
                    n_matrix[i * n + j] =
                            (f_matrix[(j - 1) + n * i] - 4 * f_matrix[j + n * i] + f_matrix[(j + 1) + n * i] +
                             el_prev[j] + f_matrix[(j) + n * (i + 1)]) * time_step / (R * C) + f_matrix[j + n * i];
                else if (i == m - 1) //если на границе
                    n_matrix[i * n + j] =
                            (f_matrix[(j - 1) + n * i] - 4 * f_matrix[j + n * i] + f_matrix[(j + 1) + n * i] +
                             el_last[j] + f_matrix[(j) + n * (i - 1)]) * time_step / (R * C) + f_matrix[j + n * i];
                else //в центре
                    n_matrix[i * n + j] =
                            (f_matrix[(j - 1) + n * i] - 4 * f_matrix[j + n * i] + f_matrix[(j + 1) + n * i] +
                             f_matrix[(j) + n * (i + 1)] + f_matrix[(j) + n * (i - 1)]) * time_step / (R * C) +
                            f_matrix[j + n * i];

            }

        MPI_Gather((void *) n_matrix, m * n, MPI_DOUBLE, (void *) N_matrix, m * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        p = f_matrix;
        f_matrix = n_matrix;
        n_matrix = p;
        if (!myrank) { //Запись результатов для графика
            for (i = 0; i < row_elems * num_of_rows; i++)
                fprintf(res, "%# -15g %# -15g %# -15g\n", (double) (i / num_of_rows), (double) (i % num_of_rows),
                        N_matrix[i]);
            fflush(res);
        }
    }

    if (!myrank) {
        gettimeofday(&tv2, &tz);
        dtv.tv_sec = tv2.tv_sec - tv1.tv_sec;
        dtv.tv_usec = tv2.tv_usec - tv1.tv_usec;
        if (dtv.tv_usec < 0) {
            dtv.tv_sec--;
            dtv.tv_usec += 1000000;
        }
        printf("Time: %ld .%ld\n", (dtv.tv_sec * 1000000 + dtv.tv_usec) / 1000000,
               (dtv.tv_sec * 1000000 + dtv.tv_usec) % 1000000);
    }

    MPI_Finalize();

    makeGNUscript(scriptFile, resultsFile, atoi(argv[1]));
    //startAnimation(scriptFile);

    exit(0);

}