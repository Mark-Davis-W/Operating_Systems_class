#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

#define maxSize 100

int main(int argc, char *argv[]) { 
  
  if(argc<2)
    {
      printf("%s error: missing filename\n", argv[0]);
      exit(1);
    }
  if(argc>2)
    {
      printf("%s error: too many parameters\n", argv[0]);
      exit(1);
    } 
  
  int offset;
  int whence;
  int bytes;
  FILE *f = fopen(argv[1],"r+");
  int file = fileno(f);
  char *buffer;
  char choice = 'a';
  //int flag = 0;
  
  if(!f)
  {
    printf("%s error: invalid filename\n", argv[1]);
    exit(1);
  }
  
  while(1)
  {
    
    fprintf(stdout,"Option (r for read, w for write, s for seek): ");
    //fflush(stdout);
    //printf("This is your choice: %c\n",choice);
    
    if(scanf(" %c", &choice) != EOF)
    {
	switch(choice){
	  case 'r':
	  case 'R':
	    printf("Enter the number of bytes you want to read: ");
	    //fflush(stdout);
	    if(scanf("%d",&bytes) != EOF)
	    {	    
	       getchar();
	       
	       buffer = (char*)malloc(maxSize);
	       memset(buffer,'\0',maxSize);
	       if(read(file,buffer,maxSize) == -1)
	       {
	         fprintf(stderr,"%s read failed.\n",argv[0]);
	         exit(1);
	       }
	    
	       //printf("Your data: %s\n",buffer);
	       printf("%s\n",buffer);
	       memset(buffer,'\0',maxSize);
	       free(buffer);
	     }
	     break;
	
	  case 'w':
	  case 'W':
	    buffer = (char*)malloc(maxSize);
	    fprintf(stdout,"Enter the data you want to write: ");
	    //fflush(stdout);
	    getchar();
	    memset(buffer,'\0',maxSize);
	    if(fgets(buffer, maxSize, stdin) != NULL)
	    {
	      buffer[strcspn(buffer,"\n")] = 0;
	    
	      //fputs(buffer,f);
	      if(write(file,buffer,strlen(buffer)) == -1)
	      {
	        fprintf(stderr,"%s write failed.\n",argv[0] );
	        exit(1);
	      }
	      //fprintf(f,"%s",buffer);
	    }
	    memset(buffer,'\0',maxSize);
	    free(buffer);
	    
	    break;

	  case 's':
	  case 'S':
	    fprintf(stdout,"Enter an offset value: ");
	    //fflush(stdout);
	    if(scanf("%d", &offset) != EOF)
	    {  
	      getchar();
	  
	      fprintf(stdout,"Enter a value for whence (0 for SEEK_SET, 1 for SEEK_CUR, 2 for SEEK_END): ");
	      //fflush(stdout);
	      if(scanf("%d", &whence) != EOF)
		{
		  getchar();

		  if(lseek(file, offset, whence) == -1)
		  {
		    fprintf(stderr,"%s seek failed.\n",argv[0]);
		    exit(1);
		  }
	  	}
		break;
	     }
	    break;
	}
    }
    else {
      //printf("Caught Ctrl+D exiting program.\n");
      fclose(f);
      exit(0);
    }
  }
  
  fclose(f);
  return 0;
 }
