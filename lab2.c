#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int 			i,isComp;
	unsigned char	cnt,cur,buff;
	FILE    		*fptr,*fptw;	

	// Inquire to Compress or Decompress
	printf("Compress or Decompress \"%s\"?\n(C or D): ",argv[1]);
	scanf("%c",&buff);
	if (buff == 'C' || buff == 'c') {
		isComp = 1;
	} else if (buff == 'D' || buff == 'd') {
		isComp = 0;
	} else {
		printf("Invalid: Please enter 'c' or 'd'.\nExiting.\n");
		exit(0);
	}
	// Open read file
	fptr = fopen(argv[1],"rb");
	if (fptr == NULL) {
		printf("Failed to open \"%s\" for read.\n",argv[1]);
		exit(0);
	}
	// Open write file
	fptw = fopen(argv[2],"w");
	if (fptw == NULL) {
		printf("Failed to open %s for write.\n",argv[2]);
		exit(0);
	}
	
	if (isComp ==1){
		// Compress
		fread(&cur,1,1,fptr);
		cnt = 1;
		while(fread(&buff,1,1,fptr) == 1) {
			if (buff == cur && cnt < 255) {
				cnt++;
			} else {			
				fwrite(&cnt,1,1,fptw);
				fwrite(&cur,1,1,fptw);				
				cur = buff;
				cnt=1;
			}
		}
		fwrite(&cnt,1,1,fptw);
		fwrite(&cur,1,1,fptw);
	} else {
		// Decompress
		while(fread(&buff,1,1,fptr) == 1) {
			fread(&cur,1,1,fptr);
			for(i=0;i<buff;i++) {
				fwrite(&cur,1,1,fptw);
			}
		}
	}
	printf("Done.\n");
	// Close Files
	fclose(fptr);
	fclose(fptw);
	return 0;
}
