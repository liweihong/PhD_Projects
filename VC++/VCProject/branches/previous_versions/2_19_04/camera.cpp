#include "VCProject.h"

void convolve (BYTE **in, BYTE **out, int flag, int **mask, int mask_h, int mask_w, int w, int h);
int iround (float u) {return int (floor (.5+u));} 
void undistortion(BYTE** oimage, BYTE** oimage_note, BYTE** image, BYTE** image_note, int w, int h);
BYTE** new_image(int w, int h);
void hough_trans (BYTE** image, BYTE** newimage, int w, int h, double* coord);
void binary_image(BYTE** image, BYTE** image1, int threshold, int w, int h);
void drawline(BYTE **image, BYTE color, int sx, int sy, int ex, int ey);

void persptrans_image(BYTE** image, BYTE** image_pptnote, double* p_matrix, int w, int h, int neww, int newh);
void auto_thres_binary_image(BYTE** image, int w, int h);
int Find_thres(BYTE** image, int T, int w, int h);
void Do_btwn_T1T2(BYTE** image, int T1, int T2, int w, int h);
void Double_thres(BYTE** image, int T1, int T2, int w, int h);

void Final_Register(Bitmap *&result, Bitmap *A_bitmap, Bitmap *C_bitmap, double* R1, double* R2);


void camera_calibrate ( BYTE **&ppt_image,Bitmap *&ppt_bitmap )
{

	//read image and convert it to gray image which 8-bit format

    CLSID  clsid;
	int y, x;	
	Bitmap* bitmap0= new Bitmap(L"image\\pre-camera.bmp");
	GetEncoderClsid(L"image/bmp", &clsid);
 	Bitmap* bitmap1 = new Bitmap(L"image\\post-camera.bmp");

	int	h = bitmap0->GetHeight();
	int w = bitmap0->GetWidth();

	int logmask[5][5]  = { 0, 0,-1, 0, 0,  
						   0,-1,-2,-1, 0,
						  -1,-2,16,-2,-1,
						   0,-1,-2,-1, 0,
						   0, 0,-1, 0, 0};

	int **mask = NULL;

	//make image from color to gray
	BYTE **oimage = new_image(w,h);
	grayimage(bitmap0, oimage);

	BYTE **oimage_note = new_image(w,h);
	grayimage(bitmap1, oimage_note);
	
	BYTE **image = new_image(w,h);
	BYTE **image1 = new_image(w,h);
	BYTE **image_note = new_image(w,h);

	undistortion(oimage, oimage_note, image, image_note, w, h);
	Save2File("image\\undistortion.jpg",  image, NULL, &clsid, w, h);
	Save2File("image\\undistortion_note.jpg",  image_note, NULL, &clsid, w, h);

	//get binary images from original image using the good threshold.
////	auto_thres_binary_image(image, w, h);
/**/
	binary_image(image, image1, 185, w, h);
	binary_image(image, image,  180, w, h);
	
	//do lapalican gaussian filter
	BYTE **logout = new_image(w,h);
	mask = new int* [5];
	for (y=0; y<5; y++)
	{
		mask[y] = new  int[5];
		for (x=0; x<5; x++)
			mask[y][x] = 0;
	}
	
	for(y=0; y<5; y++) 
	for(x=0; x<5; x++)	
		mask[y][x] = logmask[y][x];

	convolve(image, logout, 0, mask, 5, 5, w, h);

//convolve(image_note, logout1, 0, mask, 5, 5, w, h);
	Save2File("image\\logout.jpg",	logout,	NULL, &clsid, w, h);
//	Save2File("image\\logout1.jpg",	logout,	&clsid, w, h);

	//hough transform
	//coord: the coordinates of the 4 corner points.
	double coord0[8];
	BYTE **newimage = new_image(w,h);

	hough_trans (logout, newimage, w, h, coord0);
	Save2File("image\\lineout.jpg", newimage, NULL, &clsid, w, h);
	
	

	binary_image(image_note, image_note, 180, w, h);

////	auto_thres_binary_image(image_note, w, h);
	//	Save2File("image\\undistortion.jpg",  image, NULL, &clsid, w, h);
	//Save2File("image\\undistortion_note.jpg",  image_note, NULL, &clsid, w, h);
	BYTE **image_onlynote = new_image(w,h);
// get the note from the image by subtract two original images.
	for (y=0; y<h; y++)
	for (x=0; x<w; x++)
	{
		image_onlynote[y][x] = abs(image[y][x]-image_note[y][x]);
		if (image_onlynote[y][x]>200 && image1[y][x]<100)
			image_onlynote[y][x]=0;			
	}

	Save2File("image\\binarynote_onlynote.jpg",	image_onlynote, NULL,&clsid, w, h);
	
	int neww = 720;
	int newh = 540;
	double coord1[8] = {0,neww,neww,0,0,0,newh,newh};
	mwArray Coord_source(4, 2, coord0);
	mwArray Coord_target(4, 2, coord1);
	mwArray B(8, 1, coord0);
	mwArray R = presp_transform(Coord_source, Coord_target, B, 4); 
	R.Print("R");
	double* p_matrix = mxGetPr(R.GetData());	

	//store transformed image through perspective trans.
	BYTE** image_pptnote = new_image(neww, newh);

	persptrans_image(image_onlynote, image_pptnote, p_matrix, w, h, neww, newh);
	binary_image(image_pptnote, image_pptnote, 200, neww, newh);	
	Save2File("image\\pptonlynote.jpg",	image_pptnote, NULL, &clsid, neww, newh);

	// by liwh
	ppt_image = image_pptnote;
	ppt_bitmap = new Bitmap(neww, newh);
	for (int i = 0; i < neww; i++)
		for (int j = 0; j < newh; j++)
			ppt_bitmap->SetPixel(i,j, Color( 255 - ppt_image[j][i],  255 - ppt_image[j][i],  255 - ppt_image[j][i] ));


}

