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
	

	BYTE **pptprint_image = NULL;			//add 2/18
	Bitmap *bitmap_pptprint = NULL, *PPT_scaled=NULL;			//add 2/18

	
	/*
	Bitmap* bitmap_ppt = new Bitmap(L"image\\pptonlynote.bmp");	
 	set_image(ppt_image,bitmap_ppt);
	thinningImage(ppt_image,bitmap_ppt,&clsid);
	*/
	
	Bitmap *bitmap_ppt = NULL;
	camera_calibrate(ppt_image, bitmap_ppt, pptprint_image, bitmap_pptprint);		//modify 2/18
	get_image(ppt_image,bitmap_ppt);
	thinningImage(ppt_image, bitmap_ppt, &clsid);
	Save2File("image\\ppt_thinned.bmp", ppt_image, bitmap_ppt, &clsid, 0 , 0);
	

	//do thinning on the ppt_print image.
	get_image(pptprint_image,bitmap_pptprint);					//add 2/18
	thinningImage(pptprint_image, bitmap_pptprint, &clsid);		//add 2/18
	Save2File("image\\pptprint_thinned.bmp", pptprint_image, bitmap_pptprint, &clsid, 0 , 0);	//add 2/18
	Save2File("image\\pptprintbitmap_thinned.bmp", NULL, bitmap_pptprint, &clsid, 0 , 0);	//add 2/18
	//do thinning on the PPT image.
	Bitmap* bitmap_ppt_orig1 = new Bitmap(L"image\\ppt_1.jpg");	//move 2/18
//	Bitmap* ppt_video_registered1;								//move 2/18
	BYTE **PPT_image1=NULL, **video_PPT_image=NULL;											//move 2/18
	set_image(PPT_image1, bitmap_ppt_orig1);					//move 2/18
	thinningImage(PPT_image1 ,bitmap_ppt_orig1,&clsid);			//add  2/18
	Save2File("image\\PPT_thinning.bmp", PPT_image1, bitmap_ppt_orig1,&clsid, 0, 0);
	Save2File("image\\PPT_thinning_bitmap.bmp", NULL, bitmap_ppt_orig1,&clsid, 0, 0);

	scale_translate(  pptprint_image, PPT_image1, bitmap_pptprint, bitmap_ppt_orig1,PPT_scaled,params);	//add 2/18
	Save2File("image\\PPT_scaled.bmp",NULL,PPT_scaled,&clsid, 0, 0);									//add 2/18
