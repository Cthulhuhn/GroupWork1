//*****************************************************************/
// Author: 20Goto10
// Description: This is the sequential version of the matrix
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
#include <stdbool.h>
#include <omp.h>					//ADDED header
#include <time.h>
#include <unistd.h>

#define SIZE 3
#define SEED 34
#define HIGH 5

//Typedefs
typedef bool (*WorkFunction)();

//Stubs
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

//entry
int main()
{
    srand(SEED);
    fillMatrix(matA);
    fillMatrix(matB);

    //prettyPrint("A", matA);
    //prettyPrint("B", matB);

    clock_t t1, t2;

    //this is where the multi threading magic goes. Any amount of threads can make this call and have this work.
    t1 = clock();
    #pragma omp parallel			//ADDED multi threading magic
    while(requestWork()());
    t2 = clock();
    //prettyPrint("Solution", matSolution);
    printf("Time elapsed (ms): %f\n", 1000*(t2-t1)/(double) (CLOCKS_PER_SEC));
}

//Returns lambda for next unit of work or a terminator.
WorkFunction requestWork()
{
    int x, y;
	#pragma omp critical			//ADDED critical section
	{								//ADDED scoping
		currentX++;
		if(currentX >= SIZE) //handles wrap to next line
		{
			currentX = 0;
			currentY++;
        }
        x = currentX;
        y = currentY;
	}								//ADDED scoping

    if(y >= SIZE) //handles early out
    {
        return generateTerminate();
    }

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
    //printf(".\n");				//ADDED to verify the # of calls to this function
    return lambda;
}

//Generates terminator for thead in algorithm
WorkFunction generateTerminate() {
    bool lambda() {
        //todo: wait.
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
