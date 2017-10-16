#include <math.h>
#include <vector>
#include "hough.h"
#include "project.h"
#ifndef LINUX
#include <float.h>
#endif

using namespace std;

#define UCHARCH_TYPE 1
#define INTCH_TYPE 4
#define MaxGray 255
//#define DEBUG_KD

int MygetVertNorm(int cell_num, vector<int> *vec_cell, int itmp, vector3f norm1,int ptxWidth,int ptxHeight,
                  float *ptxX, float *ptxY,float *ptxZ, vector3f avgXYDir, int round);
void getFVM(vector3f *norms, int normCount, vector3f outNorm);
float Relation(vector3f u, vector3f v);

BOOL bFVM = FALSE;
float sigma = .4;
float min_x = 10000;
float max_x = -10000;
float min_y = 10000;
float max_y = -10000;
float min_z = 10000;
float max_z = -10000;
vector<float> vec_data;


st_table *st_param2indx = st_init_table(st_ptrcmp, st_ptrhash);

/* Command: Project.exe -f 10.0 -F 5 -L ss -w 1 -Z trans_4.0.txt

   Parameters:
          -f : define the size of the cell.
          -F : define the maximum number of top accumulation counter.
          -L : use New method (old method if no -L)
          -w : down-sampling ratio (for new method, -w 1, otherwise, core dumped).
          -Z : the input point cloud file -- MUST BE the LAST one

   Output file: normal.iv
*/

#if 0
void hough_plane_cb()
{
	char filename[MXSTRLEN];
	imageP I;
	pointCloudP ptCloud;

	vector3f *topNorms;
	const int topCount=5;
	int *planeIds;
	myPlaneP topPlanes;

	printf("\n\nSelect ptx file:\n\n");
	UI_fileBrowser(1, NULL);        /* allow browser to show all files */
	sprintf(filename, "%s/%s", FBrowserDir, FBrowserFile);
	printf("%s\n", FBrowserFile);

	ptCloud=(pointCloudP)malloc(sizeof(pointCloudS));
	loadPtxFile(filename,ptCloud);
	topNorms=(vector3f*)malloc(sizeof(vector3f)*topCount);
	I=IP_allocImage(360,180,UCHARCH_TYPE);

	/* find the normals with the most vote.	 */
	bFVM = FALSE;
	hough_plane(I, ptCloud, topCount,topNorms,1);
	bFVM = TRUE;
	sigma = UI_askDouble("sigma :", 0.0, 1000.0, 0.4);
	hough_plane(I, ptCloud, topCount,topNorms,1);
	bFVM = FALSE;

}
#else
void hough_plane_cb(char *filename)
{
	imageP I;
	pointCloudP ptCloud;

	vector3f *topNorms;
	int topCount=5;
    if ( gbl_opts->input_prefix )
       topCount = atoi(gbl_opts->input_prefix);
    
	int *planeIds;
	myPlaneP topPlanes;

        assert(filename);
	ptCloud=(pointCloudP)malloc(sizeof(pointCloudS));
	loadPtxFile(filename,ptCloud);
	topNorms=(vector3f*)malloc(sizeof(vector3f)*topCount);
	I=IP_allocImage(360,180,UCHARCH_TYPE);

	/* find the normals with the most vote.	 */
	bFVM = FALSE;
        printf("Starting hough_plane computing with %d data points....\n", ptCloud->ptCount);
	hough_plane(I, ptCloud, topCount,topNorms,1);
    exit(0);
}
#endif

//void IP_threshold_mine(imageP I1, float 127.0, 255.0)
void IP_threshold_mine(imageP I1, float thresh, float max_val)
{
   uchar *im = (uchar *)I1->buf[0];

   int c_zero = 0;
   int c_less = 0;
   int c_more = 0;
   for (int i = 0; i < I1->width; i ++ )
      for (int j = 0; j < I1->height; j ++ )
      {
         int idx = I1->height*i + j;
         if ( im[idx] == 0.0 ) {
            c_zero ++;
         } else if ( im[idx] <= thresh ) {
            c_less ++;
         } else
            c_more ++;
      }

   printf("Total %d: zero %d, less %d, more %d, sum %d\n", I1->width*I1->height, c_zero, c_less, c_more, c_zero+c_less+c_more);
   
}

extern void generate_iv_for_slab_set_points(FILE *fd_new, vector<double> &data );
/*	Finds the normal at each point. Converts the normal from cartesian to spherical.
	Stores the values in a table and uses it to find the top normals (with the most vote).
*/
/*
For the implementation, we avoid computation on the whole large scale point cloud
data by computing the normal for each point locally. 
For each point $P(x_p, y_p, z_p)$, we can compute its normal based on its neighbors
whose distance to $P$ is less than a radium $d$. 
To do this, a $kd$ tree can be constructed based on the input point cloud, 
which can then be used to obtain the neighbor points with distance $\le d$. 
In other words, the parameter $\rho \le d$, which make the discretization much easier.
As we know, the other two parameters are bounded with $0 \le \theta < 2\pi$ 
and $0 \le \phi \le \pi$, we can easily discretize the parameters and compute
the votes based on the \Eq{ht_plane} for any point $P$.
For each local computation, a maximum vote would give the normal based on $\theta$
and $\phi$ for the point $P$.
For each point $P$, we will get a vote on $\theta$ and $\phi$ for its normal.
After going through all input data points, the big accumulated votes would 
give the major planes.

 */
