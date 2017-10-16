#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <iostream.h>
#include <math.h>
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")
#define MIN(A,B)	((A) < (B) ? (A) : (B))
#define MAX(A,B)	((A) > (B) ? (A) : (B))
#define cnt 100
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void grayimage(Bitmap *bitmap, BYTE **image);
void Save2File(char* fname, BYTE **image, CLSID *pClsid);
void closingImage(BYTE **image,CLSID *pClsid);
void thinningImage(BYTE **image,CLSID *pClsid);
void convolve (BYTE **in, BYTE **out, int flag, int **mask, int mask_h, int mask_w);
int iround (double u) {return int (floor (.5+u));} 
int w, h;

int main ()
{

	//read image and convert it to gray image which 8-bit format

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    CLSID  clsid;
	int y, x;	
	char file_name[120];
	WCHAR fn[120];

	cout<<"Please enter the filename you want to do thinning:";
	cin>>file_name;

	mbstowcs(fn,file_name,120);

	Bitmap* bitmap = new Bitmap(fn);	
	GetEncoderClsid(L"image/bmp", &clsid);
 
	h = bitmap->GetHeight();
	w = bitmap->GetWidth();

	//make image from color to gray
	BYTE **image = new BYTE* [h];
	for (y=0; y<h; y++)
	{
		image[y] = new BYTE[w];
		for (x=0; x<w; x++)
			image[y][x] = 0;
	}
	grayimage(bitmap, image);


	//convert to black-white image 
	int threshold = 150;
	for (y=0; y<h; y++)
		for(x=0; x<w; x++)
		{
			if (image[y][x]>threshold)
				image[y][x] = 0; // This is for white color;
			else image[y][x] = 1; // This is for Black color;
		}

	Save2File("Black_White.jpg",		image,		&clsid);

//	closingImage(image,&clsid);
	thinningImage(image,&clsid);

	Save2File("thinned.jpg",		image,		&clsid);

	delete bitmap;

	for (y=0; y<h; y++)
		delete [] image[y];
	delete [] image;

	return 0;
}

