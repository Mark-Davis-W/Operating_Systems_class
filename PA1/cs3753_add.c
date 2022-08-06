#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

asmlinkage long sys_cs3753_add(int n1, int n2, int *ptr){
  printk(KERN_INFO "Inside %s. First number is: %d\nSecond number is: %d.\n",__FUNCTION__, n1, n2);
  int sum = n1 + n2;
  
  printk(KERN_INFO "The numbers added together is: %d.\n", sum);
  copy_to_user(ptr,&sum,sizeof(int));
  return 0;
}
