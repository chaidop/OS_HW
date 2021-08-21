#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){

    FILE *fp1, *fp2, *fp3;
    char buffer[8192];
    char fpath[30];
    struct stat sb;
    strcpy(fpath, "storage/test_out2");

    // Write file
    fp1 = fopen (fpath, "w+");
    for (int i = 0; i < 8192; i++) {
        /* write to file using fputc() function */
        fputc('a', fp1);
    }

    rewind(fp1);

    // Read file
    size_t num_read = (fread(buffer, sizeof(char), 100, fp1));
    buffer[num_read] = '\0';
    lstat(fpath, &sb);  // Get size of file
    printf("***********SECOND-FILE**********\n");
    printf("\nThis file contains %ld bytes of 'a'.\n", sb.st_size);
    printf("%s\n\n", buffer);
    fclose(fp1);

    return 0;
}
