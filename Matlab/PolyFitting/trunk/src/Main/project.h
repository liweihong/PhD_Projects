
#ifndef PHD_PROJECT_H_
#define PHD_PROJECT_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>
#include <deque>
#include <map>
#include <functional>
#include "array.h"
#include "st.h"
#include "avl.h"

#ifndef LINUX
#include "graph.h"
//#include "point.h"

class Point1
{
public:
	Point1(int u, int v)
	{
		this->x = u;
		this->y = v;
	}

	Point1() {};

	int x;
	int y;
};
#endif

#ifdef MSVC_60
#define for if(false) ; else for
extern int min(int a, int b);
extern int max(int a, int b);
#define ULONG_PTR ULONG 
#pragma comment(lib, "gdiplus.lib")
#endif


#ifndef LINUX
#include <windows.h>
#include <gdiplus.h>
#ifndef MSVC_60
#include <atltime.h>
#endif
#endif


#include "CImg.h"
using namespace cimg_library;

using namespace std;

#ifndef LINUX
using namespace Gdiplus;
#endif

#define BLACK 1
#define WHITE 0
#define MAXIMUM 1000
#define PI 3.1415926
#define FLOAT_SIGMA 0.01

/*  THRESHOLD value */

/*  Hough Transform Parameters */
// the hough transform distance and angle grid size
#define HT_PSTEP            1
#define HT_TETASTEP         1

/* This is used to Visual Studio 2005 ASSERTION debug */
#if 0
#define assert Assert
void Assert(bool b)
{
   if (!b)
   {
      printf("ASSERTION ERROR!\n");
      exit(0);
   }
}
#endif

// Distance within which the points to the HT line to be removed.
// This value should be related to p step value
#define HT_LAMBDATHRESH     HT_PSTEP + 2

// the parameters for red line segment end point determination
#define HT_NUMTHRESH        3  // number of points considered as a line segments
#define HT_LENTHRESH        8  // test line length.

// the MASK to remove data around a line point
#define HT_MASK             2  // 2x2 mask

/* Google SketchUp Parameters */
// search mask
#define GS_LINE_LEN_SQUARE  2500  // at least 50 pixels?
#define GS_MERGE_DIS_SQUARE 16   // at most 4 pixels for merge
#define GS_SEARCH_MASK      4    // search mask 8x8 mask
#define GS_SEARCH_ANGLE     100  // search max angle.

/* BPA boundary computation parematers */
#define BPA_ROUGH_BOUNDARY_RADIUS  128     // threshold for turning back
#define BPA_AFTER_ROUGH_RADIUS     128      // threshold for refinement radius
#define BPA_AFTER_ROUGH_REFINE_NUM 200     // threshold for refine number of after rough radius
#define BPA_REGULAR_REFINE_NUM     40      // threshold for regular refine number
#define THRES_HT_BPA_COMBINE_LEN   80      // mininum length of HT line for combination with BPA

typedef unsigned char BYTE;
typedef int DIST;  // In case we want to change the data type
                   // for symmetry distance, such as from int to float;

typedef enum _verbose_level {
   KEY_INFO,
   MID_INFO,
   LOW_INFO,
   ALL_INFO
} VERBOSE_LEVEL;

