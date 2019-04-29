#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
Display			*Monitor;
Window			ImageWindow;
GC				ImageGC;
XWindowAttributes		Atts;
XImage			*Picture;
int				ROWS,COLS,bytes,fSize,i;
unsigned char		*displaydata,*filedata;
bool            isColor;
char            head[10];
FILE            *fpt;

fpt = fopen(argv[1],"rb");
if (fpt == NULL) {
	printf("Invalid file name: \"%s\"\n",argv[1]);
	exit(0);
}
fscanf(fpt,"%s %d %d %d ",head,&COLS,&ROWS,&bytes);



if ((!(strcmp(head,"P6")) || !(strcmp(head,"P5")))) {
	if(strcmp(head,"P6")) {
		fSize = COLS*ROWS;
		isColor = 0;
	} else {
		fSize = 3*COLS*ROWS;    
		isColor = 1;
	}
} else {
	printf("Invalid file type: \"%s\". Please select a .ppm with file type P5 or P6",head);
	exit(0);
}

filedata = (unsigned char *) calloc(fSize,sizeof(unsigned char));
displaydata = (unsigned char *) calloc(2*COLS*ROWS,sizeof(unsigned char));

fread(filedata,1,fSize,fpt);
fclose(fpt);

for (i=0;i<ROWS*COLS;i++) {
	if (!isColor) {
		displaydata[2*i+1] = (filedata[i] & 0xf8) | ((filedata[i] >> 5) & 0x07);
		displaydata[2*i] = ((filedata[i] << 3) & 0xe0) | ((filedata[i] >> 3) & 0x1f);
	} else {
		displaydata[2*i+1] = (filedata[3*i] & 0xf8) | ((filedata[3*i+1] >> 5) & 0x07);
		displaydata[2*i] = ((filedata[3*i+1] << 3) & 0xe0) | ((filedata[3*i+2] >> 3) & 0x1f);
	}
}


Monitor=XOpenDisplay(NULL);
if (Monitor == NULL)
  {
  printf("Unable to open graphics display\n");
  exit(0);
  }

ImageWindow=XCreateSimpleWindow(Monitor,RootWindow(Monitor,0),
	50,10,		/* x,y on screen */
	COLS,ROWS,	/* width, height */
	2, 		/* border width */
	BlackPixel(Monitor,0),
	WhitePixel(Monitor,0));

ImageGC=XCreateGC(Monitor,ImageWindow,0,NULL);

XMapWindow(Monitor,ImageWindow);
XFlush(Monitor);
while(1)
  {
  XGetWindowAttributes(Monitor,ImageWindow,&Atts);
  if (Atts.map_state == IsViewable /* 2 */)
    break;
  }

Picture=XCreateImage(Monitor,DefaultVisual(Monitor,0),
		DefaultDepth(Monitor,0),
		ZPixmap,	/* format */
		0,		/* offset */
		displaydata,/* the data */
		COLS,ROWS,	/* size of the image data */
		16,		/* pixel quantum (8, 16 or 32) */
		0);		/* bytes per line (0 causes compute) */

XPutImage(Monitor,ImageWindow,ImageGC,Picture,
		0,0,0,0,	/* src x,y and dest x,y offsets */
		COLS,ROWS);	/* size of the image data */

XFlush(Monitor);

sleep(5);
XCloseDisplay(Monitor);
free(displaydata);
free(filedata);

return 0;

}

