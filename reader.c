
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>



char * read_file(char * name_file){
 char * text= NULL;
    long file_size;


    FILE *ptr = fopen(name_file, "r"); 
    if(ptr == NULL){
       
        printf("no such file");
        return 0;
    }
    else{

        fseek(ptr, 0, SEEK_END);/* ptr at the end of the file*/
        file_size = ftell(ptr);
        rewind(ptr); 
        text = (char *)malloc(sizeof(char)*(file_size+1));
        if(text == NULL){
            printf("out of memory"); 
            return 0;
        }
        /*read it */
        long read_size;
        read_size = fread(text, sizeof(char), (size_t) file_size, ptr);
        text[file_size] = '\0';
        if(file_size != read_size){
            free(text);
            text = NULL;
        }
        
        fclose(ptr);
        return text;


    }
}