cout<<"params[0]"<<params[0]<<endl;

	Bitmap *video_matching1 = NULL, *ppt_matched1=NULL;	
	BYTE** ppt_scaled_image= NULL;
	Bitmap *overlap_coarse_bitmap1;
	set_image(ppt_scaled_image, PPT_scaled);																		//add 2/18
	set_image(video_PPT_image, bitmap_pptprint);
	overlap_coarse(video_PPT_image, ppt_scaled_image ,overlap_coarse_bitmap1,PPT_scaled->GetWidth(), PPT_scaled->GetHeight());
	Save2File("image\\video_PPT_coarse_registered.bmp",NULL,overlap_coarse_bitmap1,&clsid, 0, 0);


	get_image(ppt_scaled_image, PPT_scaled);																		//add 2/18
	get_image(video_PPT_image, bitmap_pptprint);
	Save2File("image\\test111.bmp",NULL,PPT_scaled,&clsid,0,0);
	Save2File("image\\test112.bmp",ppt_scaled_image,PPT_scaled,&clsid,0,0);
	matching(video_PPT_image,ppt_scaled_image,bitmap_pptprint,PPT_scaled, video_matching1, ppt_matched1, params);	//add 2/18
	Save2File("image\\videoppt_matching1.bmp",video_PPT_image,bitmap_pptprint,&clsid, 0, 0);					//add 2/18
	Save2File("image\\ppt_matched.bmp",ppt_scaled_image,PPT_scaled,&clsid, 0, 0);
	Bitmap *overlap_bitmap_video;
	overlap(video_PPT_image, ppt_scaled_image,params,overlap_bitmap_video);
	Save2File("image\\video_ppt_registered.bmp",NULL,overlap_bitmap_video,&clsid, 0, 0);						//add 2/18

	Bitmap* bitmap_mimio = new Bitmap(L"image\\mimio.bmp");
	set_image(mimio_image,bitmap_mimio);
	thinningImage(mimio_image,bitmap_mimio,&clsid);
	Save2File("image\\mimio_thinned.bmp",mimio_image,bitmap_mimio,&clsid, 0, 0);

	//Scaling and translating the mimio image.
	scale_translate(ppt_image, mimio_image, bitmap_ppt, bitmap_mimio, mimio_scaled,params);
	Save2File("image\\mimio_scaled.bmp",NULL,mimio_scaled,&clsid, 0, 0);

	//Coarse registration of video + mimio
	Bitmap *overlap_coarse_bitmap;
	set_image(mimio_scaled_image, mimio_scaled); // The mimio_scaled_image will be used for next step.
	get_image(ppt_image,bitmap_ppt);
	overlap_coarse(ppt_image, mimio_scaled_image,overlap_coarse_bitmap,518,349);
	Save2File("image\\video_mimio_coarse_registered.bmp",NULL,overlap_coarse_bitmap,&clsid, 0, 0);

	//Matching the scaled mimio image with the ppt image.
	get_image(ppt_image,bitmap_ppt);
	get_image(mimio_scaled_image,mimio_scaled);
	matching(ppt_image,mimio_scaled_image,bitmap_ppt,mimio_scaled, mimio_matching, ppt_matched, params);
	Save2File("image\\mimio_matching.bmp",mimio_scaled_image,mimio_scaled,&clsid, 0, 0);
	Save2File("image\\ppt_matched.bmp",ppt_image,bitmap_ppt,&clsid, 0, 0);

	//Show the matched mimio and ppt-video image
	Bitmap *overlap_bitmap;
	get_image(mimio_image,bitmap_mimio);
	get_image(ppt_image,bitmap_ppt);
	overlap(ppt_image, mimio_image,params,overlap_bitmap);
	Save2File("image\\video_mimio_registered.bmp",NULL,overlap_bitmap,&clsid, 0, 0);
	//End of show

	Bitmap* bitmap_ppt_orig = new Bitmap(L"image\\ppt_1.jpg");
	Bitmap* ppt_mimio_registered;
	BYTE **PPT_image;
	set_image(PPT_image, bitmap_ppt_orig); 
	transformation(ppt_mimio_registered,bitmap_mimio,bitmap_ppt_orig,params);
	overlap_coarse(PPT_image, ppt_image ,overlap_coarse_bitmap,518,349);
	Save2File("image\\video_ppt_registered.bmp",NULL,overlap_coarse_bitmap,&clsid, 0, 0);
	Save2File("image\\ppt_mimio_registered.bmp",NULL,ppt_mimio_registered,&clsid, 0, 0);


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

void Save2File(char* fname, BYTE **image, Bitmap *bitmap, CLSID *clsid, int image_W, int image_H)
{
	int x, y;
	WCHAR wstr [100];
    Status stat;

    mbstowcs(wstr, fname, 100);

	if (image != NULL && ( image_W == 0 || image_H ==0) )
	{
		int h = bitmap->GetHeight();
		int w = bitmap->GetWidth();	
		Bitmap * newbitmap = new Bitmap(w,h);
 		for (y=0; y<h; y++)
			for (x=0; x<w; x++)
				newbitmap->SetPixel(x,y,Color((1-image[y][x])*255,(1-image[y][x])*255,(1-image[y][x])*255));

		stat = newbitmap->Save(wstr, clsid, NULL);

		delete newbitmap;

	}
	else if (image != NULL)
	{
		int h = image_H;
		int w = image_W;	
		Bitmap * newbitmap = new Bitmap(w,h);
 		for (y=0; y<h; y++)
			for (x=0; x<w; x++)
				newbitmap->SetPixel(x,y,Color(image[y][x], image[y][x], image[y][x]));

		stat = newbitmap->Save(wstr, clsid, NULL);

		delete newbitmap;
	}

	else stat = bitmap->Save(wstr, clsid, NULL);

    if(stat == Ok)
		cout<<fname<<" saved successfully."<<endl;
	else 
		cout<<fname<<" saved failed."<<endl;
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
//			image[y][x] = (BYTE)(i_color.GetR()*0 + i_color.GetG()*0 + i_color.GetB()*1);
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
