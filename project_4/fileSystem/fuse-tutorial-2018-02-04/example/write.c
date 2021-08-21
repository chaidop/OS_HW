#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){

    FILE *fp;
    char arr[50];
    fp = fopen ("storage/test.txt", "w+");
    fprintf(fp, "TESTING FILE");
    rewind(fp);
    while(fscanf(fp, "%s", arr) == 1)
	printf("\n%s\n", arr);
    fclose(fp);
    return 0;
}

