#include "VCProject.h"

ofstream fout("coord.txt");

int main ()
{

	//read image and convert it to gray image which 8-bit format

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    CLSID  clsid;
	GetEncoderClsid(L"image/bmp", &clsid);

	BYTE **ppt_image = NULL, **mimio_image = NULL, **mimio_scaled_image = NULL;
	Bitmap *mimio_scaled = NULL, *mimio_matching = NULL, *ppt_matched = NULL;

	double *params = new double[20]; // Used for transfer parameters between functions
	

	Bitmap* bitmap_ppt = new Bitmap(L"image\\pptonlynote.bmp");	
 	set_image(ppt_image,bitmap_ppt);
	thinningImage(ppt_image, bitmap_ppt, &clsid);
	Save2File("image\\ppt_thinned.bmp", ppt_image, bitmap_ppt, &clsid);

	Bitmap* bitmap_mimio = new Bitmap(L"image\\mimio.bmp");
	set_image(mimio_image,bitmap_mimio);
	thinningImage(mimio_image,bitmap_mimio,&clsid);
	Save2File("image\\mimio_thinned.bmp",mimio_image,bitmap_mimio,&clsid);

	//Scaling and translating the mimio image.
	scale_translate(ppt_image, mimio_image, bitmap_ppt, bitmap_mimio, mimio_scaled,params);
	Save2File("image\\mimio_scaled.bmp",NULL,mimio_scaled,&clsid);

	//Matching the scaled mimio image with the ppt image.
	set_image(mimio_scaled_image, mimio_scaled);
	get_image(ppt_image,bitmap_ppt);
	matching(ppt_image,mimio_scaled_image,bitmap_ppt,mimio_scaled, mimio_matching, ppt_matched, params);
	Save2File("image\\mimio_matching.bmp",mimio_scaled_image,mimio_scaled,&clsid);
	Save2File("image\\ppt_matched.bmp",ppt_image,bitmap_ppt,&clsid);

	
	//Show the matched mimio and ppt-video image
	Bitmap *overlap_bitmap;
	set_image(mimio_image,bitmap_mimio);
	set_image(ppt_image,bitmap_ppt);
	overlap(ppt_image, mimio_image,params,overlap_bitmap);
	Save2File("image\\overlap.bmp",NULL,overlap_bitmap,&clsid);
	//End of show
	

	Bitmap* bitmap_ppt_orig = new Bitmap(L"image\\ppt_1.jpg");
	Bitmap* ppt_mimio_registered;
	transformation(ppt_mimio_registered,bitmap_mimio,bitmap_ppt_orig,params);
	Save2File("image\\ppt_mimio_registered.bmp",NULL,ppt_mimio_registered,&clsid);


	fout.close();
	delete [] params;
	delete bitmap_ppt;
	delete bitmap_mimio;
	delete [] ppt_image[0];
	delete [] ppt_image;
	delete [] mimio_image[0];
	delete [] mimio_image;

	return 0;
}

void get_image(BYTE **image, Bitmap *bitmap)
{

	int h = bitmap->GetHeight();
	int w = bitmap->GetWidth();
	int x,y;

	grayimage(bitmap, image);

	//convert to black-white image 
	int threshold = 150;
	for (y=0; y<h; y++)
		for(x=0; x<w; x++)
		{
			if (image[y][x]>threshold)
				image[y][x] = WHITE; // This is for white color;
			else image[y][x] = BLACK; // This is for Black color;
		}

}

void set_image(BYTE **&image, Bitmap *bitmap)
{

	int h = bitmap->GetHeight();
	int w = bitmap->GetWidth();
	int x,y;

	//make image from color to gray
	image = new BYTE* [h];
	image[0] = new BYTE [h*w];
	for (y=0; y<h; y++)
		image[y] = image[0] + w*y;

	grayimage(bitmap, image);


	//convert to black-white image 
	int threshold = 150;
	for (y=0; y<h; y++)
		for(x=0; x<w; x++)
		{
			if (image[y][x]>threshold)
				image[y][x] = WHITE; // This is for white color;
			else image[y][x] = BLACK; // This is for Black color;
		}

}

void thinningImage(BYTE **image , Bitmap *bitmap, CLSID *clsid)
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
	int h = bitmap->GetHeight();
	int w = bitmap->GetWidth();

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

void Save2File(char* fname, BYTE **image, Bitmap *bitmap, CLSID *clsid)
{
	int x, y;
	WCHAR wstr [100];
    Status stat;

    mbstowcs(wstr, fname, 100);

	if (image != NULL)
	{
		int h = bitmap->GetHeight();
		int w = bitmap->GetWidth();	
		Bitmap * newbitmap = new Bitmap(w,h);
 		for (y=0; y<h; y++)
			for (x=0; x<w; x++)
				newbitmap->SetPixel(x,y,Color((1-image[y][x])*255,(1-image[y][x])*255,(1-image[y][x])*255));

		stat = newbitmap->Save(wstr, clsid, NULL);

	}
	else stat = bitmap->Save(wstr, clsid, NULL);

    if(stat == Ok)
		printf("file saved successfully.\n");
	else 
		printf("file saved failed .\n");
}

void grayimage(Bitmap *bitmap, BYTE **image)
{	

	int x, y;
	Color i_color;
	int h = bitmap->GetHeight();
	int w = bitmap->GetWidth();

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