void hough_plane(imageP I0, pointCloudP ptCloud, int topNum,vector3f *topNorms, int bDisplay)
{
	int i, j, k, tmp1;
	float norm1[3], norm2[3];
	int phi, theta;
	float x,y,z;
	float c=0, c2=0;
	int nextimage=0, *iTmp, acc[180][360]={0};
	imageP I1, I2;
	char *pixel;
	vector3f v1, v2;
	boundaryP bn;

	int *maxAcc;
	int *maxp, *maxt;

	vector3f avgXYDir;
	int ptxWidth, ptxHeight;

	ptxWidth=ptCloud->width;
	ptxHeight=ptCloud->height;
	maxAcc=(int*)malloc(sizeof(int)*topNum);
	maxp=(int*)malloc(sizeof(int)*topNum);
	maxt=(int*)malloc(sizeof(int)*topNum);

	memset(maxAcc,0,sizeof(int)*topNum);
	memset(maxp,0,sizeof(int)*topNum);
	memset(maxt,0,sizeof(int)*topNum);
	for(phi=0;phi<180;phi++)
		for(theta=0;theta<360;theta++)
			acc[phi][theta]=0;

    int ht_method = 0;  // old method
    if ( gbl_opts->BPA_parameter_file )
       ht_method = 1;   // new method

    printf("Entering findAvgXYDir...\n");
    // This is useless or only used for old method?
    if ( ht_method == 0 )
	findAvgXYDir(ptCloud->width,ptCloud->height,ptCloud->ptCount,ptCloud->ptxX,ptCloud->ptxY,ptCloud->ptxZ
		,ptCloud->ptxR,avgXYDir);

    // construct the kd-like tree
    float kd_int = 0.1;
    if ( gbl_opts->output_prefix )
       kd_int = atof(gbl_opts->output_prefix);
    
    // the total number of cells
    int total_cell_x = (int) ((max_x - min_x ) / kd_int);
    int total_cell_xy = total_cell_x * (int) ((max_y - min_y ) / kd_int);
    int total_cell_xyz = total_cell_xy * (int ) ((max_z - min_z ) / kd_int);
    int total_cell = total_cell_xyz;
                     
                     
    printf("The bounding box is [%f, %f][%f, %f][%f, %f], \n with the interval %f of total [%d] cells\n",
           min_x, max_x, min_y, max_y, min_z,max_z, kd_int, total_cell);

    if ( total_cell < 0 ) {
       printf("Exiting due to negative total cell.\n");
       exit(0);
    }

    vector<int> *vec_cell = NULL;
    st_table *st_no2list = st_init_table(st_ptrcmp, st_ptrhash); /* map from cell id to data point list */
    st_table *st_cell2norm = st_init_table(st_ptrcmp, st_ptrhash); /* map from cell id to result/normal */

    /* assign each data point to each cell in kd tree.
     * compute for each point, its index/cell_id, add it to the st_no2list table
     */
    int max_cell_points = 0;
    int total_over_cell = 0;
    for ( i = 0; i < vec_data.size(); i += 3) {
       float x = vec_data[i];
       float y = vec_data[i+1];
       float z = vec_data[i+2];

       int z_index = (int) ((z - min_z)/kd_int) ;
       int y_index = (int) ((y - min_y)/kd_int) ;
       int cell_num =  z_index * total_cell_xy + y_index * total_cell_x + (x - min_x)/kd_int;
       if ( cell_num >= total_cell ) {
             total_over_cell ++;
             continue;
             //cell_num = total_cell - 1;
       }

       if ( !st_lookup(st_no2list, (char *)cell_num, (char **)&vec_cell)) {
          vec_cell = new vector<int> ();
          st_insert(st_no2list, (char *)cell_num, (char *)vec_cell);
       }
       vec_cell->push_back(i);
       
       if ( vec_cell->size() > max_cell_points )
          max_cell_points = vec_cell->size();
    }
    printf("Maximum # of points in one cell is [%d] with total [%d] non-zero cells and %d overcell points\n", max_cell_points,
           st_count(st_no2list), total_over_cell);

#ifndef DEBUG_KD
    total_over_cell = 0;
    int total_valid_pts = 0;
    int total_count_pts = 0;
    if ( ht_method ) { // new mthod here.
       
       /* new method for getting norms at each vertex.	*/
       printf("Entering normals ... [%d %d] total [%d]\n", ptxWidth, ptxHeight, ptCloud->ptCount);
       for ( i = 0; i < vec_data.size(); i += 3) {
          float x = vec_data[i];
          float y = vec_data[i+1];
          float z = vec_data[i+2];
       
          int z_index = (int) ((z - min_z)/kd_int) ;
          int y_index = (int) ((y - min_y)/kd_int) ;
          int cell_num =  z_index * total_cell_xy + y_index * total_cell_x + (x - min_x)/kd_int;
          if ( cell_num >= total_cell ) {
             total_over_cell ++;
             continue;
             //cell_num = total_cell - 1;
          }

          assert ( st_lookup(st_no2list, (char *)cell_num, (char **)&vec_cell));

          // only if there are more than 2 points in the cell, can we compute the cross().
          if ( vec_cell->size() >= 3 )  
          {
             float *nn_vv;
             if ( !st_lookup(st_cell2norm, (char *)vec_cell, (char **)&nn_vv) ) {
                MygetVertNorm(cell_num, vec_cell, i/3 ,norm1,ptxWidth,ptxHeight,ptCloud->ptxX,ptCloud->ptxY,ptCloud->ptxZ,avgXYDir,1);
                nn_vv = (float *) malloc (sizeof(float) * 3);
                nn_vv[0] = norm1[0];
                nn_vv[1] = norm1[1];
                nn_vv[2] = norm1[2];
                st_insert(st_cell2norm, (char *)vec_cell, (char *)nn_vv);
             } else {
                norm1[0] = nn_vv[0];
                norm1[1] = nn_vv[1];
                norm1[2] = nn_vv[2];
             }

             
             total_count_pts ++;
             if(convToSphere(norm1, &theta, &phi)!=-1) {
                acc[phi][theta]++;
                total_valid_pts ++;
                //printf("%f %f %f -- %d %d\n", norm1[0], norm1[1], norm1[2], theta, phi);

                int param = phi*360 + theta;
                st_table *st_nums;
                if ( !st_lookup(st_param2indx, (char *)param, (char **)&st_nums) ) {
                   st_nums = st_init_table(st_numcmp, st_numhash);
                   st_insert(st_param2indx, (char *)param, (char *)st_nums);
                   
                } 
                st_insert(st_nums, (char *)(i/3), (char *)NULL);
             } else {
                //printf("BAD: %f %f %f\n", norm1[0], norm1[1], norm1[2]);
             }
          }
       }

    }else {
       
       /* old method for getting norms at each vertex.	*/
       printf("Entering normals ... [%d %d]\n", ptxWidth, ptxHeight);
       for(i=0;i<ptxWidth;i++) {
          for(j=0;j<ptxHeight;j++) {
             tmp1=i*ptxHeight+j;
             getVertNorm(tmp1,norm1,ptxWidth,ptxHeight,ptCloud->ptxX,ptCloud->ptxY,ptCloud->ptxZ,avgXYDir,0);
			
             if(convToSphere(norm1, &theta, &phi)!=-1) {
                acc[phi][theta]++;

                int param = phi*360 + theta;
                st_table *st_nums;
                if ( !st_lookup(st_param2indx, (char *)param, (char **)&st_nums) ) {
                   st_nums = st_init_table(st_numcmp, st_numhash);
                   st_insert(st_param2indx, (char *)param, (char *)st_nums);
                   
                } 
                st_insert(st_nums, (char *)tmp1, (char *)NULL);
             }
          }
       }
    }

	/* find the top maxima values of acc (used to scale the values to a range from 0-255)
		to create the bw image of their transform... */
    printf("\nEntering top maxima with %d/%d valid points...\n", total_valid_pts, total_count_pts);
	iTmp=(int*)malloc(sizeof(int)*topNum);
	for(phi=0;phi<180;phi++) {
		for(theta=0;theta<360;theta++) {
			for(k=0;k<topNum;k++) {
				if(acc[phi][theta]>maxAcc[k]) {
					if(k<topNum-1) {
						memcpy(iTmp, &maxAcc[k],sizeof(int)*(topNum-k-1));
						memcpy(&maxAcc[k+1], iTmp,sizeof(int)*(topNum-k-1));

						memcpy(iTmp, &maxp[k],sizeof(int)*(topNum-k-1));
						memcpy(&maxp[k+1], iTmp,sizeof(int)*(topNum-k-1));

						memcpy(iTmp, &maxt[k],sizeof(int)*(topNum-k-1));
						memcpy(&maxt[k+1], iTmp,sizeof(int)*(topNum-k-1));

					}
					maxAcc[k]=acc[phi][theta];
					maxp[k]=phi;
					maxt[k]=theta;
					break;
				}
			}
		}
	}

	pixel=(char *)I0->buf[0];
    if ( maxAcc[topNum-1] < 3 ) {
       printf("ERROR: the value of maxAcc [%d] [%d, %d, %d]should be bigger than 3.\n", maxAcc[topNum-1], maxAcc[0], maxAcc[1], maxAcc[2]);
       exit(0);
          
    }

    vector<int> vec_acc;
    st_table *st_p2p = st_init_table(st_ptrcmp, st_ptrhash);
    printf("0:: Printing normals ...\n");
    for(i=0;i<topNum;i++) {
       theta=maxt[i];
       phi=maxp[i];
       convToCartesian(theta, phi, &x, &y, &z);
       printf("TOP %d [vote:%d]: [%.3f, %.3f, %.3f] <-> [%d, %d]\n", i+1, maxAcc[i], x, y, z, theta, phi);

       int merged_id = -1;
       for (int j = 0; j < vec_acc.size(); j ++ ) {
          int s_the = maxt[vec_acc[j]];
          int s_phi = maxp[vec_acc[j]];

          if ( abs(s_the - theta ) == 180 || abs(s_the - theta ) == 179 ||
               abs(s_the - theta ) == 360 || abs(s_the - theta ) == 359 ||
               abs(s_the - theta ) <= 1) {
             if ( abs(s_phi - phi) <= 1) {
                merged_id = vec_acc[j];
                break;
             }
          }
       }

       if ( merged_id != -1 ) {
          printf("Merged with top %d\n", merged_id);

          vector<int> *vec_arr;
          if ( !st_lookup(st_p2p, (char *)merged_id, (char **)&vec_arr) ) {
             vec_arr = new vector<int> ();
             st_insert(st_p2p, (char *)merged_id, (char *)vec_arr);
          }
          vec_arr->push_back(i);
          
       } else {
          vec_acc.push_back(i);
       }
       
    }
    
    printf("1:: Printing normals after merging ...\n");
    for(i=0;i<vec_acc.size();i++) {
       theta=maxt[vec_acc[i]];
       phi=maxp[vec_acc[i]];
       convToCartesian(theta, phi, &x, &y, &z);
       printf("TOP %d [vote:%d]: [%.3f, %.3f, %.3f] <-> [%d, %d]\n", i+1, maxAcc[i], x, y, z, theta, phi);
    }

#endif // end DEBUG_KD
    
    // dump the point to iv files
    FILE *fd_new = fopen("normal.iv", "w");
    assert(fd_new);

    {
       fprintf(fd_new, "#Inventor V2.1 ascii\n");
       fprintf(fd_new, "\n");
       fprintf(fd_new, "\n");
       fprintf(fd_new, "Separator {\n");
       fprintf(fd_new, "\n");
       fprintf(fd_new, "  LightModel {\n");
       fprintf(fd_new, "    model BASE_COLOR\n");
       fprintf(fd_new, "\n");
       fprintf(fd_new, "  }\n"); // LightModel
    }

    
    
	/* convert the normals from spherical back to cartesian format */
    printf("Printing normals ...\n");
    vector<double> slice_data;

#ifndef DEBUG_KD       
    //for(i=0;i<topNum;i++) {
    for(int ii=0;ii<vec_acc.size();ii++) {
       /*
		theta=(bn[i].xmax+bn[i].xmin)/2.0+.5;
		phi=(bn[i].ymax+bn[i].ymin)/2.0+.5;
       */
       i = vec_acc[ii];
       theta=maxt[i];
       phi=maxp[i];
		convToCartesian(theta, phi, &x, &y, &z);
		topNorms[i][0]=x;
		topNorms[i][1]=y;
		topNorms[i][2]=z;

        // dump the data
        int key = phi * 360 + theta;
        st_table *st_nums;
        int *numIdx, *dummy;
        st_generator *gen;
        assert(st_lookup(st_param2indx, (char *)key, (char **)&st_nums));

        {
           st_foreach_item(st_nums, gen, (char **) &numIdx, (char **) &dummy) {
              slice_data.push_back(ptCloud->ptxX[(int)numIdx]);
              slice_data.push_back(ptCloud->ptxY[(int)numIdx]);
              slice_data.push_back(ptCloud->ptxZ[(int)numIdx]);
           }

           vector<int> *vec_arr;
           if ( st_lookup(st_p2p, (char *)i, (char **)&vec_arr) ) {
              printf("TOP %d found merged ones, total %d.\n", i, vec_arr->size());
              for ( int k = 0; k < vec_arr->size(); k ++ ) {
                 phi = maxp[vec_arr->at(k)];
                 theta = maxt[vec_arr->at(k)];
                 key = phi * 360 + theta;
                 assert(st_lookup(st_param2indx, (char *)key, (char **)&st_nums));
                 st_foreach_item(st_nums, gen, (char **) &numIdx, (char **) &dummy) {
                    slice_data.push_back(ptCloud->ptxX[(int)numIdx]);
                    slice_data.push_back(ptCloud->ptxY[(int)numIdx]);
                    slice_data.push_back(ptCloud->ptxZ[(int)numIdx]);
                 }
              }
           }
        }
#else

        for (int k = 0; k < total_cell; k ++ ) {
           if ( st_lookup(st_no2list, (char *)k, (char **)&vec_cell) ) {
              for ( int i = 0; i < vec_cell->size(); i ++ ) {
                 slice_data.push_back(vec_data[vec_cell->at(i)]);
                 slice_data.push_back(vec_data[vec_cell->at(i)+1]);
                 slice_data.push_back(vec_data[vec_cell->at(i)+2]);
              }
           } else
              continue;
        
#endif        
        
        generate_iv_for_slab_set_points(fd_new, slice_data);

        printf("TOP %d: [%.3f, %.3f, %.3f] <-> [%d, %d]\n", i+1, x, y, z, theta, phi);
        //printf("TOP %d: [%.3f, %.3f, %.3f] with %d points\n", i+1, x, y, z, slice_data.size() / 3);
        slice_data.clear();

	}

    fprintf(fd_new, "\n}\n");
    fclose(fd_new);

#ifndef DEBUG_KD
	free(maxAcc);
	free(maxp);
	free(maxt);
	free(iTmp);
#endif
    
	if(!bDisplay)
		return;

}

void normalize(vector3f vec, vector3f normVec)
{
	double length;
	
	length=sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);

#ifdef LINUX
        if ( isnan(length) )
#else
        if ( _isnan(length) )
#endif
        {
           normVec[0]=vec[0]; normVec[1]=vec[1]; normVec[2]=vec[2];
           return;
        }
        
	if(length==1 || length==0 || (length > -0.1 && length < 0.1)) {
		normVec[0]=vec[0]; normVec[1]=vec[1]; normVec[2]=vec[2];
                return;
	}
	normVec[0]=vec[0]/length;
	normVec[1]=vec[1]/length;
	normVec[2]=vec[2]/length;
}