/* TYPE: GLOBAL_OPTIONS
   Global options passed as global variable
*/
typedef struct _global_options {

   // control try sth
   int try_sth;

   /* general parameters */
   char *input_prefix;
   char *output_prefix;
   int   total_slices;

   /* this section is for BPA */
   // radius of the ball
   int radius;
   int radius_min;

   // refinement, ignore multiple boundaries
   int refine_ignore_mutli_boundaries;

   // clear the data in the starting circle for BPA
   int BPA_clear_first_circle_data;
   int BPA_starting_point_sweeping_cut_off;  // cut off poistion to stopping sweeping. In other words, the sweeping will not exceed this point.
   int BPA_starting_point_sweeping_dir;  // 0 - bottom, 1 -left, 2 - top, 3 - right
   int BPA_refine_speed_up;  // default is 1;
   int BPA_debug_color_boundary;
   int BPA_starting_point_boundary_diff_threshold;
   int BPA_starting_point_landing_len;
   int BPA_simple_starting_point;
   int BPA_check_turning_point_for_first_iteration;

   // set the gap line dynamically
   int BPA_gap_line_matters;  // if this flag is set, we must pay attention to the gap
                              // lines no matter how long the radius is.
   int BPA_gap_line_length;   // how long is the minimum gap.
   int BPA_reshape_slope_line;// whether to reshape the slope line, like / to _|.
   int BPA_boundary_break_len;// the length threshold to break a sub unit (circle);
   int BPA_refine_number;     // maxinum number of refinement.
   int BPA_debug_mode;
   int BPA_debug_image_interval;
   int BPA_need_do_HT_BPA_combination;
   int BPA_need_do_refinement;
   int BPA_merge_boundary_points;
   int BPA_clear_data_for_new_boundary;
   int BPA_clear_data_mask;
   int BPA_do_not_check_turning_around;
   int BPA_turning_around_angle;
   int BPA_2nd_round_HT_length;
   int BPA_2nd_round_HT_dilation_mask;
   int BPA_keep_remaining_boundary_point_after_split;
   int BPA_ignore_gap_line;
   int BPA_no_circle_when_replace_with_HT;
   int BPA_dump_the_vectors;
   int BPA_turning_check_pool_size;
   int BPA_2nd_round_need_no_HT_process;
   int BPA_do_not_remove_extra_turning_around_pts;

   // whether to do second round BPA
   char *BPA_parameter_file;
   char *BPA_2nd_round_parameter_file;
   
   // interval for pivoting the ball
   // default value 1, but big radius may be less than 1.
   double pace_degree;
   
   // MIN and MAX debug number
   int min_debug;
   int max_debug;

   // debug is set to 1 after this refinement iteration
   int refine_counter;

   // max length for ball pivot to avoid turning around.
   int half_length;

   // image width and height
   int img_h;
   int img_w;

   /* this section is for key slice detection preprocess */
   double KSD_slope;
   double KSD_ratio;           // the ratio that the curvature is treated as key slices
   int KSD_offset;
   int KSD_slope_num;
   int KSD_dist;
   int KSD_ref_side; // 0: use bottom as reference side; 1: use left as reference side
   int KSD_single_key_slice;
   int KSD_valid_region_min_x; // the valid region used to determine the curvature;
   int KSD_valid_region_max_x; // the valid region used to determine the curvature;
   int KSD_valid_region_min_y; // the valid region used to determine the curvature;
   int KSD_valid_region_max_y; // the valid region used to determine the curvature;

   int KSD_same;
   int KSD_size;
   int KSD_new_slice;
   int KSD_least_key_slice_interval;
   int KSD_contains_tapered_structure;
   double KSD_HD_diff_threshold;
   double KSD_HD_data_percentage;
   
   /* this section is for tapered structure generation */
   int TSG_num;
   
   /* this section is for IR Generation */
   int IR_do_not_extend_the_last_key_slice_to_one;
   int IR_do_not_extend_the_first_key_slice_to_zero;

   /* this section is for bounding box mask */
   int BBM_using_bounding_box_mask;
   char *BBM_mask_folder;

   /* this section is for xxx */
} GLOBAL_OPTIONS;

/* TYPE: POINTS_AUX
   Used by BPA refinement
 */
typedef struct _points_aux {
   /* record the end point of the ball, namely P1 */
   deque<int>  end_points;

   /* record the particular radius */
   deque<int>  radius;

   /* record the color of the lines */
   deque<int>  colors;
   
} POINTS_AUX;

/* global vars */
// Field: output_fn
// global output filename
extern char *output_fn;

// Field: global_opts
// all global options are saved in this var
extern char *global_opts;
extern GLOBAL_OPTIONS *gbl_opts;

extern int DEBUG_BPA;

// Field: image_height
// global image height
extern int image_height;

