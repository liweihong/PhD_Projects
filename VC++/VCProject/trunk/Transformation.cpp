#include "VCProject.h"
#define RUN 50

BYTE** new_image(int w, int h);

//when user call this func., sx need be less than ex. Or do a swap in this function
// if sx is greater than ex.
void drawline_red (Bitmap *bitmap, int sx,int sy, int ex,int ey)	
{
	int x, y, x1, x2, y1, y2;
	int w = bitmap->GetWidth();
	int h = bitmap->GetHeight();

	x1 = sx; x2 = ex;
	y1 = sy, y2 = ey;
	x = sx;
	y = sy;

	bitmap->SetPixel(sx, sy, Color(255,0,0));
	bitmap->SetPixel(ex, ey, Color(255,0,0));

	float step;
	step = (float)(ey-sy)/(ex-sx);
	if (abs((int)step) < 1 )
	{
		for (x=x1+1; x<x2; x++)
		{
			y = (int)(y1 + (x - 1 - x1)*step);
			//image[y][x] = 0;
			if (x >= 0 && y >= 0 && x < w && y < h)
				bitmap->SetPixel(x, y, Color(255,0,0));
		}
	}
	else
	{
		step = 1/step;
		if (y1 > y2) { int yy = y1, xx = x1; y1 = y2; x1 = x2; x2 = xx; y2 = yy;}
		for (y=y1+1; y<y2; y++)
		{
			x = (int)(x1 + (y - 1 - y1)*step);
			//image[y][x] = 0;
			if (x >= 0 && y >= 0 && x < w && y < h)
				bitmap->SetPixel(x, y, Color(255,0,0));
		}
	}
}

