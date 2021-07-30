#include<sys/syscall.h>
#include<unistd.h>
#include "slob_wrapper.h"
#define __NR_slob_get_total_alloc_mem 436
#define __NR_slob_get_total_free_mem 437

long get_total_alloc_mem(void){
    return(syscall(__NR_slob_get_total_alloc_mem));
}

long get_total_free_mem(void){
    return(syscall(__NR_slob_get_total_free_mem));
}