/*	This function was supposed to find the average distance of points in the ptx file
	from each other. It instead finds the average distance in the X and Y directions.
	It needs to be changed...
*/
void findAvgXYDir(int ptxWidth,int ptxHeight,int ptxPtCount,
				  float *ptxX, float *ptxY, float *ptxZ, float *ptxR, vector3f avgXYDir)
{
	int i, j;
	int xstart, xend, ystart, yend;
	int samplenum, itmp1, itmp2, vCount;
	vector3f v2, prevVec={0};
	float avgXdir, avgYdir, tmpAvgX, tmpAvgY;

	//samplenum=MIN(10,ptxHeight);
	samplenum=10;
	avgXdir=0;
	for(j=0;j<samplenum;j++) {
		xstart=rand()%(ptxPtCount/2);
		xend=ptxPtCount/2 + rand()%(ptxPtCount/2);
		memset(prevVec,0,sizeof(vector3f));
		tmpAvgX=vCount=0;
		for(i=xstart;i<xend-1;i++) {
           itmp1=i;
           itmp2=(i + rand()%(ptxPtCount/2)) % ptxPtCount;
			if(!IsNullPoint(ptxX[itmp1],ptxY[itmp1],ptxZ[itmp1]) && 
				!IsNullPoint(ptxX[itmp2],ptxY[itmp2],ptxZ[itmp2])) {

				v2[0]=ptxX[itmp2]-ptxX[itmp1];
				v2[1]=ptxY[itmp2]-ptxY[itmp1];
				v2[2]=ptxZ[itmp2]-ptxZ[itmp1];
                /*
				if(!IsNullPoint(prevVec[0],prevVec[1],prevVec[2]))
					if(Angle(prevVec,v2)>=10)
						continue;
                */
				vCount++;
				prevVec[0]=v2[0]; prevVec[1]=v2[1]; prevVec[2]=v2[2];
				tmpAvgX=(tmpAvgX*(vCount-1)+(ptxX[itmp2]-ptxX[itmp1]))/vCount;
			}
		}

		if(vCount<(xend-xstart+1)/2) {
			j--;
			continue;
		}
		avgXdir=(avgXdir*j+tmpAvgX)/(j+1);
	}


	/* get avgYdir	*/
	//samplenum=MIN(10,ptxWidth);
	samplenum=10;
	avgYdir=0;
	for(j=0;j<samplenum;j++) {
		xstart=rand()%(ptxPtCount/2);
		xend=ptxPtCount/2 + rand()%(ptxPtCount/2);
		memset(prevVec,0,sizeof(vector3f));
		tmpAvgX=vCount=0;
		for(i=xstart;i<xend-1;i++) {
           itmp1=i;
           itmp2=(i + rand()%(ptxPtCount/2)) % ptxPtCount;
			if(!IsNullPoint(ptxX[itmp1],ptxY[itmp1],ptxZ[itmp1]) && 
				!IsNullPoint(ptxX[itmp2],ptxY[itmp2],ptxZ[itmp2])) {

				v2[0]=ptxX[itmp2]-ptxX[itmp1];
				v2[1]=ptxY[itmp2]-ptxY[itmp1];
				v2[2]=ptxZ[itmp2]-ptxZ[itmp1];
                /*
				if(!IsNullPoint(prevVec[0],prevVec[1],prevVec[2]))
					if(Angle(prevVec,v2)>=10)
						continue;
                */
				vCount++;
				prevVec[0]=v2[0]; prevVec[1]=v2[1]; prevVec[2]=v2[2];
				tmpAvgY=(tmpAvgY*(vCount-1)+(ptxY[itmp2]-ptxY[itmp1]))/vCount;
			}
		}

		if(vCount<(yend-ystart+1)/2) {
			j--;
			continue;
		}
		avgYdir=(avgYdir*j+tmpAvgY)/(j+1);
	}

	avgXYDir[0]=avgXdir;
	avgXYDir[1]=avgYdir;
	avgXYDir[2]=0;

    printf("avgDir: %f %f %f\n",avgXYDir[0],avgXYDir[1],avgXYDir[2]);
}