void transformation(Bitmap *&result, Bitmap *A_bitmap, Bitmap *C_bitmap, double *matrix_value)
{

	int h_A = A_bitmap->GetHeight();
	int w_A = A_bitmap->GetWidth();
	int h_C = C_bitmap->GetHeight();
	int w_C = C_bitmap->GetWidth();
	
	// Calcualte the four coordinate after transforming.
	int i, j;
	double div_temp;
	
	// Get the Result matrix
	double R[9];

	/*
	R[1] =     1.9578;
	R[2] =     0.0058;
	R[3] =  -155.1952;
	R[4] =     0.0021;
	R[5] =     1.9465;
	R[6] =  -157.4790;
	R[7] =     0.0000;
	R[8] =    -0.0000;
	*/

	for ( i = 1; i < 9; i++) R[i] = matrix_value[ i - 1];


	i = 1; j = 1;
	div_temp = R[7] * j + R[8] * i + 1;
	int xx1 = (int) ((R[1] * j + R[2] * i + R[3]) / div_temp + 1);
	int yy1 = (int) ((R[4] * j + R[5] * i + R[6]) / div_temp + 1);

	i = 1; j = w_A;
	div_temp = R[7] * j + R[8] * i + 1;
	int xx2 = (int) ((R[1] * j + R[2] * i + R[3]) / div_temp + 1);
	int yy2 = (int) ((R[4] * j + R[5] * i + R[6]) / div_temp + 1);

	i = h_A; j = 1;
	div_temp = R[7] * j + R[8] * i + 1;
	int xx3 = (int) ((R[1] * j + R[2] * i + R[3]) / div_temp + 1);
	int yy3 = (int) ((R[4] * j + R[5] * i + R[6]) / div_temp + 1);

	i = h_A; j = w_A;
	div_temp = R[7] * j + R[8] * i + 1;
	int xx4 = (int) ((R[1] * j + R[2] * i + R[3]) / div_temp + 1);
	int yy4 = (int) ((R[4] * j + R[5] * i + R[6]) / div_temp + 1);

	int xx_min = (xx1 < xx3) ? xx1 : xx3;
	int xx_max = (xx2 > xx4) ? xx2 : xx4;
	int yy_min = (yy1 < yy2) ? yy1 : yy2;
	int yy_max = (yy3 > yy4) ? yy3 : yy4;

	int h_Result = yy_max - yy_min;
	int w_Result = xx_max - xx_min;
	
	// Set the size of the new image.
	result = new Bitmap(w_Result, h_Result);

	Color i_color;

	// Initilize the image canvas to a white color.
	for (i = 0; i < w_Result; ++i)
		for (j = 0; j < h_Result; ++j)
			result->SetPixel(i,j,Color(255,255,255));

	// Adjust the coordinate system and set the PPT to the big canvas.
	// Now, the origin point of the PPT is (0,0), we should change it to this:

	for (i = 0; i < w_C ; ++i)
		for (j = 0; j < h_C; ++j)
		{
			C_bitmap->GetPixel(i,j, &i_color);
			result->SetPixel(i - xx_min, j - yy_min, i_color);
		}

	cout<<endl<<"The coords:"<<xx1<<","<<yy1<<" "<<xx2<<" "<<yy2<<xx3<<yy3<<xx4<<yy4;
	drawline_red(result,xx1-xx_min,yy1-yy_min,xx2-xx_min,yy2-yy_min);
	drawline_red(result,xx3-xx_min,yy3-yy_min,xx1-xx_min,yy1-yy_min);
	drawline_red(result,xx3-xx_min,yy3-yy_min,xx4-xx_min,yy4-yy_min);
	drawline_red(result,xx2-xx_min,yy2-yy_min,xx4-xx_min,yy4-yy_min);
	// Declare a 3-D int array to store the colors.
	int ***RGB = new int **[h_A];
	RGB[0] = new int *[h_A*w_A];
	RGB[0][0] = new int[h_A*w_A*3];
	for (i = 0; i < h_A; i++)
	{
		RGB[i] = RGB[0] + i*w_A;
		for (int j =0; j < w_A; j++)
			RGB[i][j] = RGB[0][0] + (i*w_A*3 + j*3); // MUST BE i*w_A*3!!!
	}

	// Get the Mimio image pixel array.
    for ( i = 0; i<h_A; i++)
    {
		for (int j = 0 ; j < w_A; j++)
		{
				   A_bitmap->GetPixel(j,i,&i_color);
				   RGB[i][j][0] = i_color.GetR();
				   RGB[i][j][1] = i_color.GetG();
				   RGB[i][j][2] = i_color.GetB();
		}
    }

	// Get the reverse matrix.

	/*
	R[1] =  0.5105;
	R[2] = -0.0012;
	R[3] = 79.0326;
	R[4] = -0.0015;
	R[5] =  0.5147;
	R[6] = 80.8181;
	R[7] = -0.0000;
	R[8] =  0.0000;*/

	for ( i = 1; i < 9; i++) R[i] = matrix_value[ i + 7];

            
	double r1, r2;
	double v1[3],v2[3];
	int v3[3];
	int i0, j0, xx, yy;
	double xPosition, yPosition;
	
	// Computing the combined image using bilear interpolation.
	for ( i0 = 0; i0 < h_Result; ++i0)
		for (j0 = 0; j0 < w_Result; ++j0)
		{
			i = i0 + yy_min;
			j = j0 + xx_min;
			div_temp = R[7] * j + R[8] * i + 1;
			xPosition = (R[1] * j + R[2] * i + R[3]) / div_temp + 1;
			yPosition = (R[4] * j + R[5] * i + R[6]) / div_temp + 1;
			xx = (int)xPosition;
			yy = (int)yPosition;
			//Bilinear interpolation.
			r1 = xPosition - floor(xPosition);
			r2 = yPosition - floor(yPosition);

			if ( xx < 1 || xx >= w_A - 1 || yy < 1 || yy >= h_A -1)
				continue;

			for (int rgb_num = 0; rgb_num < 3; rgb_num++)
			{
				v1[rgb_num] = RGB[(int)ceil(yPosition)][(int)floor(xPosition)][rgb_num] +
							(RGB[(int)ceil(yPosition)][(int)ceil(xPosition)][rgb_num] - 
							RGB[(int)ceil(yPosition)][(int)floor(xPosition)][rgb_num]) * r1;
				v2[rgb_num] = RGB[(int)floor(yPosition)][(int)floor(xPosition)][rgb_num] +
							(RGB[(int)floor(yPosition)][(int)ceil(xPosition)][rgb_num] -
							RGB[(int)floor(yPosition)][(int)floor(xPosition)][rgb_num]) * r1;
				v3[rgb_num] = (int)(v2[rgb_num] + (v1[rgb_num] - v2[rgb_num]) * r2);
			}

			if ( xx >= 0 && xx < w_A && yy >= 0 && yy < h_A && (v3[0] + v3[1] + v3[2] < 500))
				result->SetPixel(j0, i0, Color(v3[0],v3[1],v3[2]));


		}
/* Just for paper */
	w_Result += RUN * 2;
	h_Result += RUN * 2;
	Bitmap *result_new = new Bitmap(w_Result, h_Result);
	for (i = 0; i < w_Result; ++i)
		for (j = 0; j < h_Result; ++j)
		{
			if (i < RUN || i >= w_Result - RUN || j < RUN || j >= h_Result - RUN)
				result_new->SetPixel(i,j,Color(255,255,255));
			else {
				result->GetPixel(i-RUN,j-RUN,&i_color);
				result_new->SetPixel(i,j,i_color);
			}
		}

	delete result;
	result = result_new;


	/*Release the heap memory. */
	delete [] RGB[0][0];
	delete [] RGB[0];
	delete [] RGB;

}

