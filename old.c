#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_COUNT 16
#define ARRAY_WIDTH 18 //kinda cheating to avoid the int** weirdness in initialization
#define ARRAY_HEIGHT 10
#define SEED 123
#define EVOLUTIONS 12
#define HIGHVALUE 20

//Typedefs
typedef void (*WorkFunction)();

//Stubs
WorkFunction requestWork();
WorkFunction generateDoWork(int x, int y);
WorkFunction generateBarrierWait();
void fillInitialArray();
void modifyTargetValue(int x, int y, int value);
int readCurrentValue(int x, int y);
int sumBlock(int x, int y);
void prettyPrint(int iterationID, int array[ARRAY_WIDTH][ARRAY_HEIGHT]);
void resetPointerForIteration();
int applyTransformation(int i, int initial);
void* doWork(void* parameter);

//Global State
int first[ARRAY_WIDTH][ARRAY_HEIGHT];
int second[ARRAY_WIDTH][ARRAY_HEIGHT];
int target = 1;
int currentX;
int currentY;
pthread_barrier_t evolutionBarrier;
pthread_mutex_t popMutex;

int main()
{
    pthread_mutex_init(&popMutex, NULL);
    fillInitialArray();
    prettyPrint(0, first);
    for(int i = 0; i < EVOLUTIONS; ++i) //foreach evolution
    {
        pthread_t threads[THREAD_COUNT];
        pthread_barrier_init(&evolutionBarrier, NULL, THREAD_COUNT + 1);
        target = target == 1 ? 2 : 1; //Change target
        resetPointerForIteration();
        for(int j = 0; j < THREAD_COUNT; ++j) //foreach thread
        {
            pthread_create(&threads[j], NULL, doWork, (void*)(long)(i));
        }
        pthread_barrier_wait(&evolutionBarrier);


        for (int k=0; k < THREAD_COUNT; k++) {
            //to be absolutely honest I'm not 100% sure this is necessary, but the idea of leaving threads out
            //there forever sets my teeth on edge, so I'm erring on the side of completeness.
            pthread_join(threads[k], NULL);
        }

        prettyPrint(i + 1, (target == 1 ? first : second));
        pthread_barrier_destroy(&evolutionBarrier);
    }
}

void* doWork(void* parameter)
{
    requestWork()();
}

void resetPointerForIteration() 
{
    currentX = -1; //kinda hacky but fixes the off by one error.
    currentY = 0;
}

void fillInitialArray() {    
    srand(SEED);
    for(int y = 0; y < ARRAY_HEIGHT; ++y)
    {
        for(int x = 0; x < ARRAY_WIDTH; ++x) 
        {
            modifyTargetValue(x,y,rand() % HIGHVALUE);
        }
    }
}

void modifyTargetValue(int x, int y, int value)
{
    if(target == 1) 
    {
        first[x][y] = value;
        return;
    }
    second[x][y] = value;
}

int readCurrentValue(int x, int y) {
    if(x < 0 || x >= ARRAY_WIDTH || y < 0 || y >= ARRAY_HEIGHT) 
    {
        return 0; 
        //this covers our edge cases, and acts as a zero padding without forcing me to do any extra work or use offsets.
    }
    return target == 1 ? second[x][y] : first[x][y];
}

int sumBlock(int x, int y)
{
    //remember for this we are assuming a top left orientation.
    //this means x+ is right and y+ is down, this shouldn't effect the outcome
    //as long as we're consistent.
    int sum = 0;
    sum += readCurrentValue(x-1, y-1); //topleft
    sum += readCurrentValue(x, y-1); //midleft
    sum += readCurrentValue(x+1, y-1); //rightleft
    sum += readCurrentValue(x-1, y); //midleft
    sum += readCurrentValue(x, y); //midmid
    sum += readCurrentValue(x+1, y); //midright
    sum += readCurrentValue(x-1, y+1); //botleft
    sum += readCurrentValue(x, y+1); //botmid
    sum += readCurrentValue(x+1, y+1); //botright
    return sum;
}

WorkFunction generateDoWork(int x, int y) {
    void lambda() {
        //turns out C99 doesn't handle closures. Had to set these variables
        // or the x/y would not get saved and would segfault out on me.
        // the more you know right?
        int innerX = x;
        int innerY = y;
        modifyTargetValue(innerX, innerY, applyTransformation(sumBlock(innerX, innerY), readCurrentValue(innerX,innerY)));
        WorkFunction work = requestWork();
        work();
    }
    return lambda;
}

WorkFunction requestWork()
{
    pthread_mutex_lock(&popMutex);
    currentX++;
    if(currentX >= ARRAY_WIDTH) //handles wrap to next line
    {
        currentX = 0;
        currentY++;
    }

    if(currentY >= ARRAY_HEIGHT) //handles early out
    {
        pthread_mutex_unlock(&popMutex);
        return generateBarrierWait();
    } 

    int x = currentX;
    int y = currentY;
    pthread_mutex_unlock(&popMutex);

    return generateDoWork(x, y);
}

WorkFunction generateBarrierWait() {
    void lambda() {
        pthread_barrier_wait(&evolutionBarrier);
    }
    return lambda;
}

int applyTransformation(int i, int initial)
{
    if(i % 10 == 0) 
    {
        return 0;
    }

    if(i < 50)
    {
        return initial + 3;
    }

    if(i > 150) 
    {
        return 1;
    }

    return initial - 3 < 0 ? 0 : initial - 3;    
}

void prettyPrint(int iterationID, int array[ARRAY_WIDTH][ARRAY_HEIGHT]) 
{
    printf("Iteration %d:\n", iterationID);
    
    for(int y = 0; y < ARRAY_HEIGHT; ++y)
    {
        for(int x = 0; x < ARRAY_WIDTH; ++x) 
        {
            printf("%5d", array[x][y]);
        }
        printf("\n");
    }
    printf("\n");
}