void auto_thres_binary_image(BYTE** image, int w, int h)
{	
	int i, x, y, total = w*h;
	int h1[256];
	long cumulator=0;
	//calculate threshold
	for (i=0; i<256; i++) h1[i]=0;
    for (y=0; y<h; y++) 
	for (x=0; x<w; x++)
	{
		h1[image[y][x]]++ ; //evalutes histogram
		cumulator += image[y][x];
	}

	int T = cumulator/total;
	int T1 = Find_thres(image, T, w, h);
	printf("T1= %d", T1);
	int m=0;
	int cumulator1 = 0;
	double mean=0.0;

    for (y=0; y<h; y++) 
	for (x=0; x<w; x++)
		if(image[y][x]>T1) m++;
	printf("m=%d\n",  m);
	for (i=T1+1; i<256; i++)
		cumulator1 += h1[i]*i; 
	mean = cumulator1/m;
	double variance= 0.0;
	for (i=T1+1; i<256; i++)
		variance += (i-mean)*(i-mean);
	variance = sqrt(variance/(255-T1));
	
    int T2 = mean-variance;
	
	Double_thres(image, T1, T2, w, h);
}

void Double_thres(BYTE** image, int T1, int T2, int w, int h)
{
	int y, x;
	for (y=0; y<h; y++)
	for (x=0; x<w; x++)
	{
		if (image[y][x]<T1)
			image[y][x] = 0;
		else 
		{
			if(image[y][x]>T2) 
				image[y][x] = 255;
		}
	}

	Do_btwn_T1T2(image, T1, T2, w, h);	
}

void Do_btwn_T1T2(BYTE** image, int T1, int T2, int w, int h)
{
	int flag = -1;
	int y, x, minx, miny, maxx, maxy;
	while (flag==-1)
	{
		flag = 0;
		for (y=0; y<h; y++)
		for (x=0; x<w; x++)
		{
			if (image[y][x]>T1 && image[y][x]<T2)
			{
				minx = MAX(x, 1);
				miny = MAX(y, 1);
				maxx = MIN(x, w-2);
				maxy = MIN(y, h-2);
				if (image[miny-1][x]==0 || image[y][minx-1]==0 || image[maxy+1][x]==0 || image[y][maxx+1]==0 || image[miny-1][maxx+1]==0 || image[miny-1][minx-1]==0 || image[maxy+1][minx-1]==0 || image[maxy+1][maxx+1]==0)
				{
					image[y][x] = 0;
					flag = -1;
				}
			}
		}			
	}
	for (y=0; y<h; y++)
	for (x=0; x<w; x++)
	{
		if (image[y][x]>T1 && image[y][x]<T2)
			image[y][x] = 255;
	}
}