void global_translate(BYTE ** ppt_image, BYTE ** mimio_scaled_image, int *tranx1, int *trany1, Bitmap *&result)
{
	int REG = 20; // global search region

	int tranx, trany;

	result = new Bitmap(720, 540);
	int w = result->GetWidth();
	int h = result->GetHeight();


	int maxTotal = 0;
	for (int i = -REG; i < REG; i++)
		for (int j = -REG; j < REG; j++)
		{
			int total = 0;

			for (int x = 0 ; x < w; x ++)
				for (int y = 0; y < h; y ++)
				{
					if ( (y+j) < 0 || (y+j) > h-1 || (x+i) < 0 || (x+i ) > w-1)
						continue;

					if ((mimio_scaled_image[y+j][x+i] == ppt_image[y][x]) && (mimio_scaled_image[y+j][x+i] == BLACK))
						total ++;
				}

			if (total > maxTotal)
			{
				maxTotal = total;
				tranx = i;
				trany = j;
			}

		}

	BYTE **tempScaled = new_image(w,h);

	for (int x = 0 ; x < w; x ++)
		for (int y = 0; y < h; y ++)
		{
			if ( (y+trany) < 0 || (y+trany) > h-1 || (x+tranx) < 0 || (x+tranx ) > w-1)
					continue;

			tempScaled[y][x] = mimio_scaled_image[y+trany][x+tranx];
		}

	for (int x = 0 ; x < w; x ++)
		for (int y = 0; y < h; y ++)
			mimio_scaled_image[y][x] = tempScaled[y][x];


	*tranx1 = tranx;
	*trany1 = trany;

	// delete tempScaled needed.
}

void overlap_coarse(BYTE **ppt_image, BYTE **mimio_scaled, Bitmap *&result)
{
	int i, j;

	result = new Bitmap(720, 540);

	for (i = 0; i < 720; i++)
		for (j = 0; j < 540; j++)
			result->SetPixel(i,j,Color((1 - ppt_image[j][i])*255,(1 - ppt_image[j][i])*255,(1 - ppt_image[j][i])*255));

	for (i = 0; i < 720; i++)
		for (j = 0; j < 540; j++)
			if ( mimio_scaled[j][i] == BLACK) result->SetPixel(i,j,Color(255,0,0));
}

void overlap(BYTE **ppt_image, BYTE **mimio_orig, double *matrix_value, Bitmap *&result)
{
	int i, j;
	double R[10];
	for (i = 1; i < 9; i++) R[i] = matrix_value[ i + 7];

	result = new Bitmap(720, 540);

	for (i = 0; i < 720; i++)
		for (j = 0; j < 540; j++)
			result->SetPixel(i,j,Color((1 - ppt_image[j][i])*255,(1 - ppt_image[j][i])*255,(1 - ppt_image[j][i])*255));

	double div_temp;
	double v1,v2,v3, r1,r2, xx, yy;
	for (i = 0; i < 540; i++)
		for (j = 0; j < 720; j++)
		{
			div_temp = R[7] * j + R[8] * i + 1;
			xx = (R[1] * j + R[2] * i + R[3]) / div_temp + 1;
			yy = (R[4] * j + R[5] * i + R[6]) / div_temp + 1;
			r1 = xx - floor(xx); r2 = yy - floor(yy);
			if ( xx > 0 && xx + 1 < 600 && yy > 0 && yy + 1 < 400)
			{
				v1 = mimio_orig[(int)ceil(yy)][(int)floor(xx)] +
							(mimio_orig[(int)ceil(yy)][(int)ceil(xx)] - 
							mimio_orig[(int)ceil(yy)][(int)floor(xx)]) * r1;
				v2 = mimio_orig[(int)floor(yy)][(int)floor(xx)] +
							(mimio_orig[(int)floor(yy)][(int)ceil(xx)] -
							mimio_orig[(int)floor(yy)][(int)floor(xx)]) * r1;
				v3 = (v2 + (v1 - v2) * r2);
				if (v3 != 0 ) result->SetPixel(j,i,Color(255,0,0));
			}	
		}
}



