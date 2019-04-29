#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int lookup();
int decomp();
unsigned char					**dict;
unsigned short int  			dLen[65536];
int				dTotal;

int main(int argc,char *argv[]) {
    FILE                *fptr,*fptw;
    unsigned char       p[4000],c,pc[4000],buff;
	unsigned short int	pLoc,pcLoc;
    int			        i,pLen,found,isComp;
	
    // Initial check
    if (argc != 3) {
        printf("Usage: lab3 [Read File] [Write File]\n");
        exit(0);
    }
	fptr = fopen(argv[1],"rb");
    if (fptr == NULL) {
        printf("Unable to open %s for read.\nExiting now.\n",argv[1]);
        exit(0);
    }
	fptw = fopen(argv[2],"w");
    if (fptw == NULL) {
        printf("Unable to open %s for write.\nExiting now.\n",argv[2]);
        exit(0);
    }
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
    // Initialize the first 256 patterns
    dict = (unsigned char **) calloc(65536,sizeof(unsigned char *));
    for(i=0;i<256;i++) {
        dict[i] = (unsigned char *) calloc(1,sizeof(unsigned char));
        dict[i][0]=i;
        dLen[i]=1;
    }
    dTotal=256;
    pLen = 0;
	maxLen=1;
	loops=0;
	if (!isComp) {
        decomp(fptr,fptw);
    } else {
		while(fread(&c,1,1,fptr) == 1) {
		    pc[pLen]=c;
			found=lookup(pc,pLen+1,&pcLoc);
		    if(found != -1){
		        p[pLen++]=c;
		    } else {
				lookup(p,pLen,&pLoc);
		        fwrite(&pLoc,1,2,fptw);
		        dict[dTotal]=(unsigned char *) calloc(pLen+1,sizeof(unsigned char));
		        for(i=0;i<pLen+1;i++) {
		            dict[dTotal][i]=pc[i];
		        }
		        dLen[dTotal]=pLen+1;
		        dTotal++;
				for(i=0;i<pLen;i++) {
					p[i]='\0';
					pc[i]='\0';	
				}
				pc[i]='\0';
		        p[0]=c;
				pc[0]=p[0];
		        pLen=1;         
		    }   
		}
		lookup(p,pLen,&pLoc);
		fwrite(&pLoc,1,2,fptw);
    }
	fclose(fptw);
	fclose(fptr);
    return 0;
}

int lookup(unsigned char *p,int pLen,unsigned short int *in) {
	int	i,j,check;
	
	for(i=0;i<dTotal;i++) {
        if(pLen == dLen[i]) {
			// If check remains 0, a match was found
            check=0;
            for(j=0;j<pLen;j++) {
	            if(p[j] != dict[i][j]) {
	                check=1;
					break;
	            }
            }
            if (check != 1) {
                *in=i;
                return 1;
            }
        }
    }
	return -1;
}

// Given location, determine value
int decomp(FILE *fptr,FILE *fptw) {
	unsigned short int	cLoc,pLoc;
    int			        i,found;

	fread(&pLoc,2,1,fptr);
	fwrite(dict[pLoc],1,1,fptw);
	while(fread(&cLoc,2,1,fptr) == 1) {
		
        if (cLoc < dTotal) {
            fwrite(dict[cLoc],dLen[cLoc],1,fptw);
            dict[dTotal]=(unsigned char *) calloc(dLen[pLoc]+1,sizeof(unsigned char));
            for(i=0;i<=dLen[pLoc];i++) {
                if(i < dLen[pLoc]) {
                    dict[dTotal][i]=dict[pLoc][i];
                } else {
                    dict[dTotal][i]=dict[cLoc][0];
                }
            }
            dLen[dTotal]=dLen[pLoc]+1;
            dTotal++;
        } else {
            dict[dTotal]=(unsigned char *) calloc(dLen[pLoc]+1,sizeof(unsigned char));
            for(i=0;i<=dLen[pLoc];i++) {
                if(i != dLen[pLoc]) {
                    dict[dTotal][i]=dict[pLoc][i];
                } else {
                    dict[dTotal][i]=dict[pLoc][0];
                }
            }
            dLen[dTotal]=dLen[pLoc]+1;
            fwrite(dict[dTotal],dLen[dTotal],1,fptw);
            dTotal++;
        }  
        pLoc=cLoc;  
    }
    return 1;
}
