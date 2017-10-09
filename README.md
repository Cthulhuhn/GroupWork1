# MatMul
Project Team: 20goto10  
Built for CSC410  
Due 10/10/17  
Hosted at https://github.com/Cthulhuhn/MatMul  

Authors: Lucas Roe, Charles Huhn, Shawn Cheek


TO BUILD (where % is the prompt):  
mmseq.c:     % gcc mmseq.c  
mmpthread.c: % gcc mmpthread.c -lpthread  
mmomp.c:     % gcc mmomp.c -omp  

TO RUN:  
\[all versions]: %./a.out


SHORT DESCRIPTION:  
The program will print out the input matrices, then calculate and print the solution matrix.  Finally, the time it took to calculate the solution matrix will display and the program exits.  

The size of the matrices, seed for random values, and highest possible input matrix value can be defined by changing the preprocessor macros SIZE, SEED, and HIGH, respectively.
