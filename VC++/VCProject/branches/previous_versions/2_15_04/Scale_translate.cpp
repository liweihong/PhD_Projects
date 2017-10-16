#include "VCProject.h"

struct corner{
	int x;
	int y;
	double dist;
};

double round(double f)
{
	return floor(f) + 0.5 > ceil(f) ? ceil(f) : floor(f);
}

void scale_translate(BYTE **ppt_image, BYTE **mimio_image, Bitmap *ppt_bitmap, Bitmap *mimio_bitmap, Bitmap *&newbitmap, double *params)
{
	int i,j;
	int ppt_H = ppt_bitmap->GetHeight();
	int ppt_W = ppt_bitmap->GetWidth();
	int mimio_H = mimio_bitmap->GetHeight();
	int mimio_W = mimio_bitmap->GetWidth();

	//Get the centroid of these 2 images;
	//*1 stands for ppt, *2 stands for mimio;
	int index1 = 0, index1_x = 0, index1_y = 0;
	int index2 = 0, index2_x = 0, index2_y = 0;

	for (i = 0; i < ppt_H ; i++)
		for (j = 0; j < ppt_W; j++)
		{
			if (ppt_image[i][j] == BLACK)
			{
				index1_x += j;
				index1_y += i;
				index1 ++;
			}
		}

	for (i = 0; i < mimio_H; i++)
		for (j = 0; j < mimio_W; j++)
		{
			if (mimio_image[i][j] == BLACK)
			{
				index2_x += j;
				index2_y += i;
				index2 ++;
			}
		}

	int centroid1_x = int(((float)index1_x)/index1);
	int centroid1_y = int(((float)index1_y)/index1);
	int centroid2_x = int(((float)index2_x)/index2);
	int centroid2_y = int(((float)index2_y)/index2);

	//Get the 4 coner points;
	double dist1, dist2,dist3,dist4;
	double init_dist1 = ppt_H * ppt_H + ppt_W * ppt_W;
	double init_dist2 = mimio_H * mimio_H + mimio_W * mimio_W;
	struct corner corners1[4] = { {1,1, init_dist1}, {1, ppt_W, init_dist1},
								{ppt_H,1,init_dist1},{ppt_H,ppt_W,init_dist1}};
	struct corner corners2[4] = { {1,1,init_dist2}, { 1, mimio_W, init_dist2},
								{mimio_H,1,init_dist2},{mimio_H,mimio_W,init_dist2}};

	for (i = 0; i < ppt_H; i++)
		for (j = 0; j < ppt_W; j++)
			if (ppt_image[i][j] == BLACK)
			{
				dist1 = (i-1)*(i-1) + (j-1)*(j-1);
				dist2 = (i-1)*(i-1) + (j-ppt_W)*(j-ppt_W);
				dist3 = (i-ppt_H)*(i-ppt_H) + (j-1)*(j-1);
				dist4 = (i-ppt_H)*(i-ppt_H) + (j-ppt_W)*(j-ppt_W);
				if (dist1 < corners1[0].dist ) 
				{
					corners1[0].dist=dist1;
					corners1[0].x = j;
					corners1[0].y = i;
				}

				if (dist2 < corners1[1].dist ) 
				{
					corners1[1].dist=dist2;
					corners1[1].x = j;
					corners1[1].y = i;
				}

				if (dist3 < corners1[2].dist )
				{
					corners1[2].dist=dist3;
					corners1[2].x = j;
					corners1[2].y = i;
				}
				if (dist4 < corners1[3].dist ) 
				{
					corners1[3].dist=dist4;
					corners1[3].x = j;
					corners1[3].y = i;
				}
			}

	for (i = 0; i < mimio_H; i++)
		for (j = 0; j < mimio_W; j++)
			if (mimio_image[i][j] == BLACK)
			{
				dist1 = (i-1)*(i-1) + (j-1)*(j-1);
				dist2 = (i-1)*(i-1) + (j-mimio_W)*(j-mimio_W);
				dist3 = (i-mimio_H)*(i-mimio_H) + (j-1)*(j-1);
				dist4 = (i-mimio_H)*(i-mimio_H) + (j-mimio_W)*(j-mimio_W);
				if (dist1 < corners2[0].dist ) 
				{
					corners2[0].dist=dist1;
					corners2[0].x = j;
					corners2[0].y = i;
				}

				if (dist2 < corners2[1].dist ) 
				{
					corners2[1].dist=dist2;
					corners2[1].x = j;
					corners2[1].y = i;
				}

				if (dist3 < corners2[2].dist )
				{
					corners2[2].dist=dist3;
					corners2[2].x = j;
					corners2[2].y = i;
				}
				if (dist4 < corners2[3].dist ) 
				{
					corners2[3].dist=dist4;
					corners2[3].x = j;
					corners2[3].y = i;
				}
			}

	double scales[4], scale=0.0,scale1=0.0;

	for (i = 0; i < 4; i ++)
	{
		scales[i] = sqrt(pow((centroid1_x - corners1[i].x),2) + pow((centroid1_y - corners1[i].y),2)) 
					/ sqrt(pow((centroid2_x - corners2[i].x),2) + pow((centroid2_y - corners2[i].y),2));

		scale1 += scales[i];
	}

	scale1 /= 4;
	scale = 1 / scale1;

	BYTE **mimio_orig;
	set_image(mimio_orig,mimio_bitmap);

	int trans_x = (int)floor(centroid1_x - centroid2_x * scale1);
	int trans_y = (int)floor(centroid1_y - centroid2_y * scale1);

	int scaled_H = (int)(mimio_H * scale1);
	int scaled_W = (int)(mimio_W * scale1);

	newbitmap = new Bitmap(scaled_W,scaled_H);

	// Bilinear interpolation
	double xx, yy;
	int fx, fy, v3;
	double v1,v2,v3_1,r1,r2;
 	for (i=0; i < scaled_H; i++)
		for (j=0; j < scaled_W; j++)
		{
			newbitmap->SetPixel(j,i,Color(255,255,255));
			xx = j * scale;
			yy = i * scale;
			fx = (int)floor(xx);
			fy = (int)floor(yy);
			r1 = xx - fx; r2 = yy - fy;
			if (i + trans_y > 0 && i + trans_y < scaled_H && j + trans_x > 0 && j + trans_x < scaled_W
				&& fx >= 0 && fx+1 < mimio_W && fy >= 0 && fy+1 < mimio_H)
			{
				v1 = mimio_orig[(int)ceil(yy)][(int)floor(xx)] +
							(mimio_orig[(int)ceil(yy)][(int)ceil(xx)] - 
							mimio_orig[(int)ceil(yy)][(int)floor(xx)]) * r1;
				v2 = mimio_orig[(int)floor(yy)][(int)floor(xx)] +
							(mimio_orig[(int)floor(yy)][(int)ceil(xx)] -
							mimio_orig[(int)floor(yy)][(int)floor(xx)]) * r1;
				v3_1 = (v2 + (v1 - v2) * r2);
				if (v3_1 != 0 ) v3 = BLACK; else v3 = WHITE;
				newbitmap->SetPixel(j+trans_x,i+trans_y,Color((1 - v3)*255, (1 - v3)*255,(1 - v3)*255));
			}
		}


	/* NN interpolation
	int xx, yy;
 	for (i=0; i < scaled_H; i++)
		for (j=0; j < scaled_W; j++)
		{
			newbitmap->SetPixel(j,i,Color(255,255,255));
			xx = (int)(j * scale);
			yy = (int)(i * scale);
			if (i + trans_y > 0 && i + trans_y < scaled_H && j + trans_x > 0 && j + trans_x < scaled_W
				&& xx > 0 && xx < mimio_W && yy > 0 && yy < mimio_H)
				newbitmap->SetPixel(j+trans_x,i+trans_y,Color((1 - mimio_orig[yy][xx])*255, (1-mimio_orig[yy][xx])*255,(1-mimio_orig[yy][xx])*255));
		}
	*/

	delete [] mimio_orig[0];
	delete [] mimio_orig;

	params[0] = scale1;
	params[1] = trans_x;
	params[2] = trans_y;
}

