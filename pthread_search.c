#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "reader.c"


//decler function

void *create_tabel(void * rank);
void *patternFound(void * chunk);
void *reader(void * rank);
void check_create(int ret);
void check_join(int ret);

//var usede
long thread_count;   // number of thread
char *file;          // pointer of input file
char *pattern;       // pointer of string to search
int *table;          // pointer of ausiliar array for string search
long *status_thread; // array for status of the thread
char *name_file;    //name of the file in input 
long chunk_file;    //number of division of the file 
pthread_mutex_t mutex;  //mutex for critical section add resul 
pthread_mutex_t mutex_worker; //mutex for critical section change status of worker
long count_task = 0; //number of task finish 
int len_file;  // size of the file in input 
int len_pattern ; //size of string to found
long rest;  //modul of the divison from len_file and chank_file
long q;  // size of chunk 

pthread_t *thread_handles; 


long t1[2]={0,0}; //number of task in parallel 
long *t2; //number of task in parallel 


// var for check the status of the thread
int static waitWork = 0, working = 3, finish =2 , ex=1, lost=150;
int kill=0;  //number of thread kill
int sum = 0; // number of ripetition founded 
int flag = 1; //var for flag 
long x =0;

int main(int argc, char *argv[]){
    // chek input
    // check the number of argument
    if (argc < 4)
    {
        printf("\n number of argument isufficent please insert nameFile <numbero of thread> <patter to search> <path of text> \n");
        return 1;
    }
    // check numbero of thread
    thread_count = strtol(argv[1], NULL, 10);
    
    if (thread_count < 2)
    {
        printf("\n number of thread inssuficent nuber of thread need to be >= 2\n");
        return 1;
    }
    chunk_file = thread_count;

    clock_t start , end;
    start = clock();
	

    pthread_mutex_init(&mutex_worker, NULL);
    pattern = argv[2];
    len_pattern=strlen(argv[2]);
    name_file = argv[3];
    thread_handles = malloc(thread_count * sizeof(pthread_t));
    // inizialize the status of the thread at 0
    status_thread = (long *)malloc(thread_count * sizeof(long));
    for (long i = 0; i < thread_count; i++)
    {
        status_thread[i] = waitWork;
    }

    table = malloc(strlen(pattern) * sizeof(int));
    //change status of thread 
    pthread_mutex_lock(&mutex_worker);
    status_thread[0] = working;
    status_thread[1] = working;
        
    pthread_mutex_unlock(&mutex_worker);
    //send thread 
    pthread_create(&thread_handles[0], NULL, create_tabel, (void *)0);
     
    pthread_create(&thread_handles[1], NULL, reader, (void*)1);
    //assegn thread to task 
        t1[0]=0;
        t1[1]=1;

    //check work finish 
    while(flag==1){
        //check finish work 
            if(status_thread[t1[0]]==finish && status_thread[t1[1]]==finish){
                flag = 0;
                break;
            }
            else{
                
                if(kill!= 0){
                
                    for(long i = 0; i < thread_count;i++){
                        //if don't need anther thread break 
                        if(kill == 0){
                                break;
                            }
                        //if thread[i] is free for work then send at task 
                        if(status_thread[i]==waitWork || status_thread[i]==finish){
                            
                            if(status_thread[t1[0]]==lost){
                                pthread_create(&thread_handles[i], NULL, create_tabel,(void*)i);
                                kill--;
                                t1[0]=i;
                                pthread_mutex_lock(&mutex_worker);
                                status_thread[i]=working;
                                pthread_mutex_unlock(&mutex_worker);
                                

                            }
                            if(status_thread[t1[1]]==lost){
                                pthread_create(&thread_handles[i], NULL, reader,(void*)i);
                                kill--;
                                t1[1]=i;
                                pthread_mutex_lock(&mutex_worker);
                                status_thread[i]=working;
                                pthread_mutex_unlock(&mutex_worker);

                            }
                        }
                    }
                }

                
                //check status worker assign at the task
                if(status_thread[t1[0]]==2 && status_thread[t1[1]]==2){
                  flag = 0;
                  break;
                
                }
                
                for(long i = 0 ; i < 2;i++){
                    if(status_thread[t1[i]]==working){
                        status_thread[t1[i]]++;
                    }
                }
    
               
            }
        }
   
     
   //make the join with the precedent thread
    for(long i = 0; i < thread_count; i++){
		
        if(status_thread[i]==finish || status_thread[i]==ex){
            pthread_join(thread_handles[i],NULL);
            pthread_mutex_lock(&mutex_worker);
            status_thread[i]=waitWork;
            pthread_mutex_unlock(&mutex_worker);
			
        }
    }

    // error if the file not exist
    if (file == NULL)
    {
        printf("non such file: insert a vild path \n");
        return 1;
    }

    len_file=strlen(file);
    //make cunk file 
	while(chunk_file>= strlen(file)){
      
		chunk_file = chunk_file/2;
	}
    t2 = malloc(sizeof(long)*chunk_file);
	
    rest = len_file % chunk_file;
    q = (len_file /chunk_file) + rest; //size of chunk 
    //assagne thread task
	for(long i = 0; i < chunk_file;i++){
       
		while(status_thread[x]==lost ||status_thread[x]==working){
			x++;
          
		}
       
        pthread_mutex_lock(&mutex_worker);
		pthread_create(&thread_handles[x], NULL, patternFound, (void *)i);
		
		status_thread[x]=working;
		t2[i]=x;
		pthread_mutex_unlock(&mutex_worker);
		
	}
   //check status thread 
	while(flag==0){
		
		if(count_task == chunk_file){
		
			flag = 1;
			break;
		}
		else{
			
			if(kill!=0){
				
				long j = 0;
				for(long i = 0; i < thread_count; i++){
					if(kill==0 || j==chunk_file-1){
						break;
					}
					while(status_thread[t2[j]]!=working || status_thread[t2[j]]!=lost){
						j++;
					}
					if(status_thread[i]!=lost && status_thread[i]!=working){
						pthread_create(&thread_handles[i],NULL, patternFound, (void *)j);
						pthread_mutex_lock(&mutex_worker);
						status_thread[i]=working;
						kill--;
						pthread_mutex_unlock(&mutex_worker);
					}

				}

				
			}
			
			//sleep(0.001);
			if(count_task == chunk_file){
                  flag = 1;
				  
			}
            //sleep(0.001);
			for(long i = 0 ; i < chunk_file; i++){
				if(status_thread[t2[i]]==working){
					pthread_mutex_lock(&mutex_worker);
					status_thread[t2[i]]++;
					pthread_mutex_unlock(&mutex_worker);
				}
			}
		}

	}

     for(long i = 0; i < thread_count; i++){
    
            pthread_join(thread_handles[i],NULL);
            
        
    }
    
    
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_worker);
    end = clock();
    double time_spent = (double)(end-start)/CLOCKS_PER_SEC;
    printf("\ntime spent%f \n", time_spent);

    printf("\n printer pattern %s , number of repatiotion foundend: %d\n", pattern, sum);

    free(thread_handles);

   // clock_t finish = clock();

    return 0;


			}
		