/*	Finds the normal of the point located at itmp	*/
int getVertNorm(int itmp, vector3f norm1,int ptxWidth,int ptxHeight,
				float *ptxX, float *ptxY,float *ptxZ, vector3f avgXYDir, int round)
{
	int i, itmp2, itmp3;
	vector3f v1, v2;
	int p1_i[2], p2_i[2];
	vector3f norms[8];
	int normCount=0;
	int x, y;

    static int counter = 0;
	if(IsNullPoint(ptxX[itmp],ptxY[itmp],ptxZ[itmp])) {
		norm1[0]=norm1[1]=norm1[2]=0;
        printf("return with nothing %d\n", counter++);
		return 0;
	}

	x=itmp/ptxHeight;
	y=itmp%ptxHeight;

	normCount=0;
	for(i=0;i<8;i++) {

		switch(i) {
		case 0:
			p1_i[0]=x;
			p1_i[1]=y+1;
			p2_i[0]=x-1;
			p2_i[1]=y+1;
			break;
		case 1:
			p1_i[0]=x-1;
			p1_i[1]=y+1;
			p2_i[0]=x-1;
			p2_i[1]=y;
			break;
		case 2:
			p1_i[0]=x-1;
			p1_i[1]=y;
			p2_i[0]=x-1;
			p2_i[1]=y-1;
			break;
		case 3:
			p1_i[0]=x-1;
			p1_i[1]=y-1;
			p2_i[0]=x;
			p2_i[1]=y-1;
			break;
		case 4:
			p1_i[0]=x;
			p1_i[1]=y-1;
			p2_i[0]=x+1;
			p2_i[1]=y-1;
			break;
		case 5:
			p1_i[0]=x+1;
			p1_i[1]=y-1;
			p2_i[0]=x+1;
			p2_i[1]=y;
			break;
		case 6:
			p1_i[0]=x+1;
			p1_i[1]=y;
			p2_i[0]=x+1;
			p2_i[1]=y+1;
			break;
		case 7:
			p1_i[0]=x+1;
			p1_i[1]=y+1;
			p2_i[0]=x;
			p2_i[1]=y+1;
			break;
		}

		if(p1_i[0]<0 || p1_i[1]<0 || p2_i[0]<0 || p2_i[1]<0 ||
			p1_i[0]>=ptxWidth || p1_i[1]>=ptxHeight || p2_i[0]>=ptxWidth || p2_i[1]>=ptxHeight){
			continue;
		}
		
		itmp2=p1_i[0]*ptxHeight+p1_i[1];
		itmp3=p2_i[0]*ptxHeight+p2_i[1];
		if(IsNullPoint(ptxX[itmp2],ptxY[itmp2],ptxZ[itmp2]) ||
			IsNullPoint(ptxX[itmp3],ptxY[itmp3],ptxZ[itmp3]) ) {
			continue;
		}

		if(round <= 0) {
			v1[0]=ptxX[itmp2]-ptxX[itmp];
			v1[1]=ptxY[itmp2]-ptxY[itmp];
			v1[2]=ptxZ[itmp2]-ptxZ[itmp];

			v2[0]=ptxX[itmp3]-ptxX[itmp];
			v2[1]=ptxY[itmp3]-ptxY[itmp];
			v2[2]=ptxZ[itmp3]-ptxZ[itmp];

			cross(v1,v2,norms[normCount]);
			normCount++;
		}else {
			getVertNorm(itmp2,norms[normCount],ptxWidth, ptxHeight, ptxX,ptxY, ptxZ, avgXYDir,round-1);
			normCount++;
		}
	}

	/* compute average	*/
	if(!bFVM) {
		norm1[0]=norm1[1]=norm1[2]=0;
		for(i=0;i<normCount;i++) {
			norm1[0]=(norm1[0]*i+norms[i][0])/(i+1);
			norm1[1]=(norm1[1]*i+norms[i][1])/(i+1);
			norm1[2]=(norm1[2]*i+norms[i][2])/(i+1);
		}
	}else {
		getFVM(norms,normCount,norm1);
	}

	return 1;
}

#if 1

/*	Finds the normal of the point located at itmp	*/
/*      new method */
int MygetVertNorm(int cell_num, vector<int> *vec_cell, int itmp, vector3f norm1,int ptxWidth,int ptxHeight,
                  float *ptxX, float *ptxY,float *ptxZ, vector3f avgXYDir, int round)
{
   int i, j, k, itmp2, itmp3;
   vector3f v1, v2, tmp_v;
   int p1_i[2], p2_i[2];
   int x, y;

   static int counter = 0;
   if(IsNullPoint(ptxX[itmp],ptxY[itmp],ptxZ[itmp])) {
      norm1[0]=norm1[1]=norm1[2]=0;
      printf("return with nothing\n ");
      assert(0);
      return 0;
   }

   vector<float *> vec_norms;
   
#if 0

   // this doesn't work for some reason.
   // maybe due to max_try --- there are too many points colinear.
   int data_len = vec_cell->size();
   int max_count = 2 * data_len;
   int max_try = max ( 8 * data_len, 10000);
   int cur_count = 0, cur_try = 0;

   while ( cur_count < max_count && cur_try < max_try ) {

      int i_rand = rand() % data_len;
      itmp  = vec_cell->at(i_rand) / 3; // it was "i" when push_back into vec_cell!

      while ( i_rand == itmp ) {
         i_rand = rand() % data_len;
      }
      itmp2 = vec_cell->at(i_rand) / 3;

      while ( i_rand == itmp || i_rand == itmp2) {
         i_rand = rand() % data_len;
      }
      itmp3 = vec_cell->at(i_rand) / 3;
            
      {
         v1[0]=ptxX[itmp2]-ptxX[itmp];
         v1[1]=ptxY[itmp2]-ptxY[itmp];
         v1[2]=ptxZ[itmp2]-ptxZ[itmp];
         
         v2[0]=ptxX[itmp3]-ptxX[itmp];
         v2[1]=ptxY[itmp3]-ptxY[itmp];
         v2[2]=ptxZ[itmp3]-ptxZ[itmp];

         vector3f tmp_vv;
         //cross(v1,v2, tmp_v);
         cross(v1,v2, tmp_vv);
         normalize(tmp_vv, tmp_v);

         cur_try ++;
         if ( tmp_vv[0] != 0 || tmp_vv[1] != 0 || tmp_vv[2] != 0 ) {
            cur_try = 0;
            cur_count ++;
            
            float *vv = (float *) malloc (sizeof(float) * 3);
            vv[0] = tmp_v[0];
            vv[1] = tmp_v[1];
            vv[2] = tmp_v[2];
            vec_norms.push_back(vv);
         }
      }
   }
   
#else
   
   for (i = 0; i < vec_cell->size(); i ++ ) {
      itmp  = vec_cell->at(i) / 3; // it was "i" when push_back into vec_cell!

      bool found = false;
      for ( j = i+1; j < vec_cell->size(); j ++ ) {
         itmp2 = vec_cell->at(j) / 3;

         for ( k = j+1; k < vec_cell->size(); k ++ ) {
            itmp3 = vec_cell->at(k) / 3;
            
            {
               v1[0]=ptxX[itmp2]-ptxX[itmp];
               v1[1]=ptxY[itmp2]-ptxY[itmp];
               v1[2]=ptxZ[itmp2]-ptxZ[itmp];

               v2[0]=ptxX[itmp3]-ptxX[itmp];
               v2[1]=ptxY[itmp3]-ptxY[itmp];
               v2[2]=ptxZ[itmp3]-ptxZ[itmp];

               vector3f tmp_vv;
               //cross(v1,v2, tmp_v);
               cross(v1,v2, tmp_vv);
               normalize(tmp_vv, tmp_v);

               if ( tmp_vv[0] != 0 || tmp_vv[1] != 0 || tmp_vv[2] != 0 ) {
                  //if ( 1 ) {
                  found = true;
                  float *vv = (float *) malloc (sizeof(float) * 3);
                  vv[0] = tmp_v[0];
                  vv[1] = tmp_v[1];
                  vv[2] = tmp_v[2];
                  vec_norms.push_back(vv);
                  break;
               }
            }
         }

         if ( found )
            break; //linear;
      }
   }
   
#endif   
   
   /* compute average	*/
   {
      int num = vec_norms.size();
      if ( num == 0 ) {
         // printf("BAD - No good normal found for this cell!\n");
         norm1[0]=norm1[1]=norm1[2]=0;
         return 1;
      }
      float norm_x = 0, norm_y = 0, norm_z = 0;
      for(i=0;i<num;i++) {
         norm_x += vec_norms[i][0];
         norm_y += vec_norms[i][1];
         norm_z += vec_norms[i][2];
         free(vec_norms[i]);
      }
      
      norm1[0] = norm_x / num;
      norm1[1] = norm_y / num;
      norm1[2] = norm_z / num;

      //printf("%f %f %f\n", norm1[0], norm1[1], norm1[2]);
   }

   return 1;
}

#else

/*	Finds the normal of the point located at itmp	*/
/*      new method */
int MygetVertNorm(int cell_num, vector<int> *vec_cell, int itmp, vector3f norm1,int ptxWidth,int ptxHeight,
                  float *ptxX, float *ptxY,float *ptxZ, vector3f avgXYDir, int round)
{
   int i, j, k, itmp2, itmp3;
   vector3f v1, v2, tmp_v;
   int p1_i[2], p2_i[2];
   int x, y;

   static int counter = 0;
   if(IsNullPoint(ptxX[itmp],ptxY[itmp],ptxZ[itmp])) {
      norm1[0]=norm1[1]=norm1[2]=0;
      printf("return with nothing\n ");
      return 0;
   }

   vector<float *> vec_norms;
   for (i = 0; i < vec_cell->size(); i ++ ) {
      itmp  = vec_cell->at(i) / 3; // it was "i" when push_back into vec_cell!

      for ( j = i+1; j < vec_cell->size(); j ++ ) {
         for ( k = j+1; k < vec_cell->size(); k ++ ) {
            itmp2 = vec_cell->at(j) / 3;
            itmp3 = vec_cell->at(k) / 3;
            
            {
               v1[0]=ptxX[itmp2]-ptxX[itmp];
               v1[1]=ptxY[itmp2]-ptxY[itmp];
               v1[2]=ptxZ[itmp2]-ptxZ[itmp];

               v2[0]=ptxX[itmp3]-ptxX[itmp];
               v2[1]=ptxY[itmp3]-ptxY[itmp];
               v2[2]=ptxZ[itmp3]-ptxZ[itmp];

               vector3f tmp_vv;
               cross(v1,v2, tmp_vv);
               normalize(tmp_vv, tmp_v);
               float *vv = (float *) malloc (sizeof(float) * 3);
               vv[0] = tmp_v[0];
               vv[1] = tmp_v[1];
               vv[2] = tmp_v[2];
               vec_norms.push_back(vv);
            }
         }
      }
   }

   /* compute average	*/
   {
      int num = vec_norms.size();
      float norm_x = 0, norm_y = 0, norm_z = 0;
      for(i=0;i<num;i++) {
         norm_x += vec_norms[i][0];
         norm_y += vec_norms[i][1];
         norm_z += vec_norms[i][2];
      }
      norm1[0] = norm_x / num;
      norm1[1] = norm_y / num;
      norm1[2] = norm_z / num;
   }

   return 1;
}

