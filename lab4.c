#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned int ** buildTree(unsigned int *freq);
void mover();
struct Index {
    struct Index        *l,*r;
    unsigned char       *p;
    int                 len;
    unsigned int   freq;

} ;
struct Index        *first, *last,*cur,*move;
int main(int argc,char *argv[]) {
    unsigned char       buff,pushLen,push,tLen;
    unsigned int        **vBits,temp,*aFreq,total;
    int                 i,isComp;
    FILE                *fptr, *fptw;
    
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
	// Frequency cells
    aFreq = (unsigned int *) calloc(256,sizeof(unsigned int));
    for (i=0;i<256;i++) {
        aFreq[i]=0;
    }


    if(isComp) {
		temp=0;
        while(fread(&buff,1,1,fptr) == 1) {
            aFreq[buff]++;
			temp++;
            
        }
        // Write the frequencies of each character to fptw
        for(buff=0;buff<256;buff++) {
            if (aFreq[buff] > 0 && buff != 255) {
                fwrite(&buff,1,1,fptw);
                fwrite(&aFreq[buff],sizeof(unsigned int),1,fptw);
				tLen=buff;
            } else if (aFreq[buff] > 0 && buff == 255) {
                fwrite(&buff,1,1,fptw);
                fwrite(&aFreq[buff],sizeof(unsigned int),1,fptw);
                break;
            } else if (buff == 255) {
                break;
            }
        }
        fwrite(&tLen,1,1,fptw);
        fwrite(&temp,sizeof(unsigned int),1,fptw);
        // Build the Tree and return the bit vectors
        vBits = buildTree(aFreq);
				
        rewind(fptr);

        pushLen = 7;
        push=0;
        // Push bits to fptw
        while(fread(&buff,1,1,fptr) == 1) {
            for(i=0;i<vBits[buff][1];i++) {
                push |= (((vBits[buff][0] >> i) & 0x01) << pushLen);
                pushLen--;
                if(pushLen == 255) {
                    fwrite(&push,1,1,fptw);
                    pushLen=7;
                    push=0;
                }
            }
        }
		if (pushLen < 7)
        	fwrite(&push,2,1,fptw);
    // Decompress
    } else {
        while(1) {
            fread(&buff,1,1,fptr);
            fread(&temp,sizeof(unsigned int),1,fptr);
            if (aFreq[buff] == 0) {
                aFreq[buff]=temp;
            } else {
				total=temp;
                break;
            }
        }
        // Build the Tree and return the bit vectors
        vBits = buildTree(aFreq);

        pushLen = 7;
        temp = 0;
        tLen = 0;
        // Read in bits from fptr
        // use bLen and temp
        while(fread(&buff,1,1,fptr) == 1) {
            for(i=7;i>=0;i--) {
                // Store bit in temp
                temp |= ((0x01 & (buff >> i)) << tLen);
                tLen++;
                // Compare bLen, then temp to all vBits
                for(push=0;push<256;push++) {
                    if (temp == vBits[push][0] && tLen == vBits[push][1]) {
                        fwrite(&push,1,1,fptw);
						total--;
                        temp=0;
                        tLen=0;
                        break;
                    } else if (push == 255) {
                        break;
                    }
                }
				if (total == 0) break;
            }
			if (total == 0) break;
        }

    }
    
    fclose(fptr);
    fclose(fptw);
    return 0;
}


// Creates the Huffman tree structure
// Returns the variant size bit vectors that represent Huffman's Alphabet
unsigned int ** buildTree(unsigned int *freq) {
    unsigned int  **bits = (unsigned int **) calloc(256,sizeof(unsigned int *));
    unsigned char       c,*np;
    int                 i;
    c=0;
    // Initialize   bits[i][0] = bit vector
    //              ...[1] = len of bit vector
    for(i=0;i<256;i++) {
        bits[i] = (unsigned int *) calloc(2,sizeof(unsigned int));
        bits[i][0] = bits[i][1]= 0;
    }
    // Initialize linked list
    first = (struct Index *) calloc(1,sizeof(struct Index));
    last = (struct Index *) calloc(1,sizeof(struct Index));
    first->l = last->r = NULL;
    first->p = last->p = NULL;
    first->freq = last->freq = 0;
    first->r = last;
    last->l = first;
    cur = last;
    // Insert all nodes
    for(i=0;i<256;i++) {
        if (freq[i] > 0) {
            cur->l = (struct Index *) calloc(1,sizeof(struct Index));
            cur->l->r = cur;
            cur = cur->l;
            cur->l = first;
            cur->p = (unsigned char *) calloc(1,sizeof(unsigned char));
            cur->p[0] = c;
            cur->len = 1;
            cur->freq = freq[i];
            first->r = cur;
			// Move current pattern to appropriate location in list
			mover();
        }
        c++;
    }
	cur = first->r;
	while(cur != last) {
		cur = cur->r;
	}
    cur = first->r;
    while (cur->r != last) {
        // Add the two lowest limb patterns and push new bits into the bits vector
        np = (unsigned char *) calloc(cur->len+cur->r->len,sizeof(unsigned char));
        for(i=0;i<(cur->len+cur->r->len);i++) {
            if (i < cur->len) {
                np[i] = cur->p[i];
                bits[np[i]][1]++; 
                bits[np[i]][0] <<= 1;
            } else {
                np[i] = cur->r->p[i-cur->len];
                bits[np[i]][0] <<= 1;
                bits[np[i]][0] |= 0x1;
				bits[np[i]][1]++;
            }
        }
        // Free old pattern pointers and set new pattern
        free(cur->p);
        free(cur->r->p);
        cur->p=np;
        // Update length of pattern and frequency of pattern
        cur->len += cur->r->len;
        cur->freq += cur->r->freq;
        // Remove unused index in the linked list
        cur->r = cur->r->r;
        free(cur->r->l);
        cur->r->l = cur;
        // Move current pattern to appropriate location in list
        mover();
    }
    free(cur->p);
    free(cur);
    return bits;
}

// Compares an entry to the all bit vectors
void mover() {
	move = cur->r;
	while(move != last) {
	    if (cur->freq < move->freq) {
	        break;
	    } else if (cur->freq == move->freq) {
	        if(cur->len < move->len) {
	            break;
	        } else if (cur->len == move->len) {
	            if(cur->p[0] < move->p[0]) {
	                break;
	            }
	        }
	    }
		move = move->r;
	}
	if(move != cur->r) {
	    first->r = cur->r;
	    cur->r->l = first;
	    move->l->r = cur;
	    cur->l = move->l;
	    move->l = cur;
	    cur->r = move;
	    cur = first->r;
	}
}