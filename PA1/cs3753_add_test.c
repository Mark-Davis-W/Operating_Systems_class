#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  
  int sumAddress;
  syscall(334,100,11,&sumAddress);
  printf("The result of cs3753_add system call is: %d\n.",sumAddress);
  
  return 0;
}