#endif

/* calculates the Fuzzy Vector Median	*/
void getFVM(vector3f *norms, int normCount, vector3f outNorm)
{
	int i, j;
	int *dists, min, median;
	float Ri, RiSum;
	vector3f vsum;

	dists = (int*)malloc(sizeof(int)*normCount);
	memset (dists, 0, sizeof(int)*normCount);

	/* calculate the vector median */
	for(i=0;i<normCount;i++) {
		
		for( j=0;j<normCount;j++)
			if(i != j)
				dists[i] += Angle(norms[i], norms[j]);
		if( i==0 ) {
			min = dists[i];
			median = 0;
		}else if(dists[i]<min) {
			min = dists[i];
			median = i;
		}
	}

	/* get the denominator in the FVM formula */
	RiSum = 0;
	for(i=0;i<normCount;i++)
		RiSum += Relation(norms[i], norms[median]);

	/* nominator	*/
	vsum[0] = vsum[1] = vsum[2] = 0;
	for(i=0;i<normCount;i++) {
		Ri = Relation(norms[i],norms[median]);
		vsum[0] += norms[i][0]*Ri;
		vsum[1] += norms[i][1]*Ri;
		vsum[2] += norms[i][2]*Ri;
	}

	vsum[0] = vsum[0]/RiSum;
	vsum[1] = vsum[1]/RiSum;
	vsum[2] = vsum[2]/RiSum;
	normalize(vsum,outNorm);

	free(dists);
}

/* relation between two vectors (based on the angle between them) */
float Relation(vector3f u, vector3f v)
{
	//float sigma = .4;
	float power;
	int angle, i;
	static BOOL bFirst= TRUE;
	static float powers[360], curSigma=0;

	if(bFirst || curSigma != sigma) {
		curSigma = sigma;
		bFirst = FALSE;
		for(i=0;i<360;i++)
			powers[i] = exp(-(i*i)/(2*sigma*sigma));
	}

	angle = Angle(u,v);
	//power = -(angle*angle)/(2*sigma*sigma);
	//return exp(power);

	return (powers[abs(angle)]);
}

float getDistFromPlane(myPlaneS plane, float x, float y, float z)
{
	float a,b,c;
	float x1,y1,z1;
	float dist=1;

	a=plane.normal[0];
	b=plane.normal[1];
	c=plane.normal[2];
	x1=plane.point[0];
	y1=plane.point[1];
	z1=plane.point[2];

	if(a!=0 || b!=0 || c!=0)
		dist=(a*(x-x1)+b*(y-y1)+c*(z-z1))/sqrt(a*a+b*b+c*c);
	else
		dist=0;

	return dist;
}

void assignIdsToPoints(pointCloudP ptc, int topNum,vector3f *topNorms,int *planeIds)
{
#define		AngleThres		10//10		//shepherd: ~20   /* * changed for Sheperd * */
	int i, j, k;
	int itmp1;
	vector3f norm1, norm2;
	vector3f avgXYDir;

	memset(planeIds,-1, sizeof(int)*ptc->ptCount);

	findAvgXYDir(ptc->width, ptc->height,ptc->ptCount,ptc->ptxX,
		ptc->ptxY,ptc->ptxZ,ptc->ptxR,avgXYDir);

	for(i=0;i<ptc->width;i++) {
		for(j=0;j<ptc->height;j++) {
			itmp1=i*ptc->height+j;
			getVertNorm(itmp1,norm1,ptc->width,ptc->height,ptc->ptxX,ptc->ptxY,ptc->ptxZ,avgXYDir,1);
			
			for(k=0;k<topNum;k++) {
				if(Angle(topNorms[k],norm1)<=AngleThres) {
					if(planeIds[itmp1]==-1) planeIds[itmp1]=k;
					break;
				}
			}
		}
	}

#undef		AngleThres
}

#ifdef		DONOTRUN

