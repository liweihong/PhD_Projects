#include "VCProject.h"

const int NAVI_SIZE = 20;
const int VALID_P = 50;
const double MATCHED_RATIO = 0.5;
const int LEN = 120;

struct comp_coord{
	int y;
	int x;
	comp_coord *next;
} coord1;  // Define component coords list.

struct coord{
	int yMin;
	int yMax;
	int xMin;
	int xMax;
	comp_coord *head; // This pointer points to the component coords list.
	coord *next;
} coord2;

void matching(BYTE **ppt_image, BYTE **mimio_image, Bitmap *ppt_bitmap, Bitmap *mimio_bitmap, Bitmap *&matching, Bitmap *&matched, double *matrix_val)
{

	double scale1 = matrix_val[0];
	double scale = 1 / scale1;
	int trans_x = (int) matrix_val[1];
	int trans_y = (int) matrix_val[2];

	int h = mimio_bitmap->GetHeight();
	int w = mimio_bitmap->GetWidth();

	int x, y;

	// Set up the LUT for the connective component
	BYTE **item_LUT = new BYTE* [h];
	item_LUT[0] = new BYTE [h * w];
	for (y=0; y<h; y++)
		item_LUT[y] = item_LUT[0] + y * w; 
	for (y=0; y < h; y++)
		for (x=0; x<w; x++)
			item_LUT[y][x] = 0;


	//Segment the items.
	struct coord *items, *cur, *items_list = NULL;
	struct comp_coord *component_ptr = NULL;
	int item_num = 0, segment_pixels = 0;
	int yMin,yMax,xMin,xMax;

	for (y=0; y<h; y++)
	{
		for(x=0; x<w; x++)
		{
			if (mimio_image[y][x] == BLACK && item_LUT[y][x] == 0)
			{
				yMin = yMax = y; xMin = xMax = x;
				item_seg(y,x,mimio_image,item_LUT,yMin,xMin,yMax,xMax,segment_pixels,component_ptr);
				items = new coord;
				items->head = component_ptr; items->yMin = yMin; items->xMin = xMin; items->yMax = yMax; items->xMax = xMax; items->next = NULL;
				component_ptr = NULL;
				item_num ++;
				if (items_list == NULL) 
					cur = items_list = items;
				else { 
					cur->next = items;
					cur = cur->next;
				}

			}
		}

	}

	delete [] item_LUT[0];
	delete [] item_LUT;

	items = items_list;

	int good_matched_num = 0;
	struct coord *good_items = NULL, *temp_ptr = NULL;

	for (int num = 0; num < item_num; num++)
	{
		yMin = items_list->yMin;
		yMax = items_list->yMax;
		xMin = items_list->xMin;
		xMax = items_list->xMax;


		if ( yMax + xMax - yMin - xMin < VALID_P)
		{
			items_list = items_list->next;
			continue; // 1st condition
		}

		int temp_matched = 0, max_matched = 0;
		int mean_x = (xMin+xMax)/2;
		int mean_y = (yMin+yMax)/2;
		int matched_x, matched_y;
		int x0, y0;

		int i, j;
		int t_w = xMax - xMin + 1;
		int t_h = yMax - yMin + 1;
		struct comp_coord *comp_ptr = items_list->head;
		BYTE **mimio_component = new BYTE *[t_h];
		mimio_component[0] = new BYTE[t_h * t_w];
		for (i = 0; i<t_h; i++)
			mimio_component[i] = mimio_component[0] + t_w * i;
		for (i = 0; i < t_h; i++)
			for (j = 0; j < t_w ; j++)
				mimio_component[i][j] = 0;
		while (comp_ptr)
		{
			mimio_component[comp_ptr->y - yMin][comp_ptr->x - xMin] = BLACK;
			comp_ptr = comp_ptr->next;
		}  // Get the component from the list.

		for (y = mean_y - NAVI_SIZE; y < mean_y + NAVI_SIZE; y++)
			for (x = mean_x - NAVI_SIZE; x < mean_x + NAVI_SIZE; x++)
			{
				for (y0 = yMin; y0 < yMax; y0++)
					for (x0 = xMin; x0 < xMax; x0++)
						if (y0 + y - mean_y < 0 || x0 + x - mean_x < 0  || y0 + y - mean_y > h || x0 + x - mean_x > w )
							continue;
						else if ( mimio_component[y0 - yMin][x0 - xMin] * ppt_image[y0 + y - mean_y ][x0 + x - mean_x] == 1 ) temp_matched ++;
						//else if ( mimio_image[y0][x0] * ppt_image[y0 + y - mean_y ][x0 + x - mean_x] == 1 ) temp_matched ++;

				if (temp_matched > max_matched)
				{
					max_matched = temp_matched;
					matched_x = x;
					matched_y = y;
				}

				temp_matched = 0;
			}

		int valide_pixels = 0;
		for (y0 = yMin; y0 < yMax; y0++)
			for (x0 = xMin; x0 < xMax; x0++)
				if (mimio_image[y0][x0] == 1) valide_pixels ++;

		if (((float)max_matched)/valide_pixels < MATCHED_RATIO) 
		{
			items_list = items_list->next;
			continue; // 2nd condition;
		}

		for (y = matched_y - ((yMax - yMin) / 2) - 1; y < matched_y + ((yMax - yMin) / 2) + 2; y++)
		{
			ppt_image[y][matched_x - (xMax - xMin)/2 -1] = 1;
			ppt_image[y][matched_x + (xMax - xMin)/2 +1] = 1;
		}
		for (x = matched_x - ((xMax - xMin) / 2) - 1; x < matched_x + ((xMax - xMin) / 2) + 2; x++)
		{
			ppt_image[matched_y - (yMax - yMin)/2 -1][x] = 1;
			ppt_image[matched_y + (yMax - yMin)/2 +1][x] = 1;
		}

		good_matched_num ++;

		temp_ptr = new coord;
		temp_ptr->xMax = matched_x;
		temp_ptr->yMax = matched_y;
		temp_ptr->xMin = mean_x;
		temp_ptr->yMin = mean_y;

		if (good_items == NULL) 
			cur = good_items = temp_ptr;
		else { 
			cur->next = temp_ptr;
			cur = cur->next;
		}

		fout<<matched_x<<" "<<matched_y<<" "<<mean_x<<" "<<mean_y<<endl;
		//break;

		items_list = items_list -> next;

		delete [] mimio_component[0];
		delete [] mimio_component;


	}

	items_list = items;

	for ( num = 0; num < item_num; num++)
	{
		yMin = items_list->yMin;
		yMax = items_list->yMax;
		xMin = items_list->xMin;
		xMax = items_list->xMax;

		for (y = yMin-1; y<yMax+2; y++)
		{
			mimio_image[y][xMin-1] = 1;
			mimio_image[y][xMax+1] = 1;
		}
		for (x=xMin-1; x < xMax+2; x++)
		{
			mimio_image[yMin-1][x] = 1;
			mimio_image[yMax+1][x] = 1;
		}

		items_list = items_list->next;
	}


	// Calculate the Matrix and its Inverse value.
	// Invariant: 
	// matrix_value[0] thru matrix_value[7] store the matrix value
	// matrix_value[8] thru matrix_value[16] = 1.0 store the inverse value.

	double *dest = new double[ 2 * good_matched_num ];
	double *mimio = new double[ 2 * good_matched_num ];

	temp_ptr = good_items;
	for (int i = 0; i < good_matched_num; i++)
	{
		dest[i*2] = temp_ptr->xMax;
		dest[i*2 + 1] = temp_ptr->yMax;
		mimio[i*2] = (temp_ptr->xMin - trans_x) * scale;
		mimio[i*2 + 1] = (temp_ptr->yMin - trans_y) * scale;
		temp_ptr = temp_ptr->next;
	}

	mwArray Coord_dest(2,good_matched_num,dest);
	mwArray Coord_mimio(2,good_matched_num,mimio);
	mwArray *result_matrix_inv = new mwArray;
	mwArray result_matrix = matrix_value(result_matrix_inv,Coord_dest,Coord_mimio,good_matched_num);
	
	double *p_matrix = mxGetPr(result_matrix.GetData());
	for (i = 0; i < 8; i++)
	{
		fout<<"R["<<i+1<<"] = "<<p_matrix[i]<<endl;
		matrix_val[i] = p_matrix[i];
	}

	p_matrix = mxGetPr(result_matrix_inv->GetData());
	for (i = 0; i < 9; i++)
	{
		fout<<"R_Inv["<<i+1<<"] = "<<p_matrix[i]<<endl;
		matrix_val[i + 8] = p_matrix[i];
	}

//	Save2File("mimio_matching.bmp",		mimio_image,		&clsid);

//	Save2File("ppt_matched.bmp",		ppt_image,		&clsid);

}

