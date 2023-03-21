#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_SIZE 20

//first, second arrays a,b; 3 versions of the third array c1, c2, c3
int a[MAX_SIZE][MAX_SIZE], b[MAX_SIZE][MAX_SIZE], c1[MAX_SIZE][MAX_SIZE], c2[MAX_SIZE][MAX_SIZE], c3[MAX_SIZE][MAX_SIZE];
int x, y, z;            //dimensions of the arrays a(xz), b(zy), c(xy)

//struct to carry thread data to be passed as thread create function argument
struct thread_data
{
    int row;
    int col;
};

//function to multiply 2 matrices using 1 thread
void* multiply_per_matrix(){
    for(int i = 0; i < x; i++){
        for(int j = 0; j < z; j++){
            c1[i][j] = 0;
            for(int k = 0; k < y; k++){
                c1[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

//function to multiply 2 matrices using row threads
void* multiply_per_row(void* arg){
    int row = (int) arg;
    for(int j = 0; j < z; j++){
        c2[row][j] = 0;
        for(int k = 0; k < y; k++){
            c2[row][j] += a[row][k] * b[k][j];
        }
    }
    pthread_exit(NULL);
}

//function to multiply 2 matrices using row*col threads
void* multiply_per_element(void* args){
    struct thread_data* data = (struct thread_data *)args;
    int row = data->row;
    int col = data->col;
    c3[row][col] = 0;
    for(int k = 0; k < y; k++){
        c3[row][col] += a[row][k] * b[k][col];
    }
    pthread_exit(NULL);
}

//function to read file
void readFile(char* fileName, int check){
    FILE *file = fopen(fileName, "r");  // Open the file for reading
    int row, col;
    int arr[MAX_SIZE][MAX_SIZE];
    fscanf(file, "row=%d col=%d\n", &row, &col);
    // Read the integers from the file and store them in the a
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            fscanf(file, "%d", &arr[i][j]);
        }
    }
    if(check == 1){
        //first array
        x = row; y = col; memcpy(a, arr, sizeof(arr));
    }else{
        //second array
        y = row; z = col; memcpy(b, arr, sizeof(arr));
    }
    fclose(file);  // Close the file
}

void thread_per_matrix_method(char *outputFile){
    //use stop and start to calculate time for each method 
    struct timeval start, end;
    // multiply_per_matrix
    pthread_t thread_per_matrix;
    gettimeofday(&start, NULL); //start checking time
    //create the single thread
    // if (pthread_create(&thread_per_matrix, NULL, multiply_per_matrix, NULL)){
    //     printf("ERROR in create thread of multiply_per_matrix\n");
    //     exit(-1);
    // }
    //join for the single thread
    // if(pthread_join(thread_per_matrix, NULL)){
    //     printf("ERROR in joining thread of multiply_per_matrix\n");
    //     exit(-1);
    // }
    multiply_per_matrix();
    gettimeofday(&end, NULL); //end checking time
    printf("\033[0;36mMethod: A thread per matrix\033[0m\n");
    printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    printf("\033[0;32mNumber of created threads:\033[0m \033[0;36m%d thread\033[0m\n", 1);
    printf("\033[0;32mTime taken in microseconds:\033[0m \033[0;36m%lu us\033[0m\n", end.tv_usec - start.tv_usec);
    printf("---------------------------------------------------------------\n");

    //write to per_matrix text file
    char temp[20];
    strcpy(temp, outputFile);
    strcat(temp, "_per_matrix.txt\0");

    FILE *file1 =  fopen(temp,"w");
    fprintf(file1, "Method: A thread per matrix\nrow=%d col=%d\n", x, z);
    for(int i = 0; i < x; i++){
        for(int j = 0; j < z; j++){
            fprintf(file1,"%d ", c1[i][j]);
        }
        fprintf(file1, "\n");
    }
    fclose(file1);
}

void thread_per_row_method(char* outputFile){
    //multiply_per_row
    pthread_t rowThreads[x];
    struct timeval start, end;
    gettimeofday(&start,NULL);
    //create the row threads
    for(int i = 0; i < x; i++){
        if (pthread_create(&rowThreads[i], NULL, multiply_per_row, (void*) i)){
            printf("ERROR in create thread %d of multiply_per_row\n", i);
            exit(-1);
        }
    }
    //join for the row threads
    for(int i = 0; i < x; i++){
        if(pthread_join(rowThreads[i], NULL)){
            printf("ERROR in joining thread %d of multiply_per_row\n", i);
            exit(-1);
        }
    }
    gettimeofday(&end,NULL);

    printf("\033[0;36mMethod: A thread per row\033[0m\n");
    printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    printf("\033[0;32mNumber of created threads:\033[0m \033[0;36m%d threads\033[0m\n", x);
    printf("\033[0;32mTime taken in microseconds:\033[0m \033[0;36m%lu us\033[0m\n", end.tv_usec - start.tv_usec);
    printf("---------------------------------------------------------------\n");

    //write to per_row text file
    char temp[20];
    strcpy(temp, outputFile);
    strcat(temp, "_per_row.txt\0");

    FILE *file1 =  fopen(temp,"w");
    fprintf(file1, "Method: A thread per row\nrow=%d col=%d\n", x, z);
    for(int i = 0; i < x; i++){
        for(int j = 0; j < z; j++){
            fprintf(file1,"%d ", c2[i][j]);
        }
        fprintf(file1, "\n");
    }
    fclose(file1);
}

void thread_per_element_method(char* outputFile){
    //multiply_per_element
    pthread_t elementThreads[x][z];
    struct thread_data *tdata[x][z];
    struct timeval start, end;
    gettimeofday(&start, NULL);
    //create the row*col threads
    for(int i = 0; i < x; i++){
        for(int j = 0; j < z; j++){
            tdata[i][j] = malloc(sizeof(struct thread_data));
            tdata[i][j]->row = i;
            tdata[i][j]->col = j;
            if (pthread_create(&elementThreads[i][j], NULL, multiply_per_element, (void*) tdata[i][j])){
                printf("ERROR in create thread of row: %d, col: %d of multiply_per_element\n", i, j); 
                exit(-1);
            }
        }
    }

    //join for the row*col threads
    for(int i = 0; i < x; i++){
        for(int j = 0; j < z; j++){
            if(pthread_join(elementThreads[i][j], NULL)){
                printf("ERROR in joining thread of row: %d, col: %d of multiply_per_element\n", i, j);
                exit(-1);
            }else{
                //free the the memory allocated in the dynamic heap for the struct
                free(tdata[i][j]);
            }
        }
    }
    gettimeofday(&end, NULL);
    printf("\033[0;36mMethod: A thread per element\033[0m\n");
    printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    printf("\033[0;32mNumber of created threads:\033[0m \033[0;36m%d threads\033[0m\n", x*z);
    printf("\033[0;32mTime taken in microseconds:\033[0m \033[0;36m%lu us\033[0m\n", end.tv_usec - start.tv_usec);
    //write to per_element text file
    char temp[20];
    strcpy(temp, outputFile);
    strcat(temp, "_per_element.txt\0");

    FILE *file1 =  fopen(temp,"w");
    fprintf(file1, "Method: A thread per element\nrow=%d col=%d\n", x, z);
    for(int i = 0; i < x; i++){
        for(int j = 0; j < z; j++){
            fprintf(file1,"%d ", c3[i][j]);
        }
        fprintf(file1, "\n");
    }
    fclose(file1);
}

int main(int argc, char **argv){
    //get files names
    char *f1, *f2, *f3;
    if(argc == 1){
        f1 = "a"; f2 = "b"; f3 = "c";
    }else if(argc == 2){
        f1 = argv[1]; f2 = "b"; f3 = "c";
    }else if(argc == 3){
        f1 = argv[1]; f2 = argv[2]; f3 = "c";
    }else if(argc == 4){
        f1 = argv[1]; f2 = argv[2]; f3 = argv[3];
    }else{
        printf("wrong input format\n");
        return 1;
    }
    //read the first array
    char temp[20];
    strcpy(temp, f1);
    strcat(temp, ".txt\0");
    readFile(temp, 1);

    //read the second array
    strcpy(temp, f2);
    strcat(temp, ".txt\0");
    readFile(temp, 2);

    //multiply per matrix
    thread_per_matrix_method(f3);
    //multiply per row
    thread_per_row_method(f3);
    //multiply per element
    thread_per_element_method(f3);
    return 0;
}