/*
	This function takes the already classified points according to their normals
	and reclassifies them according to the planes they belong to (their distance 
	from the origin (0,0,0) ).
*/
void assignIdsToPointsByDist(pointCloudP ptc,int topCount,vector3f *topNorms, int *planeIds,
							 int *topCount2, myPlaneP *mPlanes, int *planeIds2, int topPerNorm)
{
	int scale = 50;					///<----------------precision....
	int i, j, k;
	float ftmp1, ftmp2;
	int maxPtDist=0, iDist;
	point3f pMax, pMin;
//	vector3f avgXYDir;
	int **acc;
	int *ptxDists;
	myPlaneS mPlane;
	vector3f normVec;
	int itmp;
	imageP I, I1, I2;
	uchar	*pixel;
	int tmp_max, *levelcount, **levels_min, **levels_max, regCount, *levelAcc;
	int thr = 3;							/* * changed for Sheperd * */
	boundaryP bn;

	pMin[0]=pMax[0]=ptc->ptxX[0];
	pMin[1]=pMax[1]=ptc->ptxY[0];
	pMin[2]=pMax[2]=ptc->ptxZ[0];
	for(i=1;i<ptc->ptCount;i++) {
		if(pMin[0] > ptc->ptxX[i]) pMin[0] = ptc->ptxX[i];
		if(pMin[1] > ptc->ptxY[i]) pMin[1] = ptc->ptxY[i];
		if(pMin[2] > ptc->ptxZ[i]) pMin[2] = ptc->ptxZ[i];
		if(pMax[0] < ptc->ptxX[i]) pMax[0] = ptc->ptxX[i];
		if(pMax[1] < ptc->ptxY[i]) pMax[1] = ptc->ptxY[i];
		if(pMax[2] < ptc->ptxZ[i] && ptc->ptxZ[i] < 0.0) pMax[2] = ptc->ptxZ[i];

	}

//	findAvgXYDir(ptxWidth,ptxHeight,ptxPtCount,ptxX,ptxY,ptxZ,ptxR,avgXYDir);

	ftmp1=sqrt(pMax[0]*pMax[0]+pMax[1]*pMax[1]+pMax[2]*pMax[2]);
	ftmp2=sqrt(pMin[0]*pMin[0]+pMin[1]*pMin[1]+pMin[2]*pMin[2]);
	maxPtDist=MAX(abs(ftmp1+1),abs(ftmp2+1))+2;

	acc=(int**)malloc(sizeof(int*)*topCount);
	for(i=0;i<topCount;i++) {
		acc[i]=(int*)malloc(sizeof(int)*maxPtDist*2 * scale);		/////
		memset(acc[i],0,sizeof(int)*maxPtDist*2 *scale);			/////
	}

	ptxDists=(int*)malloc(sizeof(int)*ptc->ptCount);
	memset(ptxDists,-1,sizeof(int)*ptc->ptCount);

	memset(planeIds2,-1,sizeof(int)*ptc->ptCount);

	/* calculate the distance of each point from the camera origin in the 
		direction of their normals	*/
	tmp_max=0;
	for(i=0;i<ptc->ptCount;i++) {
		if(planeIds[i]!=-1) {
			mPlane.normal[0]=topNorms[planeIds[i]][0];
			mPlane.normal[1]=topNorms[planeIds[i]][1];
			mPlane.normal[2]=topNorms[planeIds[i]][2];
			mPlane.point[0]=0;
			mPlane.point[1]=0;
			mPlane.point[2]=0;
			iDist=getDistFromPlane(mPlane,ptc->ptxX[i],ptc->ptxY[i],ptc->ptxZ[i])*scale;	//+.5	/////
			if(abs(iDist)<maxPtDist*scale) {								//////
				for(j=-1*scale;j<=1*scale;j++) {
					if(abs(j)<.1*scale)
						acc[planeIds[i]][iDist+j+ maxPtDist*scale ]+=(.2*scale-.2*abs(j)+1);
				}
			
			//	acc[planeIds[i]][iDist+maxPtDist*scale ]++;
				ptxDists[i]=iDist;
				if(acc[planeIds[i]][iDist+ maxPtDist*scale ]>tmp_max)
					tmp_max=acc[planeIds[i]][iDist+ maxPtDist*scale ];
			}
		}
	}


	/* display the resulting spectrum	*/
	printf("tmp_max: %d\n",tmp_max);
	ftmp1=255.0/(float)tmp_max;
	I=IP_allocImage(maxPtDist*scale*2,topCount*scale,UCHARCH_TYPE);
	pixel=I->buf[0];
	memset(pixel,0,sizeof(uchar)*I->width*I->height);
	for(i=0;i<topCount;i++) {
		for(j=0;j<I->width;j++) {
			pixel[(scale*i+1)*I->width+j]= acc[i][j]*ftmp1;
			pixel[(scale*i+2)*I->width+j]= acc[i][j]*ftmp1;
			pixel[(scale*i+3)*I->width+j]= acc[i][j]*ftmp1;
			pixel[(scale*i+4)*I->width+j]= acc[i][j]*ftmp1;
			pixel[(scale*i+5)*I->width+j]= acc[i][j]*ftmp1;
			pixel[(scale*i+6)*I->width+j]= acc[i][j]*ftmp1;
		}
	}


/*	NextImageP=ImagePtr[NextImage];
	IP_copyImage(I,NextImageP);
	IP_displayImage();
*/
	IP_threshold(I, thr, thr, 0.0, thr, 255.0, I);
	//IP_threshold(I, tmp_max/20, tmp_max/20, 0.0, 255, 255, I);
	NextImageP=ImagePtr[NextImage];
	IP_copyImage(I,NextImageP);
	IP_displayImage();

	/* new - find top planes	*/
	levelcount=(int*)malloc(sizeof(int)*topCount);
	levels_min=(int**)malloc(sizeof(int*)*topCount);
	levels_max=(int**)malloc(sizeof(int*)*topCount);
	/* use connected components alg. and find local maxima at each component */
	for(i=0;i<topCount;i++) {
		I1=IP_allocImage(maxPtDist*scale*2,3,UCHARCH_TYPE);
		pixel=I1->buf[0];
		memset(pixel,0,sizeof(uchar)*I1->width*I1->height);
		for(j=0;j<I1->width;j++) {
			pixel[I1->width+j] = acc[i][j]*ftmp1;
		}
		IP_threshold(I1, thr, thr, 0.0, thr, 255.0, I1);
		pixel=I1->buf[0];
		tmp_max=0;
		for(j=0;j<I1->width;j++) {
			tmp_max += pixel[I1->width+j];
		}
		if(tmp_max == 0) {
			levelcount[i]=0;
			IP_freeImage(I1);
			continue;
		}

		I2 = IP_allocImage(I1->width, I1->height, UCHARCH_TYPE);
		regCount=topPerNorm;
		label(I1, I2, 255,&regCount, &bn,1,1);
		IP_freeImage(I1);
		IP_freeImage(I2);
		levelcount[i]=regCount;
		levels_min[i]=(int*)malloc(sizeof(int)*regCount);
		levels_max[i]=(int*)malloc(sizeof(int)*regCount);

		for(j=0;j<regCount;j++) {
			levels_min[i][j]=bn[j].xmin;
			levels_max[i][j]=bn[j].xmax;
		}
		free(bn);
	}

	levelAcc = (int*)malloc(sizeof(int)*topCount);
	levelAcc[0]=0;
	for(i=1;i<topCount;i++)
		levelAcc[i] = levelAcc[i-1] + levelcount[i-1];

	/*	reclassify points based on their distance from the origin	*/
	for(i=0;i<ptc->ptCount;i++) {
		if(planeIds[i]!=-1 && ptxDists[i]!=-1) {
			for(j=0;j<levelcount[planeIds[i]];j++) {
				if( ptxDists[i]+maxPtDist*scale>=levels_min[planeIds[i]][j] &&
					ptxDists[i]+maxPtDist*scale <= levels_max[planeIds[i]][j]){
					planeIds2[i]=levelAcc[planeIds[i]]+j;
					break;
				}
			}
		}
	}

	*topCount2 = 0;
	for(i=0;i<topCount;i++)
		*topCount2 += levelcount[i];
	*mPlanes=(myPlaneP)malloc(sizeof(myPlaneS)*(*topCount2));
	itmp = 0;
	for(i=0;i<topCount;i++) {
		normalize(topNorms[i],normVec);
		for(j=0;j<levelcount[i];j++) {
			k=itmp+j;
			(*mPlanes)[k].normal[0]=normVec[0];
			(*mPlanes)[k].normal[1]=normVec[1];
			(*mPlanes)[k].normal[2]=normVec[2];
			ftmp1 = (levels_max[i][j] + levels_min[i][j]) /2.0;
			(*mPlanes)[k].point[0]=normVec[0]*((ftmp1 - maxPtDist*scale)/(scale*1.0));	/////
			(*mPlanes)[k].point[1]=normVec[1]*((ftmp1 - maxPtDist*scale)/(scale*1.0));	/////
			(*mPlanes)[k].point[2]=normVec[2]*((ftmp1 - maxPtDist*scale)/(scale*1.0));	/////
		}
		itmp += levelcount[i];
	}

	/*	free tmp buffers	*/
	for(i=0;i<topCount;i++) {
		free(levels_max[i]);
		free(levels_min[i]);
		free(acc[i]);
	}
	free(levelcount);
	free(levelAcc);
	free(acc);
	free(ptxDists);
}


#endif

/*	--NEW--	*/

void classify_dists(int **acc, int numNorms, int depth, int thr, int dist_thr, int **acc_index)
{
	int i, j;

	for(i=0;i<numNorms;i++) {
		acc_index[i][0] = 0;
		for(j=1;j<depth;j++) {
			if(acc[i][j]>thr) {
				acc_index[i][j]=acc_index[i][j-1];
				acc[i][acc_index[i][j]] +=acc[i][j];
				acc[i][j] = 0;
			}else
				acc_index[i][j]=j;
		}
	}

}