int Find_thres(BYTE** image, int T, int w, int h)
{
		long cumulator1=0;
	long cumulator2=0;
	int m=0;
	int n=0;
	for (int y=0; y<h; y++)
	for (int x=0; x<w; x++)
	{
		if(image[y][x] < T)
		{
			cumulator1 += image[y][x];
			m++;
		}	
		else
		{
			cumulator2 += image[y][x];
			n++;
		}
	}
	double mean1 = cumulator1/m;
	double mean2 = cumulator2/n;
	printf("mean1 = %f, mean2= %f\n", mean1, mean2);
	int T1 =(mean1+mean2)/2;
	if(T1 != T) T1=Find_thres(image, T1, w, h);
	return T1;
}



void persptrans_image(BYTE** image, BYTE** image_pptnote, double* p_matrix, int w, int h, int neww, int newh)
{
	double zz, xx, yy, dx, dy;
	int fx, fy; 
	for(int y=0; y<newh; y++)
	for(int x=0; x<neww; x++)
	{
		zz =  p_matrix[6]*x + p_matrix[7]*y + 1;
		xx = (p_matrix[0]*x + p_matrix[1]*y + p_matrix[2])/zz;
		yy = (p_matrix[3]*x + p_matrix[4]*y + p_matrix[5])/zz;
		fx = (int)floor(xx);
		fy = (int)floor(yy);
		dx = xx - fx;
		dy = yy - fy;
		if(fx>0&&fx+1<w&&fy>0&&fy+1<h)
			image_pptnote[y][x]= (BYTE)((1-dy)*( (1-dx)*image[fy][fx]+dx*image[fy][fx+1] ) + dy*((1-dx)*image[fy+1][fx]+dx*image[fy+1][fx+1]));
	}
}

void binary_image(BYTE** image, BYTE** image1, int threshold, int w, int h)
{
	for (int y=0; y<h; y++)
	for (int x=0; x<w; x++)
	{
		if (image[y][x]<threshold)
			image1[y][x] = 0;
		else image1[y][x] = 255;
	}
}

void hough_trans (BYTE** image, BYTE** newimage, int w, int h, double* coord)
{
	int x, y;
	x=y=0;
	int p = (int) sqrt(w*w+h*h);
	int angle = 180;
	int **	parameter = new int* [p];
	for (y=0; y<p; y++)
	{
		parameter[y] = new int[angle];
		for (x=0; x<angle; x++)
			parameter[y][x] = 0;
	}
	
	int tp;
	BYTE a=0, b=0; 
	const double pi = 3.1415/180;
	double theta;
	for (y=0; y<h; y++)
	for (x=0; x<w; x++)
		if (image[y][x]==255)
				for (int j=0; j<angle; j++)
				{
					theta = j*pi;
					tp = (int) ((x-w/2)*cos(theta)+(y-h/2)*sin(theta))+p/2;
					if ((tp>0)&&(tp<p))
						parameter[tp][j]++;
				}

	int maxvote[cnt] ;
	int maxptable[cnt], maxangletable[cnt]; 
	int counter = 0;

	for (int i=0; i<p; i++)
	for (int j=0; j<angle; j++)
	{
		if(parameter[i][j]>200)
		{
			maxvote[counter] = parameter[i][j];
			maxptable[counter] = i;
			maxangletable[counter] = j;
			counter++;
		}
	}

	for(i=0; i<counter; i++)
	{
		if (maxvote[i]!=0)
		{
			for(int j=i+1; j<counter; j++)
			{ 
				
				if(abs(maxptable[j]-maxptable[i])<8 && abs(maxangletable[j]-maxangletable[i])<3)
				{
					maxvote[i] += maxvote[j];
					maxptable[i] = (maxptable[i]*maxvote[i]+maxptable[j]*maxvote[j])/(maxvote[i]+maxvote[j]);
					maxangletable[i] = (maxangletable[i]*maxvote[i]+maxangletable[j]*maxvote[j])/(maxvote[i]+maxvote[j]);
					maxptable[j] = 0;
					maxangletable[j] = 0;
					maxvote[j] = 0;
				}
			}
		}
	}
	
	int max, temp;
	for(i=0; i<counter; i++)
	{
		max = i;
		for(int j=i; j<counter; j++)
		{
			if(maxvote[j]>=maxvote[max])
				max = j;
		}
		temp = maxvote[i];
		maxvote[i] = maxvote[max];
		maxvote[max] = temp;

		temp = maxptable[i];
		maxptable[i] = maxptable[max];
		maxptable[max] = temp;
		
		temp = maxangletable[i];
		maxangletable[i] = maxangletable[max];
		maxangletable[max] = temp;
	}
	double angle4[4], distance4[4];
	for(i=0; i<4; i++)
	{
		distance4[i] = maxptable[i];
		angle4[i] = maxangletable[i];
		printf("%d maxvote[%d]= %d, maxptable[%d]=%d maxangletable[%d]=%d\n",i,i,maxvote[i],i,maxptable[i],i,maxangletable[i]);

		if(maxvote[i]>=0)
		{
			theta=maxangletable[i]*pi;
			int dis = maxptable[i];
			for (y=0; y<h; y++)
			for (x=0; x<w; x++)
			{
				tp = (int) ((x-w/2)*cos(theta)+(y-h/2)*sin(theta))+p/2;
				if (tp == dis) newimage[y][x] = 255;
			}
		}
/**/
	}

	mwArray mangle(4, 1, angle4);
	mwArray mdistance(4, 1, distance4);
	mwArray coordinate = get6points(mangle, mdistance, w, h);
	double* result1 = mxGetPr(coordinate.GetData());	
	double* result = new double[8];
	int index = 0;
	double x_cen, y_cen, x_sum, y_sum;
	x_sum=y_sum=0;
	for(i=0; i<6; i++)
	{
		if(result1[i]<w && result1[i]>0 && result1[i+6]<h && result1[i+6]>0)
		{
			result[index] = result1[i];
			result[index+1] = result1[i+6];
			x_sum += result[index];
			y_sum += result[index+1];
			index += 2;
		}
	}
	x_cen = x_sum/4;
	y_cen = y_sum/4;
	printf("x_cen=%f, y_cen=%f\n",x_cen, y_cen);
	for(i=0; i<8; i=i+2)
	{
		if(result[i]<x_cen && result[i+1]<y_cen)
		{
			coord[0] = result[i];
			coord[4] = result[i+1];
		}
		if(result[i]>x_cen && result[i+1]<y_cen)
		{
			coord[1] = result[i];
			coord[5] = result[i+1];
		}

		if(result[i]>x_cen && result[i+1]>y_cen)
		{
			coord[2] = result[i];
			coord[6] = result[i+1];
		}
		if(result[i]<x_cen && result[i+1]>y_cen)
		{
			coord[3] = result[i];
			coord[7] = result[i+1];
		}
	}


	for(i=0; i<8; i++)
		printf("coord[%d] = %f\n", i, coord[i]);

}