void *reader(void * rank){
	   long my_rank = (long)rank;
  
    //check the status of the thread if is lost exit 
    if(status_thread[my_rank ]== lost){
        pthread_mutex_lock(&mutex_worker);
        status_thread[my_rank]=ex;
        pthread_mutex_unlock(&mutex_worker);
        
        pthread_exit(NULL);
    }
    else{ //read the file 
    file = read_file(name_file);
    pthread_mutex_lock(&mutex_worker);
    status_thread[my_rank]=finish;
    pthread_mutex_unlock(&mutex_worker);
    }
    return NULL;
    
}

//create the tabel 
void *create_tabel(void * rank){   
    
long my_rank = (long) rank;

if(status_thread[my_rank]==lost){
        pthread_mutex_lock(&mutex_worker);
        status_thread[my_rank]=ex;
       
        pthread_mutex_unlock(&mutex_worker);
        pthread_exit(NULL);
    }
else{
        table[0]=0;
        int len = 0;
        int j = 1;
    while (j < len_pattern){
        if (pattern[j] == pattern[len])
        {
            len++;
            table[j] = len;
            j++;
        }
        else{
            if (len != 0){
                len = table[len]-1; 
            }
            else{
                table[j - 1] = 0;
                table[j] = 0;
                j++;
            }
        }
    }
    pthread_mutex_lock(&mutex_worker);
    status_thread[my_rank] = 2;
    pthread_mutex_unlock(&mutex_worker);
    
    }

	return NULL;
}


void *patternFound(void * chunk){
	long my_chunk = (long) chunk;
    int count_pattern = 0;  //number of ripetition founded
    int i = 0;
    long j = q * my_chunk;   //when to start to control the file 
    long len_text = j + q + len_pattern - 1; //when to finish the chunk of file
    
   
    while ((j < len_file) && (j < len_text))
    {
        if (pattern[i] == file[j])
        {
            j++;
            i++;
        }
        if (i == len_pattern)
        {

            

            count_pattern++;
            i = table[i - 1];
        }

        else if (j < len_text && pattern[i] != file[j])
        {
            if (i != 0)
            {
                i = table[i - 1];
            }
            else
            {
                j = j + 1;
            }
        }
    }
  //critical section increment golobal sum and change status thread 
   pthread_mutex_lock(&mutex_worker);
    pthread_mutex_lock(&mutex);

    sum += count_pattern;
    pthread_mutex_unlock(&mutex);
	count_task++;
    status_thread[t2[chunk_file]] = finish;
    pthread_mutex_unlock(&mutex_worker);

    return NULL;
}