// Field: image_width
// global image width
extern int image_width;

// function declaration:
void load_lines_from_ras2vec(vector<int> &r2v_line_set, char *fn, int h, int w);
void load_options(char *string, int& argc, char **argv);

// Helper functions for vector processing
typedef struct _vector
{
   size_t size;
   size_t capacity;
   int *data;
} Vector;

typedef struct _cc_array
{
   size_t size;
   size_t capacity;
   Vector *data;
} CC_Head;

typedef struct _point_ {
   float x;
   float y;
   float z;
} POINT_3D;

typedef struct _line_ {
   POINT_3D P0;
   POINT_3D P1;
} LINE_3D;

typedef struct _plane_ {
   float A;
   float B;
   float C;
   float D;     // Ax + By + Cz + D = 0
   POINT_3D N;  // normal 
   POINT_3D V;  // a point on plane
} PLANE_3D;

/*******************   project.c  ***********************/
#ifdef LINUX
void Save2File(char* fname, BYTE **image0, CImg<unsigned char> *bitmap, int image_W, int image_H, int color, BYTE ***image_color, int save_format = 0);
CImg<unsigned char> * load_image(char *file_name, int &h, int &w);
void set_image(BYTE **&image, CImg<BYTE> *bitmap);
#else
void Save2File(char* fname, BYTE **image, Bitmap *bitmap, int image_W, int image_H, int color, BYTE ***image_color, int save_format = 0);
Bitmap * load_image(char *file_name, int &h, int &w);
void set_image(BYTE **&image, Bitmap *bitmap);
#endif

BYTE** new_image(int h, int w);
BYTE*** new_3D_image(int h, int w);
void free_image(BYTE **src);
void free_image_3D(BYTE ***src);
char *digit_string(int num, int counter);
bool inside_3D_container_for_dir_0(double x, double y, double z);
DIST ** set_distance_table(BYTE**image, int img_h, int img_w);
inline int index(int x, int y);
inline int y_axis(int index);
inline int x_axis(int index);
void set_x_min(double x);
void set_x_max(double x);
void set_y_min(double x);
void set_y_max(double x);
void set_z_min(double x);
void set_z_max(double x);
bool file_exist(char *fn);
inline bool valid_pixel(int x, int y);
void image_dilation(BYTE ** &im);
void thinningImage(BYTE **image , int h, int w);
BYTE** copy_image(BYTE **src, int h, int w);
vector<int> find(BYTE *im, int len, int value);
vector<int>* find(BYTE *im, vector<int> *index, int value);
void my_HT(BYTE **im_2D, int h, int w, int &pdetect, int &tetadetect);
void set_3D_image(BYTE **Imbinary, BYTE *** &image, int h,int w);
void draw_line_on_2D_image(BYTE **im, int h, int w, int x1, int y1, int x2, int y2);
void write_lines_to_image(char *fn, vector<int> &line_set, int h, int w, vector<int> *colors_ptr = NULL);
void draw_line_on_data(int pd, int td, BYTE **im, int h, int w, BYTE ***im3D, BYTE **orig_im, vector<int> &line_set);
int compute_intersection_point(int pd0, int td0, int pd1, int td1, int &int_x, int &int_y);
void dump_boundary_points_to_IR();
void generate_dxf_from_IR(char *fn);
bool has_significant_change(BYTE **ref_im, BYTE **cur_im, int h, int w, int matching_approach);
void BPA_boundary(char *fn);
array_t * load_end_points_from_IR(const char *fn, array_t *z_arr);
array_t * load_end_points_from_IR_with_dirs(const char *fn, array_t *z_arr);
array_t * load_end_points_from_IR_with_follow_me(const char *fn, array_t *z_arr, array_t *fm_arr);
char *str_replace ( const char *s , const char *pattern , const char *replacement , int startPosition , int occurences );
void  draw_line(BYTE ***im, int h, int w, int x1, int y1, int x2, int y2, int r, int g, int b);
vector<int> *pixels_between_2_points_in_order(int h, int w, int x1, int y1, int x2, int y2);
void draw_boundary_image(array_t *boundary_array, array_t *boundary_array_aux, int rad, char *fn = NULL);
void BPA_boundary_image(BYTE **im, array_t *&boundary_array);
BYTE ** draw_boundaries_on_image(array_t *arr_all_boundaires, int rad = -2);
void dump_boundaries_to_file(array_t *arr_all_boundaries, int rad, FILE *fd);
void ball_pivoting(BYTE **cur_im, deque<int>& points, int rad, int p1, BYTE **&debug_im, int sep_num, POINTS_AUX *points_aux);
bool is_good_matched(BYTE **im, int h, int w, int x1, int y1, int x2, int y2, float ratio = 0.5);
inline void cross_product(POINT_3D &v1, POINT_3D &v2, POINT_3D &result);
inline float dot_product(POINT_3D &v1, POINT_3D &v2);
float compute_dist_point_to_line( POINT_3D &P, POINT_3D &L_P0, POINT_3D &L_P1);
PLANE_3D* compute_plane_from_3D_pts(POINT_3D &p1, POINT_3D &p2, POINT_3D &p3);
POINT_3D* compute_intersection_line_from_2_planes(PLANE_3D &Pn1, PLANE_3D &Pn2, POINT_3D &p0, POINT_3D &p1);
void my_printf(const char *fmt, ...);

