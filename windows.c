#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

LRESULT CALLBACK EventProcessor (HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{ return(DefWindowProc(hWnd,uMsg,wParam,lParam)); }

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,int nCmdShow)
{
WNDCLASS			wc;
HWND				WindowHandle;
BITMAPINFO			*bm_info;
HDC					hDC;
int					ROWS,COLS,bytes,fSize,dSize,i,j;
unsigned char		*displaydata,*filedata;
int					isColor=0;
char				head[10];
FILE				*fpt;
BITMAPINFOHEADER	bm_head;

if(__argv[1] == NULL) {
	MessageBox(NULL,_T("Invalid input: Please input a filename"),_T(""),MB_OK | MB_APPLMODAL);
	exit(0);
}
fpt = fopen(__argv[1],"rb");
if (fpt == NULL) {
	MessageBox(NULL,_T("Invalid file name: \"%s\"\n"),_T(""),MB_OK | MB_APPLMODAL);
	exit(0);
}
fscanf(fpt,"%s %d %d %d ",head,&COLS,&ROWS,&bytes);



if ((!(strcmp(head,"P6")) || !(strcmp(head,"P5")))) {
	if(strcmp(head,"P6")) {
		fSize = COLS*ROWS;
		dSize = fSize;
		isColor = 0;
	} else {
		fSize = 3*COLS*ROWS;    
		dSize = 2*COLS*ROWS;
		isColor = 1;
	}
} else {
	printf("Invalid file type: \"%s\". Please select a .ppm with file type P5 or P6",head);
	exit(0);
}
filedata = (unsigned char *) calloc(fSize,sizeof(unsigned char));
displaydata = (unsigned char *) calloc(dSize,sizeof(unsigned char));

fread(filedata,1,fSize,fpt);
fclose(fpt);

if (isColor) {
	for (i=0;i<ROWS;i++) {
		for (j=0;j<COLS;j++) {
			displaydata[2*((ROWS-i-1)*COLS+j)+1] = (((filedata[3*(i*COLS+j)] >> 1) & 0x7c)) | ((filedata[3*(i*COLS+j)+1] >> 6) & 0x03);
			displaydata[2*((ROWS-i-1)*COLS+j)] = ((filedata[3*(i*COLS+j)+1] << 2) & 0xe0) | ((filedata[3*(i*COLS+j)+2] >> 3) & 0x1f);
		}
	}
	
} else {
	displaydata = filedata;
}



wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)EventProcessor;
wc.cbClsExtra=wc.cbWndExtra=0;
wc.hInstance=hInstance;
wc.hIcon=NULL;
wc.lpszMenuName=NULL;
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszClassName= "Image Window Class";
if (RegisterClass(&wc) == 0)
  exit(0);

WindowHandle=CreateWindow("Image Window Class",_T("ECE468 Lab1"),
						  WS_OVERLAPPEDWINDOW,
						  10,10,COLS,ROWS,
						  NULL,NULL,hInstance,NULL);
if (WindowHandle == NULL)
  {
  MessageBox(NULL,_T("No window"),_T("Try again"),MB_OK | MB_APPLMODAL);
  exit(0);
  }
ShowWindow (WindowHandle, SW_SHOWNORMAL);

bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
hDC=GetDC(WindowHandle);

bm_head.biSize = sizeof(BITMAPINFOHEADER);
bm_head.biWidth = COLS;
bm_head.biPlanes = 1;
bm_head.biCompression = BI_RGB;
bm_head.biSizeImage = 0;
bm_head.biXPelsPerMeter = 0;
bm_head.biYPelsPerMeter = 0;
bm_head.biClrUsed = 0;
bm_head.biClrImportant = 0;

if ( !isColor) {
	bm_head.biHeight = -ROWS;
	bm_head.biBitCount = 8;
} else {
	bm_head.biHeight = ROWS;
	bm_head.biBitCount = 16;
}
bm_info->bmiHeader=bm_head;

for (i=0; i<256; i++)	/* colormap */
  {
  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
  bm_info->bmiColors[i].rgbReserved=0;
  } 

SetDIBitsToDevice(hDC,0,0,COLS,ROWS,0,0,
			  0, /* first scan line */
			  COLS, /* number of scan lines */
			  displaydata,bm_info,DIB_RGB_COLORS);
ReleaseDC(WindowHandle,hDC);
free(bm_info);
MessageBox(NULL,_T("Press OK to continue"),_T(""),MB_OK | MB_APPLMODAL);



}