void join(float **center, int i, int label)
{
	int j = (int)center[i][8];
	int total = (int)(center[i][0]+center[j][0]);
	center[i][1] = (center[i][0]*center[i][1]+center[j][0]*center[j][1])/total;
	center[i][2] = (center[i][0]*center[i][2]+center[j][0]*center[j][2])/total;
	center[i][0] = (float)total;
	center[i][3] = MIN(center[i][3], center[j][3]);
	center[i][4] = MIN(center[i][4], center[j][4]);
	center[i][5] = MAX(center[i][5], center[j][5]);
	center[i][6] = MAX(center[i][6], center[j][6]);
	center[j][0] = 0;
//	printf("join one more time\n");
}

void calcu_min_centroid_dis(float **center, int i, int label)
{
	double dis=0.0;
	for(int j=i+1; j<label; j++)	
	{
		if (center[j][0] != 0)
		{
			dis = sqrt((center[i][1]-center[j][1])*(center[i][1]-center[j][1])+(center[i][2]-center[j][2])*(center[i][2]-center[j][2]));
			if (center[i][7] > dis) 
			{
				center[i][7] = (float)dis;
				center[i][8] = (float)j;
			}
		}
	}
}

BYTE** new_image(int w, int h)
{
	BYTE **image = new BYTE* [h];
	for (int y=0; y<h; y++)
	{
		image[y] = new BYTE[w];
		for (int x=0; x<w; x++)
			image[y][x] = 0;
	}
	return(image);
}