/*
	This function takes the already classified points according to their normals
	and reclassifies them according to the planes they belong to (their distance 
	from the origin (0,0,0) ).
*/
void assignIdsToPointsByDist(pointCloudP ptc,int topCount,vector3f *topNorms, int *planeIds,
							 int *topCount2, myPlaneP *mPlanes, int *planeIds2, int topPerNorm)
{
	int scale = 100;					///<----------------precision....
	int i, j, k;
	float ftmp1, ftmp2;
	int maxPtDist=0, iDist;
	point3f pMax, pMin;
	uchar **acc_thr;
	int **acc;
	int *ptxDists;
	myPlaneS mPlane;
	vector3f normVec;
	int itmp;
	imageP I, I1, I2;
	uchar	*pixel;
	
	int **acc_index;
	int *level_count, **levels, *level_vals, depth;

	int tmp_max, *levelAcc;//, *levelcount, **levels_min, **levels_max, regCount ;
	int thr = 100;							/* * changed for Sheperd * */
	int dist_thr = scale*2;
	//boundaryP bn;

	pMin[0]=pMax[0]=ptc->ptxX[0];
	pMin[1]=pMax[1]=ptc->ptxY[0];
	pMin[2]=pMax[2]=ptc->ptxZ[0];
	for(i=1;i<ptc->ptCount;i++) {
		if(pMin[0] > ptc->ptxX[i]) pMin[0] = ptc->ptxX[i];
		if(pMin[1] > ptc->ptxY[i]) pMin[1] = ptc->ptxY[i];
		if(pMin[2] > ptc->ptxZ[i]) pMin[2] = ptc->ptxZ[i];
		if(pMax[0] < ptc->ptxX[i]) pMax[0] = ptc->ptxX[i];
		if(pMax[1] < ptc->ptxY[i]) pMax[1] = ptc->ptxY[i];
		if(pMax[2] < ptc->ptxZ[i] && ptc->ptxZ[i] < 0.0) pMax[2] = ptc->ptxZ[i];

	}

	ftmp1=sqrt(pMax[0]*pMax[0]+pMax[1]*pMax[1]+pMax[2]*pMax[2]);
	ftmp2=sqrt(pMin[0]*pMin[0]+pMin[1]*pMin[1]+pMin[2]*pMin[2]);
	maxPtDist=MAX(abs(ftmp1+1),abs(ftmp2+1))+2;

	acc =(int**)malloc(sizeof(int*)*topCount);
	acc_thr = (uchar**)malloc(sizeof(uchar*)*topCount);
	for(i=0;i<topCount;i++) {
		acc[i]=(int*)malloc(sizeof(int)*maxPtDist*2 * scale);		/////
		memset(acc[i],0,sizeof(int)*maxPtDist*2 *scale);			/////
		acc_thr[i]=(uchar*)malloc(sizeof(uchar)*maxPtDist*2 * scale);		/////
	}

	ptxDists=(int*)malloc(sizeof(int)*ptc->ptCount);
	memset(ptxDists,-1,sizeof(int)*ptc->ptCount);

	memset(planeIds2,-1,sizeof(int)*ptc->ptCount);

	/* calculate the distance of each point from the camera origin in the 
		direction of their normals	*/
	tmp_max=0;
	for(i=0;i<ptc->ptCount;i++) {
		if(planeIds[i]!=-1) {
			mPlane.normal[0]=topNorms[planeIds[i]][0];
			mPlane.normal[1]=topNorms[planeIds[i]][1];
			mPlane.normal[2]=topNorms[planeIds[i]][2];
			mPlane.point[0]=0;
			mPlane.point[1]=0;
			mPlane.point[2]=0;
			iDist=getDistFromPlane(mPlane,ptc->ptxX[i],ptc->ptxY[i],ptc->ptxZ[i])*scale;	//+.5	/////
			if(abs(iDist)<maxPtDist*scale) {								//////
				acc[planeIds[i]][iDist+maxPtDist*scale ]++;
				ptxDists[i]=iDist;
				if(acc[planeIds[i]][iDist+ maxPtDist*scale ]>tmp_max)
					tmp_max=acc[planeIds[i]][iDist+ maxPtDist*scale ];
			}
		}
	}


	depth = maxPtDist*2*scale;
	acc_index = (int**)malloc(sizeof(int*)*topCount);
	for(i=0; i<topCount; i++) {
		acc_index[i] = (int*)malloc(sizeof(int)*depth);
	}
	classify_dists(acc,topCount,maxPtDist*2*scale, thr, dist_thr,acc_index);

	/* sort the planes in n^2 */
	level_count = (int*)malloc(sizeof(int)*topCount);
	memset(level_count,0,sizeof(int)*topCount);
	level_vals = (int*)malloc(sizeof(int)*topPerNorm);
	levels = (int**)malloc(sizeof(int*)*topCount);
	for(i=0;i<topCount;i++) {
		levels[i]=(int*)malloc(sizeof(int) * topPerNorm);
		memset(levels[i],-1,sizeof(int)*topPerNorm);
	}
	/* sort the planes in n^2 */
	for(i=0;i<topCount;i++) {
		memset(level_vals, 0, sizeof(int)*topPerNorm);
		for(j=0;j<depth;j++) {
			for(k=0;k<topPerNorm;k++) {
				if(acc[i][j]>level_vals[k]) {
					if(k+1<topPerNorm) {
						memmove(&level_vals[k+1],&level_vals[k],sizeof(int)*(topPerNorm-k));
						memmove(&levels[i][k+1],&levels[i][k],sizeof(int)*(topPerNorm-k));
					}
					level_vals[k]=acc[i][j];
					levels[i][k]=j;
					level_count[i] = MIN(level_count[i]+1,topPerNorm);
					break;
				}
			}
		}
	}

	levelAcc = (int*)malloc(sizeof(int)*topCount);
	levelAcc[0]=0;
	for(i=1;i<topCount;i++)
		levelAcc[i] = levelAcc[i-1] + level_count[i-1];

	/*	reclassify points based on their distance from the origin	*/
	for(i=0;i<ptc->ptCount;i++) {
		if(planeIds[i]!=-1 && ptxDists[i]!=-1) {
			for(j=0;j<level_count[planeIds[i]];j++) {
				if( acc_index[planeIds[i]][ptxDists[i]+maxPtDist*scale] == levels[planeIds[i]][j]){
					planeIds2[i]=levelAcc[planeIds[i]]+j;
					break;
				}
			}
		}
	}

	*topCount2 = 0;
	for(i=0;i<topCount;i++)
		*topCount2 += level_count[i];
	*mPlanes=(myPlaneP)malloc(sizeof(myPlaneS)*(*topCount2));
	itmp = 0;
	for(i=0;i<topCount;i++) {
		normalize(topNorms[i],normVec);
		for(j=0;j<level_count[i];j++) {
			k=itmp+j;
			(*mPlanes)[k].normal[0]=normVec[0];
			(*mPlanes)[k].normal[1]=normVec[1];
			(*mPlanes)[k].normal[2]=normVec[2];
			ftmp1 = levels[i][j];
			(*mPlanes)[k].point[0]=normVec[0]*((ftmp1 - maxPtDist*scale)/(scale*1.0));	/////
			(*mPlanes)[k].point[1]=normVec[1]*((ftmp1 - maxPtDist*scale)/(scale*1.0));	/////
			(*mPlanes)[k].point[2]=normVec[2]*((ftmp1 - maxPtDist*scale)/(scale*1.0));	/////
		}
		itmp += level_count[i];
	}


	/*	free tmp buffers	*/
	for(i=0;i<topCount;i++) {
//		free(levels[i]);
		free(acc[i]);
		free(acc_thr[i]);
	}
	free(level_count);
	free(levelAcc);
	free(acc);
	free(acc_thr);
	free(ptxDists);
}


////////////////////////////

int loadPtxFile(char *filename, pointCloudP ptCloud)
{
	FILE *fp;
	int n, i, width, height, count;

	fp=fopen(filename, "r");
	if(!fp) {
		printf("Error: couldn't open %s.\n",filename);
		return 0;
	}

    float x, y, z;
    char line[1000];

    int counter = -1;
	while( fgets(line, 1000, fp) ) {
       if (fscanf(fp,"%f %f %f", &x, &y, &z) != 3 )
       {
          printf("READING ERROR at line %d\n", vec_data.size() / 3);
          continue;
       }

       // down-sampling
       if ( (counter ++) % gbl_opts->img_h != 0 )
          continue;
          
       vec_data.push_back(x);
       vec_data.push_back(y);
       vec_data.push_back(z);

       if ( x > max_x ) max_x = x;
       if ( x < min_x ) min_x = x;
       if ( y > max_y ) max_y = y;
       if ( y < min_y ) min_y = y;
       if ( z > max_z ) max_z = z;
       if ( z < min_z ) min_z = z;
	}

    if ( vec_data.size() % 3 != 0) {
       printf("EEEEEEEE\n");
       exit(0);
    }
    count = vec_data.size() / 3;
    
	ptCloud->ptCount = count;
    n = count;

	//ptCloud->height = count;
	//ptCloud->width = 1;
    int tmp_n = 100;
    while ( n / tmp_n > 10 * tmp_n )
       tmp_n *= 10;
    
	ptCloud->height = tmp_n;
	ptCloud->width = n/ptCloud->height;
	ptCloud->ptxX = (float*)malloc(sizeof(float)*n);
	ptCloud->ptxY = (float*)malloc(sizeof(float)*n);
	ptCloud->ptxZ = (float*)malloc(sizeof(float)*n);
	ptCloud->ptxR = (float*)malloc(sizeof(float)*n);

    for ( int i = 0; i < count; i ++ ) {
       ptCloud->ptxX[i] = vec_data[3*i];
       ptCloud->ptxY[i] = vec_data[3*i+1];
       ptCloud->ptxZ[i] = vec_data[3*i+2];
       ptCloud->ptxR[i] = vec_data[3*i+2];
    }

	fclose(fp);
	return 1;

}

BOOL IsNullPoint(float x, float y, float z)
{
	if(x==0 && y==0 && z==0)
		return TRUE;
	return FALSE;
}

int convToSphere(float *vec, int *theta, int *phi)
{
	float r=0;
	float p, t;

#ifdef LINUX
        if ( isnan(vec[0]) || isnan(vec[1]) || isnan(vec[2]))
#else
        if ( _isnan(vec[0]) || _isnan(vec[1]) || _isnan(vec[2]))
#endif           
           return -1;
        
	r=sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);

	if(r==0) {
		return -1;//r=.000001;
        }

	if(vec[0]!=0)
		t=atan(vec[1]/vec[0]);
	else
		t=PII/2;
	if(vec[0]<0)
		t=t+PII;
	if(t<0)
		t=2*PII+atan(vec[1]/vec[0]);

	p=acos(vec[2]/r);

        if ( p > 8 || p < -8 || t > 8 || t < -8 )
           return -1;

	*theta=(t*180)/PII;
	*phi=(p*180)/PII;

    if ( *phi >= 180 )
       *phi = 179;
    if ( *theta >= 360 )
       *theta = 359;

	return 1;
}

int convToCartesian(int theta, int phi, float *x, float *y, float *z)
{
	float c2=PII/180;

	*z=cos(phi*c2);
	*y=sin(phi*c2)*sin(theta*c2);
	*x=sin(phi*c2)*cos(theta*c2);

	return 1;
}

