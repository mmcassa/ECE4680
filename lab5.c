#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define PI      3.141592654
#define ELMV    "element vertex "
#define ELMF    "element face "
#define ENDH    "end_head"
#define MAXLEN  100
#define ROWS    256
#define COLS    256

void pmat(double *m);
void sca_mult(double s,double *m);
void fmat(double *m);
void mat_dup(double *a,double *c);
void mat_add(double *a, double *b,int s,double *c);
void mat_crx(double *a, double *b,double *c);
double mat_a(double *m);
double mat_dot(double *a, double *b);

int main(int argc,char *argv[]) {
    // Initialize & Instantiate Variables
    FILE		    *fptr,*fptw;
    int             i,j,k,l,space,skip,count,c2;
    double          min[3],max[3],e,a,n,d,D,q;
    double          rad[3],rotate[3];
    double          *left,*right,*top,*bottom,*topleft,*rl,*bt;
    double          **verts;
    char            buff,line[MAXLEN],numBuff[20],*str;
    int             zbuff,cVert, cFace,**faces;
    double          *camera,*up,*mTemp,*mTemp2,*mTemp3,*mTemp4,*mCenter;
    double          *isec;
    double          *image,**v,*v10,*v20,*abc,*dot;
    unsigned char   ppm[ROWS][COLS]=0;
    char            *keys[3];
	count = 0;
    camera = (double *) calloc(3,sizeof(double));
    up = (double *) calloc(3,sizeof(double));
    left = (double *) calloc(3,sizeof(double));
    right = (double *) calloc(3,sizeof(double));
    top = (double *) calloc(3,sizeof(double));
    bottom = (double *) calloc(3,sizeof(double));
    topleft = (double *) calloc(3,sizeof(double));
    rl = (double *) calloc(3,sizeof(double));
    bt = (double *) calloc(3,sizeof(double));
    v10 = (double *) calloc(3,sizeof(double));
    v20 =  (double *) calloc(3,sizeof(double));
    mTemp = (double *) calloc(3,sizeof(double));
    mTemp2 = (double *) calloc(3,sizeof(double));
    mTemp3 = (double *) calloc(3,sizeof(double));
    mTemp4 = (double *) calloc(3,sizeof(double));
    mCenter= (double *) calloc(3,sizeof(double));
    isec = (double *) calloc(3,sizeof(double));
    image = (double *) calloc(3,sizeof(double));
    abc = (double *) calloc(3,sizeof(double));
    v = (double **) calloc(3,sizeof(double *));
    dot = (double *) calloc(3,sizeof(double));
    for(i=0;i<3;i++) {
        v[i]= (double *) calloc(3,sizeof(double));
    }
	for (i=0;i<3;i++) {
		keys[i] = (char *) calloc(15,sizeof(char));
	}
	keys[0] = ELMV;
	keys[1] = ELMF;
	keys[2] = ENDH;
	

    // Test input is correct
    if (argc != 6) {
        printf("Usage: lab5 [Read File] [X Rotation] [Y Rotation] [Z Rotation] [Write File]\n");
        exit(0);
    }
	fptr = fopen(argv[1],"rb");
    if (fptr == NULL) {
        printf("Unable to open %s for read.\nExiting now.\n",argv[1]);
        exit(0);
    }
    fptw = fopen(argv[5],"w");
    if (fptw == NULL) {
        printf("Unable to open %s for write.\nExiting now.\n",argv[2]);
        exit(0);
    }
    fwrite("P5 256 256 255\n", sizeof(unsigned char), 15, fptw);
    // Parse PLY file header data
    // j =0 ply test, j = 1 element vertex, j = 2 element face, j = 3 end_head
    // nevermind I'm not doing the ply test.
    space = i = j = 0;
    while(fgets(line,MAXLEN,fptr) != NULL) {
        if (strstr(line,keys[j]) != NULL) {
            if (j != 2) {
                for (i=strlen(keys[j]);line[i]!='\n';i++) {
                    numBuff[i-strlen(keys[j])] = line[i];
                }
                numBuff[i-strlen(keys[j])] = '\n';
                if (j == 0) cVert = atoi(numBuff);
                else cFace = atoi(numBuff);
            }
            else break;
            j++;
        }
    }

    // Calloc space for verticies and faces
    verts = (double **) calloc(cVert,sizeof(double *));
    faces = (int **) calloc(cFace,sizeof(int *));
    // Get all verticies    
    for(i=0;i<cVert;i++) {
        verts[i] = (double *) calloc(3,sizeof(double));
        fgets(line,MAXLEN,fptr);
        space = 0;
        j = 0;
        while(space != 3) {
            verts[i][space]= strtod(&line[j],&str);
            if (i==0 || verts[i][space] > max[space]) {
                max[space] = verts[i][space];
            } 
            if (i==0 || verts[i][space] < min[space]) {
                min[space] = verts[i][space];
            }
            while(&line[j] != str) { j++;}
			space++; 
        }
    }
    mCenter[0] = (min[0]+max[0])/2;
    mCenter[1] = (min[1]+max[1])/2;
    mCenter[2] = (min[2]+max[2])/2;
    e = max[0] - min[0];
    for (i=0;i<3;i++) {
        if (max[i] - min[i] > e) {
            e = max[i] - min[i];
        }
    }
    // Get all Faces
    for(i=0;i<cFace;i++) {
        faces[i] = (int *) calloc(3,sizeof(int));
		fscanf(fptr, "%d", &faces[i][0]);
		fscanf(fptr, "%d", &faces[i][0]);
		fscanf(fptr, "%d", &faces[i][1]);
		fscanf(fptr, "%d", &faces[i][2]);
    }
    fclose(fptr);

    ///////////////////////////////////////////////
    //      ALL FILE DATA RECEIVED              ///
    ///////////////////////////////////////////////


    // Convert input angles to radians
    for(i=0;i<3;i++) {
        rad[i] = strtod(argv[i+2],&str)*PI/180;
    }
    // Adjust Camera view, UP, and the bounding box indicies
    double rx[3][3] = {{1,0,0},{0,cos(rad[0]),-sin(rad[0])},{0,sin(rad[0]),cos(rad[0])}};
    double ry[3][3] = {{cos(rad[1]),0,sin(rad[1])},{0,1,0},{-sin(rad[1]),0,cos(rad[1])}};
    double rz[3][3] = {{cos(rad[2]),-sin(rad[2]),0},{sin(rad[2]),cos(rad[2]),0},{0,0,1}};
    camera[0] = 1; camera[1]= 0; camera[2] = 0;
    up[0] = 0; up[1] = 0; up[2] = 1;
    for(i=0;i<3;i++) {
        mTemp[i] = camera[0]*rx[0][i] + camera[1]*rx[1][i] + camera[2]*rx[2][i];
        mTemp2[i] = up[0]*rx[0][i] + up[1]*rx[1][i] + up[2]*rx[2][i];
    }
    for(i=0;i<3;i++) {
        camera[i] = mTemp[i];
        up[i] = mTemp2[i];
    }
    for(i=0;i<3;i++) {
        mTemp[i] = camera[0]*ry[0][i] + camera[1]*ry[1][i] + camera[2]*ry[2][i];
        mTemp2[i] = up[0]*ry[0][i] + up[1]*ry[1][i] + up[2]*ry[2][i];
    }
    for(i=0;i<3;i++) {
        camera[i] = mTemp[i];
        up[i] = mTemp2[i];
    }
    for(i=0;i<3;i++) {
        mTemp[i] = camera[0]*rz[0][i] + camera[1]*rz[1][i] + camera[2]*rz[2][i];
        mTemp2[i] = up[0]*rz[0][i] + up[1]*rz[1][i] + up[2]*rz[2][i];
    }
    for(i=0;i<3;i++) {
        camera[i] = mTemp[i];
        up[i] = mTemp2[i];
    }

    sca_mult(1.5*e,camera);
    mat_add(camera,mCenter,1,mTemp);
    for(i=0;i<3;i++) {
        camera[i] = mTemp[i];
    }
    // Determine 3D Bounding Box
    mat_add(mCenter,camera,-1,mTemp);
    mat_crx(up,mTemp,mTemp2);
    a = mat_a(mTemp2);
    sca_mult(e/2/a,mTemp2);
    mat_add(mTemp2,mCenter,1,left);
    mat_crx(mTemp,up,mTemp2);
    sca_mult(e/2/a,mTemp2);
    mat_add(mTemp2,mCenter,1,right);
    mat_dup(up,mTemp);
    sca_mult(e/2,mTemp);
    mat_add(mTemp,mCenter,1,top);
    mat_add(mTemp,left,1,topleft);
    sca_mult(-1.0,mTemp);
    mat_add(mTemp,mCenter,1,bottom);
    
    // Do the image thing
    for(i=0;i<ROWS;i++) {
        for(j=0;j<COLS;j++) {
            ppm[i][j]=0;
            mat_add(right,left,-1,rl);
            mat_add(bottom,top,-1,bt);
            zbuff=9999999;
            mat_dup(rl,mTemp);
            sca_mult(((double)j)/(COLS-1),mTemp);
            mat_add(topleft,mTemp,1,mTemp2);
            mat_dup(bt,v10);
            sca_mult(((double)i)/(ROWS-1),v10);
            mat_add(v10,mTemp2,1,image); //image set
            for(k=0;k<cFace;k++) {
                mat_add(verts[faces[k][1]],verts[faces[k][0]],-1,mTemp);
                mat_add(verts[faces[k][2]],verts[faces[k][0]],-1,mTemp2);
                mat_crx(mTemp,mTemp2,abc);
                D = -mat_dot(abc,verts[faces[k][0]]);
                n= -mat_dot(abc,camera)-D;
                mat_add(image,camera,-1,mTemp);
                d = mat_dot(abc,mTemp);
                if(d > .0001) {
                    skip = 0;
                    mat_add(image,camera,-1,mTemp);
                    sca_mult(n/d,mTemp);
                    mat_add(camera,mTemp,1,isec);
                    for(l=0;l<3;l++) {
                        mat_add(verts[faces[k][(l+2)%3]],verts[faces[k][(l)%3]],-1,v20);
                        mat_add(verts[faces[k][(l+1)%3]],verts[faces[k][(l)%3]],-1,v10);
                        mat_crx(v20,v10,mTemp3);
                        mat_add(isec,verts[faces[k][(l)%3]],-1,mTemp);
                        mat_crx(mTemp,v10,mTemp4);
                        dot[l] = mat_dot(mTemp3,mTemp4);
                        if (dot[l] < 0) {skip = 1;break;}
                    }
                    if (skip != 1  && ((n/d) <= zbuff)) {
                        zbuff = n/d;
                        ppm[i][j] = 155 + k%100;
                    }
                }
            }
        }
    }
    fwrite(&ppm,sizeof(unsigned char),ROWS*COLS,fptw);
    fclose(fptw);
    
    free(camera);
    free(up);
    free(left);
    free(right);
    free(top);
    free(bottom);
    free(topleft);
    free(rl);
    free(bt);
    free(v10);
    free(v20);
    free(mTemp);
    free(mTemp2);
    free(mTemp3);
    free(mTemp4);
    free(mCenter);
    free(isec);
    free(image);
    free(abc);
    for(i=0;i<3;i++) {
        free(v[i]);
    }
    free(v);
    free(dot);
    for(i=0;i<cVert;i++) {
        free(verts[i]);
    }
    free(verts);
    return 0;
}

void pmat(double *m) {
    printf("%lf %lf %lf\n",m[0],m[1],m[2]);
}

void mat_dup(double *a,double *c) {
    int i;
    for (i=0;i<3;i++) {
        c[i] = a[i];
    }
}

void fmat(double *m) {
    m[0]= m[1] = m[2] = 0;
}
void sca_mult(double s,double *m) {
    int j;
    for(j=0;j<3;j++) {
        m[j] = m[j]*s;
    }
}

double mat_a(double *m) {
    int i;
    double a=0;
    for(i=0;i<3;i++){
        a += m[i]*m[i];
    }
    return sqrt(a);
}

void mat_add(double *a, double *b,int s,double *c) {
    int i;
    for (i=0;i<3;i++) {
        if(s != 1) {
            c[i] = a[i]-b[i];
        } else {
            c[i] = a[i]+b[i];
        }
    }
}

void mat_crx(double *a, double *b,double *c) {
    
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
    
}

double mat_dot(double *a, double *b) {
    int i;
    double c=0;
    for(i=0;i<3;i++){
        c += a[i]*b[i];
    }
    return c;
}