void undistortion(BYTE** oimage, BYTE** oimage_note, BYTE** image, BYTE** image_note, int w, int h)
{
	double xx, yy;
	double dx, dy;
	double  ratio;
	float x1, y1;
	int fx, fy;

	for (int y=0; y<h; y++)
	for (int x=0; x<w; x++)
	{
		x1 = (float)x;
		y1 = (float)y;
		x1 = (x1-w/2)/w;
		y1 = (y1-h/2)/h;
		ratio = (1.00  + (-0.35)*(x1*x1 + y1*y1) + (0.39)*(x1*x1+y1*y1)*(x1*x1+y1*y1) );
		xx = x1*ratio*w+w/2;
		yy = y1*ratio*h+h/2;
		fx = (int)floor(xx);
		fy = (int)floor(yy);
		dx = (double)(xx-fx);
		dy = (double)(yy-fy);
		if(xx>0&&xx<w&&yy>0&&yy<h)
		{
			image[y][x]= (BYTE)((1-dy)*((1-dx)*oimage[fy][fx]+dx*oimage[fy][fx+1]) + dy*((1-dx)*oimage[fy+1][fx]+dx*oimage[fy+1][fx+1]));//oimage[yy][xx];
			image_note[y][x]= (BYTE)((1-dy)*((1-dx)*oimage_note[fy][fx]+dx*oimage_note[fy][fx+1]) + dy*((1-dx)*oimage_note[fy+1][fx]+dx*oimage_note[fy+1][fx+1]));//oimage[yy][xx];
		}
	}

}

void convolve (BYTE **in, BYTE **out, int flag, int **MASK, int maskw, int maskh, int w, int h)
{
	int i=0, j=0; 
	int label=0;
	int max=0, min=0, total=0, threshold=0;
	int x, y, xx,yy,yofst,xofst,buf_w;

	//allocate one mask_h X (w+5) temporary array to calculate covolution
	yofst = (maskh-1)/2;
	xofst = (maskw-1)/2;
	BYTE **maskbuf = new BYTE* [maskh];
	buf_w = w+maskw-1;
	for(i=0; i<maskh; i++)
  		maskbuf[i] = new BYTE[buf_w];

	//use same padding
	for(y=0; y<yofst; y++)
	for(x=0; x<w  ; x++)
  		maskbuf[y][x+xofst] = in[0][x];
  		
	for(   ; y<maskh ; y++)
	for(x=0; x<w  ; x++)
  		maskbuf[y][x+xofst] = in[y][x];
  		
	for(y=0; y<maskh ; y++)
	for(x=0; x<xofst; x++)
	{
  		maskbuf[y][x] = maskbuf[y][xofst];
  		maskbuf[y][xofst+w+x] = maskbuf[y][w+xofst-1];
	}

	for(y=0; y<maskh-1; y++)
	{
	  for(x=0; x<maskw+w-1; x++)
//  		printf(" %d ",maskbuf[y][x]);
	  printf("\n");
	}
	double sum;	
	//covolve image with lapalican gaussian filter
	for(yy=0; yy<h; )
	{
	for(xx=0; xx<w; xx++)
	{
		sum = 0.0;
		for(y=0; y<maskh; y++)
		for(x=0; x<maskw; x++)
		{
			sum+=maskbuf[y][x+xx] * MASK[y][x];	
		}
		if (flag==1)	  sum = sum/1115; //printf("%f\t",sum);}
		if (sum > 255.0)  sum=255;
		if (sum < 0.0)    sum=0;

		out[yy][xx] = (BYTE)sum;
//		if (flag == 3)  printf("%d\t",out[yy][xx]);
	}//for xx=0->w
			
	yy++;
	for(y=0; y<maskh-1; y++)	
	for(x=0; x<w   ; x++)
		maskbuf[y][x+xofst] = maskbuf[y+1][x+xofst];
			
	for(x=0; x<w; x++)
		if(yy < h-yofst)  maskbuf[maskh-1][x+xofst]=in[yy+yofst][x];
				
	for(y=0; y<maskh ; y++)
	for(x=0; x<xofst; x++)
	{
		maskbuf[y][x] = maskbuf[y][xofst];
		maskbuf[y][w+xofst+x] = maskbuf[y][w+xofst-1];
	}//padding
	}

}


void drawline (BYTE **image, BYTE color, int sx,int sy, int ex,int ey)	//when user call this func., sx need be less than ex. Or do a swap in this function if sx is greater than ex.
{
	int x, y, x1, x2, y1, y2;

	x1 = MIN(sx, ex);	x2 = MAX(sx, ex);
	y1 = MIN(sy, ey);	y2 = MAX(sy, ey);
	x = sx;
	y = sy;
	image[sy][sx] = 0;
	image[ey][ex] = 0;
	float step;
	if(abs(ey-sy) > abs(ex-sx))
	{
		step = (ex-sx)/(ey-sy);
		for (y=y1+1; y<y2; y++)
		{
			x = (int)(x1 + step);
			image[y][x] = 0;
		}
	}
	else
	{
		step = (ey-sy)/(ex-sx);
		for (x=x1+1; x<x2; x++)
		{
			y = (int)(y + step);
			image[y][x] = 0;
		}
	}
}