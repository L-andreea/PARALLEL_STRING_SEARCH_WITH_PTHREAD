# PARALLEL_STRING_SEARCH_WITH_PTHREAD

in this folder is a parallel string search that parallelize the Knuth-Morris-Pratt algorithm with pthread. 

- reader.c read the file in input
- pthread_search.c implement algorithm parallelize 


for compile run:
  
-gcc -Wall -o pthread_search pthread_search.c -lpthread

for run:
  
-./pthread_serach <number of thread> <string to search> <path file>
