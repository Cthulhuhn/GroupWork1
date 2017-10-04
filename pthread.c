//*****************************************************************/
// Author: 20Goto10
// Description: This is the pthread version of the matrix
//  multiplcation problem. This abstracts the movement of the
//  iteration over the matrices into a linked list of recursive
//  functions over a notional linked list of functions. This
//  linked list is modeled as a recursive call to requesting
//  additional work allowing eventually terminating in a notional
//  wait method. This allows the threading of this solution to
//  any amount, as each notional linked list is seperate from
//  eachother during runtime, but aren't seperate at compile-
//  time.
// Model Example:
//  let R = RequestWork
//  let G = GenerateWork (solves a slot then calls R)
//  let T = GenerateBarrierWait (acts as a wait/terminator for the
//      work when no more work exists.)
//  let R -> (G|T)
//  Call stack looks like: R() -> G() -> G() -> ... -> T()
//  where G eventually calls an R which returns a T halting the
//  program
//*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define THREAD_COUNT 5
#define SIZE 1000
#define SEED 34
#define HIGH 5

//Typedefs
typedef bool (*WorkFunction)();

//Stubs
void* doWork(void* parameter);
WorkFunction requestWork();
WorkFunction generateDoWork(int x, int y);
WorkFunction generateTerminate();
int getDotProduct(int row, int col);
void fillMatrix(int matrix[SIZE][SIZE]);
void prettyPrint(char* name, int matrix[SIZE][SIZE]);

//Global State
int matA[SIZE][SIZE];
int matB[SIZE][SIZE];
int matSolution[SIZE][SIZE];
int currentX = -1; //makes the iteration below simpler. Could solve the off by one down there too though.
int currentY = 0;
pthread_barrier_t solutionBarrier;
pthread_mutex_t popMutex;

//entry
int main()
{
    srand(SEED);
    fillMatrix(matA);
    fillMatrix(matB);

    //prettyPrint("A", matA);
    //prettyPrint("B", matB);

    struct timespec start, finish;
    double elapsed;
    
    pthread_mutex_init(&popMutex, NULL);
    pthread_t threads[THREAD_COUNT];
    pthread_barrier_init(&solutionBarrier, NULL, THREAD_COUNT + 1);
    clock_gettime(CLOCK_MONOTONIC, &start);    

    for(int i = 0; i < THREAD_COUNT; ++i)
    {
        pthread_create(&threads[i], NULL, doWork, (void*)(long)i);
    }
    pthread_barrier_wait(&solutionBarrier);
    clock_gettime(CLOCK_MONOTONIC, &finish);


    for(int j = 0; j < THREAD_COUNT; ++j)
    {
        //To be perfectly honest, I'm not 100% sure this is necessary
        //  but leaving threads in fire and forget mode makes my
        //  eye twitch... so I'm cleaning up here.
        pthread_join(threads[j], NULL);
    }

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    
    //prettyPrint("Solution", matSolution);
    printf("Time elapsed (ms): %f\n", elapsed);
    pthread_barrier_destroy(&solutionBarrier);
}

//work wrapper for our request lambda launch
void* doWork(void* parameter)
{
    while(requestWork()());
}

//Returns lambda for next unit of work or a terminator.
WorkFunction requestWork()
{
    pthread_mutex_lock(&popMutex);
    currentX++;
    if(currentX >= SIZE) //handles wrap to next line
    {
        currentX = 0;
        currentY++;
    }

    //this variable needs to be set before we release the lock or
    //  we risk race conditions.
    if(currentY >= SIZE) //handles early out
    {
        pthread_mutex_unlock(&popMutex);
        return generateTerminate();
    }
    int x = currentX;
    int y = currentY;
    pthread_mutex_unlock(&popMutex);
    return generateDoWork(x, y);
}

//Generates lambda for doing work.
WorkFunction generateDoWork(int x, int y) {
    bool lambda() {
        // C doesn't handle closures. These variables
        // avoid a segfault.
        int innerX = x;
        int innerY = y;
        matSolution[innerX][innerY] = getDotProduct(innerX,innerY);
        return true;
    }
    return lambda;
}

//Generates terminator for thead in algorithm
WorkFunction generateTerminate() {
    bool lambda() {
        pthread_barrier_wait(&solutionBarrier);
        return false;
    }
    return lambda;
}

//Gets the dot product for a given row and column
int getDotProduct(int row, int col)
{
    //I suppose this could be threadable too... but it would super complicate the algorithm and would
    // explode the number of threads required to solve for a single spot. I wouldn't recommend this.
    int sum = 0;
    for(int i = 0; i < SIZE; ++i)
    {
        sum += matA[i][col] * matB[row][i];
    }
    return sum;
}

//Helper to fill matrix with random value.
void fillMatrix(int matrix[SIZE][SIZE]) {
    for(int y = 0; y < SIZE; ++y)
    {
        for(int x = 0; x < SIZE; ++x)
        {
            matrix[x][y] = rand() % HIGH;
        }
    }
}

//Helper function for printing a matrix.
void prettyPrint(char* name, int matrix[SIZE][SIZE])
{
    printf("Matrix %s:\n", name);

    for(int y = 0; y < SIZE; ++y)
    {
        for(int x = 0; x < SIZE; ++x)
        {
            printf("%5d", matrix[x][y]);
        }
        printf("\n");
    }
    printf("\n");
}