/********************  main_taper.c *********************/
extern void load_point_cloud_left_right();
extern void infer_taper_use_other_dirs();

extern void assign_3D_box_Y_dir(double &BOX_MIN_X,
                                double &BOX_MAX_X,
                                double &BOX_MIN_Y,
                                double &BOX_MAX_Y,
                                double &BOX_MIN_Z,
                                double &BOX_MAX_Z,
                                int type = 0);
extern double get_3D_coordinate_from_2D_pixel_value(int dir, int coord, int coord_value);

/********************  seg_merge.c *********************/
extern void ksd_with_segmentation(void **);
extern void ksd_with_segmentation_and_tapers(void **params);
extern void ksd_follow_me_detect(void **params);
extern void ksd_all_info_fusion(void **params);
extern void bpa_update_contours(void **params);
extern void ksd_zipper_boundary(void **params);
extern void ksd_extrusion_intersection(void **params);
extern void comp_segments_image(BYTE **im, st_table *&st_all_regions, st_table *&st_all_data, int thres_mask, int approach = 0);
extern void fill_polygon(vector<int> &vec_poly, vector<int> &vec_fill);
extern BYTE **load_image_to_array(char *fn);
extern int gc_main();

#endif

/*
Curvature computation and figure generation:

Refer to http://www.mathkb.com/Uwe/Forum.aspx/matlab/56371/Computation-of-curvature-of-2-D-curve
for curvature computation using Matlab

The code for curvature computation on fig8:
xe = [x(3);x;x(end-2)]; ye = [y(3);y;y(end-2)];
dx = diff(xe); dy = diff(ye);
dxb = dx(1:end-1); dyb = dy(1:end-1);
dxf = dx(2:end); dyf = dy(2:end);
d2x = xe(3:end)-xe(1:end-2); d2y = ye(3:end)-ye(1:end-2);
curv = 2*(dxb.*dyf-dyb.*dxf)./ ...
      sqrt((dxb.^2+dyb.^2).*(dxf.^2+dyf.^2).*(d2x.^2+d2y.^2));

Matlab code:
x = B(:,1);
y = B(:,2);
curv2=curv;
sc_curv2=curv2*15;
int_curv2 = int8(sc_curv2);
shift_int_curv2 = 49 - int_curv2

Binary code:
project -q -F matlab_curv2_slice_3.txt -f matlab_cur2_slice3.png -W 323 -w 548

comments:
*. set param.ini to generate vector txt file;
*. copy the [x,y] in vector into B;
*. copy columnly from shift_int_curv2 to vector txt file;
*. use the above command to load IR to imagel

 */
 
