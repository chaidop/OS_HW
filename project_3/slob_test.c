#include "slob_wrapper.h"
#include "slob_mem_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
long al, fr, temp_al, temp_fr;
int * temp0, *temp1, *temp2,* temp3, *temp4, *temp5,* temp6,* temp7, *temp8, *temp9;

        temp0 = (int*)slob_kmalloc(sizeof(int)*1000000);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp1 = (int*)slob_kmalloc(sizeof(int)*10000);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp2 = (int*)slob_kmalloc(sizeof(int)*50000);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp3 = (int*)slob_kmalloc(sizeof(int)*5500);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();

        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp4 = (int*)slob_kmalloc(sizeof(int)*1000);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp5 = (int*)slob_kmalloc(sizeof(int)*10000);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp6 = (int*)slob_kmalloc(sizeof(int)*30000);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp7 = (int*)slob_kmalloc(sizeof(int)*9999);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp8 = (int*)slob_kmalloc(sizeof(int)*1000);
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        temp9 = (int*)slob_kmalloc(sizeof(int)*10000);
	if(!temp0)
		printf("WTF\n");
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("=========\n\nALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);
        
        slob_kfree(temp0);
        if(!temp0)
            printf("WTF\n");
        slob_kfree(temp1);
        slob_kfree(temp2);
        slob_kfree(temp3);
        slob_kfree(temp4);
        slob_kfree(temp5);
        slob_kfree(temp6);
        slob_kfree(temp7);
        slob_kfree(temp8);
        slob_kfree(temp9);
        
        al = get_total_alloc_mem();
        fr = get_total_free_mem();
        printf("ALLOCATED: %ld\n",al);
        printf("FREE: %ld\n",fr);

    return 0;
}