void closingImage(BYTE **image , CLSID *clsid)
{
	int Bit_shift = 0;
	int x,y;

	//copy image to image1
	BYTE **image1 = new BYTE* [h];
	for (y=0; y<h; y++)
	{
		image1[y] = new BYTE[w];
		for (x=0; x<w; x++)
			image1[y][x] = 0;
	}

	for (x=1; x<w-1; x++)   // dilation operation.
		for (y=1; y<h-1; y++ )
		{
			if (image[y][x] == 0)
			{
				// 8-Neighbor filter: 
				/*
				Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
							(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
							(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
							(image[y][x-1] << 1) | (image[y-1][x-1] );*/
				Bit_shift = (image[y-1][x] + image[y-1][x+1] +
							image[y][x+1] + image[y+1][x+1]  +
							image[y+1][x] + image[y+1][x-1]  +
							image[y][x-1] + image[y-1][x-1] );

				// 4-Neighbor filter:
				//Bit_shift = (image[y-1][x] << 7) | (image[y][x+1] << 5) |
				//			(image[y+1][x] << 3) | (image[y][x-1] << 1);
							
				image1[y][x] = Bit_shift > 1 ? 1 : 0;
			} else image1[y][x] = image[y][x];
		}

	for (x=1; x<w-1; x++)   // dilation operation.
		for (y=1; y<h-1; y++ )
		{
			image[y][x] = image1[y][x];
			image1[y][x] = 0;
		}


	Save2File("dilation.jpg",		image,		clsid);

	
	for (x=1; x<w-1; x++)   // erosion operation.
		for (y=1; y<h-1; y++ )
		{
			if (image[y][x] == 1)
			{
			    // 8-Neighbor filter: 
				Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
							(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
							(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
							(image[y][x-1] << 1) | (image[y-1][x-1] );

				// 4-Neighbor filter:
				//Bit_shift = (image[y-1][x] << 7) | (image[y][x+1] << 5) |
				//			(image[y+1][x] << 3) | (image[y][x-1] << 1);
				//image1[y][x] = Bit_shift == 0xAA ? 1 : 0;

				image1[y][x] = Bit_shift == 0xFF ? 1 : 0;
			} else image1[y][x] = image[y][x];
		}

	Save2File("erosion.jpg",		image1,		clsid);

	for (x=1; x<w-1; x++)   // erosion operation.
		for (y=1; y<h-1; y++ )
		{
			image[y][x] = image1[y][x];
			image1[y][x] = 0;
		}


	
	for (y=0; y<h; y++)
		delete [] image1[y];
	delete [] image1;

}

void thinningImage(BYTE **image , CLSID *clsid)
{
	BYTE THINNING_LUT[256] = {  1,1,1,0,1,1,0,0,1,1,0,0,0,1,0,0,
								1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, /*32*/
								1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, 
								0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, /*64*/
								1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
								1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
								0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,
								0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, /*128*/
								1,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,
								1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,
								1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0};


	int x, y;
	BYTE Bit_shift = 0;
	int is_done = 0;

	while (is_done == 0)
	{
		is_done = 1;  // To indicate no things to be changed.

		// First try to do north and south.
		// Starting from 1, we don't need to worry about the bounder.
		// Need to finish another part, i.e. the bouder situation.
		for (x=1; x<w-1; x++)   // thinning from north.
			for (y=1; y<h-1; y++ )
			{
				if (image[y][x] == 1)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != 1)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from north.
					for (y = y + 1; y < h-1 && image[y][x] ==1; ++y);
				}// end of if image[y][x] == 1;

			}// end of for.
		
		for (x=1; x<w-1; x++)   // thinning from south.
			for (y=h-1; y>1; y-- )
			{
				if (image[y][x] == 1)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != 1)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from south.
					for (y = y - 1; y > 1 && image[y][x] ==1; --y);
				}// end of if image[y][x] == 1;

			}// end of for.

		// Now try to do west and east.
		// Starting from 1, we don't need to worry about the bounder.
		// Need to finish another part, i.e. the bouder situation.
		for (y=1; y<h-1; y++ )   // thinning from west.
			for (x=1; x<w-1; x++)
			{
				if (image[y][x] == 1)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != 1)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from west.
					for (x = x + 1; x < w-1 && image[y][x] ==1; ++x);
				}// end of if image[y][x] == 1;

			}// end of for.

		for (y=1; y<h-1; y++ )    // thinning from east.
			for (x=w-1; x>1; x--)
			{
				if (image[y][x] == 1)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != 1)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from east.
					for (x = x - 1; x > 1 && image[y][x] ==1; --x);
				}// end of if image[y][x] == 1;

			}// end of for.

	} // end of while;
}

void Save2File(char* fname, BYTE **image, CLSID *clsid)
{
	int x, y;
	WCHAR wstr [100];
    Status stat;
 
    mbstowcs(wstr, fname, 100);
	Bitmap * newbitmap = new Bitmap(w,h);
 	for (y=0; y<h; y++)
	for (x=0; x<w; x++)
		newbitmap->SetPixel(x,y,Color((1-image[y][x])*255,(1-image[y][x])*255,(1-image[y][x])*255));

    stat = newbitmap->Save(wstr, clsid, NULL);

    if(stat == Ok)
		printf("file saved successfully.\n");
	else 
		printf("file saved failed .\n");
}

void grayimage(Bitmap *bitmap, BYTE **image)
{	

	int x, y;
	Color i_color;

    for (y=0; y<h; y++)
		for (x=0; x<w; x++)
		{
			bitmap->GetPixel(x,y,&i_color);
			image[y][x] = (BYTE)(i_color.GetR()*0.299 + i_color.GetG()*0.587 + i_color.GetB()*0.114);
		}
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0; // number of image encoders
    UINT size = 0; // size of the image encoder array in bytes
	
    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
    return -1; // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
    return -1; // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j; // Success
        }
    }

    free(pImageCodecInfo);
    return -1; // Failure
}