int Angle(vector3f v1, vector3f v2)
{
	float s1, s2, mul;
	float x1, y1, z1, x2, y2, z2;
	float cs=0;
	int deg=0;

	x1=v1[0]; y1=v1[1]; z1=v1[2];
	x2=v2[0]; y2=v2[1]; z2=v2[2];

	s1=sqrt(x1*x1+y1*y1+z1*z1);
	s2=sqrt(x2*x2+y2*y2+z2*z2);
	mul=x1*x2+y1*y2+z1*z2;

	if(s1==0 || s2==0) {
	//	UI_printf("betAng: Error: s1 || s2 == 0.");
		s1=s2=1;
	}
	cs=mul/(s1*s2);

    if ( cs >= 1.0 || cs <= -1.0 )
       deg = 0;
    else
       deg=acos(cs)*180/PII;

    if ( deg > 360 || deg < -360 ) {
       printf("ERROR: deg is %d, cs is %f\n", deg, cs);
       assert(0);
    }

	return deg;
}

/*	v1 x v2	*/
void cross(float *v1, float *v2, float *norm)
{
	norm[0]=v1[1]*v2[2] - v1[2]*v2[1];
	norm[1]=-(v1[0]*v2[2] - v1[2]*v2[0]);
	norm[2]=v1[0]*v2[1] - v1[1]*v2[0];
}

/*	v1 . v2	*/
float dot(vector3f v1, vector3f v2)
{
	return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);
}



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * IP_allocImage:
 *
 * Allocate space for an image of width w and height h.
 * Each pixel in the image has size s (in bytes).
 * Return image structure pointer. 
 */
imageP IP_allocImage(int w, int h, int s)
{
	uchar	*p;
	imageP	 I;

    
	/* allocate memory */
	I = (imageP ) malloc(sizeof(imageS));
	p = (uchar *) malloc(w * h * s);
	if(p == NULL) {
		fprintf(stderr, "IP_allocImage: Insufficient memory\n");
		return ((imageP) NULL);
	}

	/* init structure */
	I->width  = w;
	I->height = h;
	// I->image  = p;
	I->buf[0]  = p;

	return(I);
}

void IP_copyImage(imageP src, imageP dst)
{
   memcpy(dst->buf[0], src->buf[0], src->width * src->height);
}

void IP_freeImage(imageP I )
{
	free((char *) I->buf[0]);
	free((char *) I);
}

#include "dl.h"

int MAXLABEL;

void	pixel_label(int, int, int *);

/* if regNum == 0, label will not return any of the found regions.
 * if regNum == -1, label will return all of the found regions.
 * else, label will return regNum number of regions.
 * if bSort is true, label will sort the regions in descending order.
 */
void label(imageP I, imageP Iout, int regColor, int *regNum, boundaryP *bn, BOOL bSort,
		   int scaleOutImage)
{
	uchar	*p, *out;
	int		i, k, x, y, *l, *in;
	int		w, h;
	imageP	Itmp;

	int *maxX, *maxY, *minX, *minY, *count, *countmax, *tmpM;
	int totRegCount;
	int tmpcount=0;
	int j=0,tmp1;

	p = (uchar *)I->buf[0];
	w = I->width;
	h = I->height;

	MAXLABEL = w * h + 1;


    // FIX here soon.
	Itmp = IP_allocImage(w, h, INTCH_TYPE);
	in = (int *)Itmp->buf[0];


	for (i=0; i<w*h; i++) {
		if (p[i] == regColor) in[i] = i+1;
		else			 in[i] = MAXLABEL;
	}

	pixel_label(w, h, in);	/* label connected component */

	for(i=0,y=0; y<h; y++)
		for(x=0; x<w; x++, i++) {
			if (in[i] == MAXLABEL) in[i] = 0;
			/* border does not have label */
			if (x == 0 || x == w-1 || y == 0 || y == h-1) in[i] = 0;
	}

	l = (int *) malloc(w * h * sizeof(int));
	for(i=0; i<w*h; i++) {l[i] = 0;}	

	in = (int *)Itmp->buf[0];
	for(i=0; i<w*h; i++) {
		if (in[i] != 0) l[in[i]] = 1;
	}
	/* color each label (segment) with different gray level */

	tmpcount=0;
	for(i=0, k = 2; i < w*h; i++) 
		if (l[i] == 1) {l[i] = k; k = k + 1; tmpcount++;}

	printf("region count: %d\n",tmpcount);

	out = (uchar *)Iout->buf[0];

	if(scaleOutImage) {
		for(i=0; i<w*h; i++) {
			out[i] = (l[in[i]]==0)?0:l[in[i]]*5+50;
		}
	}else {
		for(i=0; i<w*h; i++) {
			out[i] = (l[in[i]]==0)?0:l[in[i]]-1;				//////////////////-1
		}
	}

	if(*regNum == 0) {
		IP_freeImage(Itmp);
		free(l);
		return;
	}
	if(*regNum == -1)
		totRegCount=tmpcount;//+2;
	else
		totRegCount=MIN(tmpcount,*regNum);
	*regNum=totRegCount;

	maxX=(int*)malloc(sizeof(int)*(tmpcount+2));
	maxY=(int*)malloc(sizeof(int)*(tmpcount+2));
	minX=(int*)malloc(sizeof(int)*(tmpcount+2));
	minY=(int*)malloc(sizeof(int)*(tmpcount+2));

	count=(int*)malloc(sizeof(int)*(tmpcount+2));
	if(bSort) {
		countmax=(int*)malloc(sizeof(int)*totRegCount);
		tmpM=(int*)malloc(sizeof(int)*totRegCount);
	}

	if( bSort )
		for(i=0;i<totRegCount;i++)
			countmax[i]=0;

	for(i=0;i<tmpcount+2;i++) {
		maxX[i]=0;
		maxY[i]=0;
		minX[i]=w;
		minY[i]=h;
		count[i]=0;
	}

	/*	count the number of points in each region.	*/
	for(j=0;j<h;j++)
		for(i=0;i<w;i++) {
			tmp1=i+j*w;
			if(l[in[tmp1]]>1) {
				if(i>maxX[l[in[tmp1]]]) maxX[l[in[tmp1]]]=i;
				if(j>maxY[l[in[tmp1]]]) maxY[l[in[tmp1]]]=j;
				if(i<minX[l[in[tmp1]]]) minX[l[in[tmp1]]]=i;
				if(j<minY[l[in[tmp1]]]) minY[l[in[tmp1]]]=j;
				count[l[in[tmp1]]]++;
			}
		}

	if ( bSort ) {
		for(i=2;i<tmpcount+2;i++) {
			for(k=0;k<totRegCount;k++) {
				if(count[i]>count[countmax[k]]) {
					if(k<totRegCount-1) {
						memcpy(tmpM, &countmax[k],sizeof(int)*(totRegCount-k-1));
						memcpy(&countmax[k+1], tmpM,sizeof(int)*(totRegCount-k-1));
					}
					countmax[k]=i;
					break;
				}
			}
		}
	}

	// show the center of each region
	/*for(i=2;i<tmpcount+2;i++) {
		tmp1=avX[i]+avY[i]*w;
		out[tmp1]=255;
	}*/

	/*	pick the top largest planes	*/
	*bn=(boundaryP)malloc(sizeof(boundaryS)*totRegCount);
	if ( bSort ) {
		for(i=0;i<totRegCount;i++) {
			(*bn)[i].xmin=minX[countmax[i]];
			(*bn)[i].xmax=maxX[countmax[i]];
			(*bn)[i].ymin=minY[countmax[i]];
			(*bn)[i].ymax=maxY[countmax[i]];
		}
	} else {
		for(i=0;i<totRegCount;i++) {
			(*bn)[i].xmin=minX[i+2];		///
			(*bn)[i].xmax=maxX[i+2];		//
			(*bn)[i].ymin=minY[i+2];
			(*bn)[i].ymax=maxY[i+2];		//
		}
	}

	IP_freeImage(Itmp);
	free(l);

	if(bSort) {
		free(countmax);
		free(tmpM);
	}
	free(maxX);
	free(maxY);
	free(minX);
	free(minY);

}


/***********************************************************************/
/* Labels all connected pixels in the image I                          */
/***********************************************************************/

void pixel_label(int w, int h, int *in)
{


	int	 x, y, eot=0, flag, min, idx;
	int	 N, S, E, W, NE, NW, SE, SW;
	int	*pixel;

	pixel = in;

    int count = 0;
    //	while (!eot) {
	{
		flag = 1;
		for(y=1; y<h-1; y++) {
			for(x=1; x<w-1; x++) {
				idx = y*w+x;
				if (pixel[idx] != MAXLABEL) {
					N  = pixel[idx - w];
					S  = pixel[idx + w];
					E  = pixel[idx + 1];
					W  = pixel[idx - 1];
					NE = pixel[idx - w + 1];
					NW = pixel[idx - w - 1];
					SE = pixel[idx + w + 1];
					SW = pixel[idx + w - 1];
					min = MIN(N, MIN(S, MIN(E, MIN(W, MIN(NE, MIN(NW, MIN(SE, MIN(SW, pixel[idx]))))))));
					if ((pixel[idx] ==  min) && (flag == 1))  
						eot |= 1; 
					else {
						eot = 0; 
						flag = 0;
					}
					pixel[idx] = min;
				}
		   }
		}
        
	}

	
}