int item_seg(int y, int x, BYTE **image, BYTE **item_LUT, int& yMin, int& xMin, int& yMax, int& xMax, int& seg_pixels, struct comp_coord *&comp_ptr)
{
	if (item_LUT[y][x] == 1) return seg_pixels;

	struct comp_coord *temp_ptr = new comp_coord;
	temp_ptr->x = x;
	temp_ptr->y = y;
	temp_ptr->next = NULL;

	if (comp_ptr == NULL)
		comp_ptr = temp_ptr;
	else {
		temp_ptr->next = comp_ptr;
		comp_ptr = temp_ptr;
	}

	item_LUT[y][x] = 1;
	seg_pixels ++;
	
	if (image[y-1][x] == 1) 
	{
		if (yMin > y-1) yMin = y-1;
		seg_pixels = item_seg(y-1,x,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}
	if (image[y-1][x+1] == 1) 
	{
		if (yMin > y-1) yMin = y-1;
		if (xMax < x+1) xMax = x+1;
		seg_pixels = item_seg(y-1,x+1,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}
	if (image[y][x+1] == 1) 
	{
		if (xMax < x+1) xMax = x+1;
		seg_pixels = item_seg(y,x+1,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}
	if (image[y+1][x+1] == 1) 
	{
		if (yMax < y+1) yMax = y+1;
		if (xMax < x+1) xMax = x+1;
		seg_pixels = item_seg(y+1,x+1,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}
	if (image[y+1][x] == 1) 
	{
		if (yMax < y+1) yMax = y+1;
		seg_pixels = item_seg(y+1,x,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}
	if (image[y+1][x-1] == 1) 
	{
		if (yMax < y+1) yMax = y+1;
		if (xMin > x-1) xMin = x-1;
		seg_pixels = item_seg(y+1,x-1,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}
	if (image[y][x-1] == 1) 
	{
		if (xMin > x-1) xMin = x-1;
		seg_pixels = item_seg(y,x-1,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}
	if (image[y-1][x-1] == 1) 
	{
		if (xMin > x-1) xMin = x-1;
		if (yMin > y-1) yMin = y-1;
		seg_pixels = item_seg(y-1,x-1,image,item_LUT,yMin,xMin,yMax,xMax,seg_pixels,comp_ptr);
	}

	return seg_pixels;
}

