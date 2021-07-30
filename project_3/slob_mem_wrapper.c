#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include "slob_mem_wrapper.h"
#define __NR_slob_kmalloc 438
#define __NR_slob_kfree 439

void * slob_kmalloc(size_t size){
    return(syscall(__NR_slob_kmalloc, size));
}

void slob_kfree(const void * addr){
    syscall(__NR_slob_kfree, addr);
    return;
}
