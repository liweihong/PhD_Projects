/*
  Two tasks fulfilled here:
  * DXF model from left to right.
  * Tapered structure inference.

  * Steps for DXF generation:
  
  0: sliced images generated (1000) - _
  1: noise cleaned based on bounding box (1000) - _cleaned
  2: symmetry computation (1000) - _recoverd
  3: boundary computation for key slices detection (1000) - _boundary
     4.0: key slices generation (65/1000) - _result (aaa_xxx.png)
     4.1: boundary vectorization (65/1000) - _result (bbb_xxx.png)
     4.2: manually remove some noise based on result of 4.1, go back to 4.1 again
     4.3: dump the vector into ascii file for dxf generation.
  5: generate dxf file.

*/

#include "project.h"

// define local variables
static double INIT  = 10000.0;
static double x_min = INIT;
static double x_max = INIT;
static double y_min = INIT;
static double y_max = INIT;
static double z_min = INIT;
static double z_max = INIT;

// Function: project_point_cloud_left_right
// convert the txt file into image
//
// Command options - project.exe -l ..\data\Rotated.bpa -O "-s total_slices -x -X -y -Y -z -Z -d 0/1/2"
// project.exe -A -O "-s 1000 -d 0" // get 1k sliced image bottom up
// project.exe -A -O "-s 1000 -w 1000 -h 2048 -Z 45.0 -d 2" // get 1k sliced image left - right
// to mapping back from preprocessed key slice, using the following :
// z_i = IMAGE_HEIGHT - (int)((slice_data[i+1] - BOX_MIN_Y) * scale + .5) - 1 - 40; =>
// 1053 = 2048 - ((x - 0.0) * (1000/(45.0 - 20.0)) + .5) - 41
// slice # = x/(44.0 - 0.0) = .542...
// Symmetry line 507; // computed already.  (506 of 1024 - a few)
void project_point_cloud_left_right()
{
   char *fn = "../data/Rotated.bpa";
   FILE *fid = fopen(fn, "r");
   if (!fid)
   {
      printf("Could not open the file %s\n", fn);
      return;
   }

   // CHANGE: IMAGE_HEIGHT
   int IMAGE_WIDTH  = 600;   // 1000
   int IMAGE_HEIGHT = 1024;  // 2048
   int pre_slice_num = -1;

   // direction: 0 - Y axis (bottom up), 1 - Z axis (face to face), 2 - X axis (left to right)
   int direction = 2;
   int has_set_size = 0;

   // BOX : -36.2841 82.3821 -2.4001 80.6962 -15.4828 136.499
   // TEST: -x 20 -X 90 -z 20 -Z 60 -y -3.0 -Y 44.0
   double BOX_MIN_X = 20.0;
   double BOX_MAX_X = 90.0;
   double BOX_MIN_Y =  0.0;
   double BOX_MAX_Y = 44.0;
   double BOX_MIN_Z = 20.0;
   double BOX_MAX_Z = 50.0; // 60.0
   
   if (global_opts)
   {
      int  argc = 0;
      char *argv[30];
      load_options(global_opts, argc, argv);
      
      char c;
      char *cmdline;
      util_getopt_reset();
      while ((c = util_getopt(argc, argv, "d:h:w:x:X:y:Y:z:Z:s:")) != EOF)
      {
         switch (c) {
            case 'd':
               direction = atoi(util_optarg);
               assert(direction == 0 || direction == 1 || direction == 2);
               break;
            case 'h':
               IMAGE_HEIGHT = atoi(util_optarg);
               has_set_size = 1;
               break;
            case 'w':
               IMAGE_WIDTH  = atoi(util_optarg);
               has_set_size = 1;
               break;
            case 's':
               pre_slice_num = atoi(util_optarg);
               break;
            case 'x':
               BOX_MIN_X = atof(util_optarg);
               break;
            case 'X':
               BOX_MAX_X = atof(util_optarg);
               break;
            case 'y':
               BOX_MIN_Y = atof(util_optarg);
               break;
            case 'Y':
               BOX_MAX_Y = atof(util_optarg);
               break;
            case 'z':
               BOX_MIN_Z = atof(util_optarg);
               break;
            case 'Z':
               BOX_MAX_Z = atof(util_optarg);
               break;
            default:
               break;
         }
      }
   }

   // COMPUTING THE SYMMETRY COORDINATE (based on x = 507 symmetry line)
   int symmetry_x = 507;
   double diff_x = BOX_MAX_X - BOX_MIN_X;
   double sym_x = BOX_MIN_X + ((double)symmetry_x/(double)1024)*diff_x;

   // LOADING data...
   char str_line[1000];
   char x_s[30], y_s[30], z_s[30];
   double x, y, z;
   vector<double> data;
   printf("loading data from file...\n");
   while (fgets(str_line, 1000, fid))
   {
      if (sscanf(str_line, "%s %s %s", z_s, y_s, x_s) == 3)
      {
         x = atof(x_s);
         y = atof(y_s);
         z = atof(z_s);

         // rule out of some points
         if (x < BOX_MIN_X || x > BOX_MAX_X ||
             y < BOX_MIN_Y || y > BOX_MAX_Y ||
             z < BOX_MIN_Z || z > BOX_MAX_Z)
            continue;

         // rule out noise for Y axis (direction == 0)
         // if (direction == 0 && !inside_3D_container_for_dir_0(x, y, z))
         if (!inside_3D_container_for_dir_0(x, y, z))
            continue;

         data.push_back(y); // push y first, so that index don't need to be add by 1 in the following.
         data.push_back(x);
         data.push_back(z);

         // reflect this point based on the symmetry line
         data.push_back(y); // push y first, so that index don't need to be add by 1 in the following.
         data.push_back(x + 2*(sym_x - x));
         data.push_back(z);
      }
   }

   fclose(fid);

   // starting write data into image;
   assert(data.size() % 3 == 0);

   double diff_y = BOX_MAX_Y - BOX_MIN_Y;
   double diff_z = BOX_MAX_Z - BOX_MIN_Z;
   int slice_total = pre_slice_num == -1 ? 1000 : pre_slice_num;
   int slice_number =  0;


   // CHANGE: diff_y
   double slice_int =  diff_x / slice_total;
   vector<double> slice_data;

#if 0   

   for (int i = 0; i < slice_total; i ++)
   {
      printf("\nProcessing slice #%d, ", i);
      // find out the appropriate points
      // CHANGE: BOX_MIN_Y   
      double BOX_MIN = BOX_MIN_X;
      double slice_min = i*slice_int + BOX_MIN;
      double slice_max = slice_min + slice_int;
      // double diff_sym_x = 2*(sym_x - slice_min);
      // double slice_min_1 = slice_min  + (diff_sym_x > 0 ? diff_sym_x : - diff_sym_x); BUG: 
      // double slice_min_1 = slice_min  + diff_sym_x;
      // double slice_max_1 = slice_min_1 + slice_int;
      for (int index = 0; index < data.size(); index += 3)
      {
         // CHANGE: index
         int index_ = index+1;
         if ((data[index_] > slice_min && data[index_] < slice_max))
         {
            slice_data.push_back(data[index+2]);  // z axis
            slice_data.push_back(data[index]);    // y axis
            slice_data.push_back(data[index+1]);  // x axis
         }
      }

      /*
      if (slice_data.size() == 0)
         continue;
      */

      printf("found data/image, slice number: %d, slice_min: %f.", slice_number, slice_min);
      // z - height, x - width, y - 3D high
      BYTE **im = new_image(IMAGE_HEIGHT, IMAGE_WIDTH);
      // CHANGE: diff_x
      double dir_scale_diff = diff_z;
      double scale = (double)IMAGE_WIDTH / dir_scale_diff;
      for (int i = 0; i < slice_data.size(); i+=3)
      {
         // CHANGE: BOX_MIN_Z
         int z_i = IMAGE_HEIGHT - (int)((slice_data[i+1] - BOX_MIN_Y) * scale + .5) - 1 - 40; // y
         if (z_i < 0)
            continue;

         int x_i = (int)(((slice_data[i] - BOX_MIN_Z)/diff_z)*IMAGE_WIDTH  + .5); // z
         if (x_i == IMAGE_WIDTH)
            x_i -= 1;
         
         im[z_i][x_i] = BLACK;

      }

      char ofn[30];
      char *ds = digit_string(4, slice_number++);
      sprintf(ofn, "result/point_cloud_images/image_slice_%s.png", ds);
      free(ds);
      Save2File(ofn, im, NULL, IMAGE_WIDTH, IMAGE_HEIGHT, 0, NULL);
      free_image(im);
      slice_data.clear();
   }
   
#else

   st_table *images = st_init_table(st_ptrcmp, st_ptrhash);
   vector<double> *arr_slice;
   double BOX_MIN = BOX_MIN_X;
   for (int i = 0; i < slice_total; i ++)
   {
      arr_slice = new vector<double>();
      st_insert(images, (char *)i, (char *)arr_slice);
   }
   
   for (int index = 0; index < data.size(); index += 3)
   {
      // CHANGE: index
      int index_ = index+1;
      int arr_idx = (data[index_] - BOX_MIN )/slice_int;
      assert(arr_idx <= slice_total);
      if (arr_idx ==  slice_total)
         arr_idx --;

      assert(st_lookup(images, (char *)arr_idx, (char **)&arr_slice));
      {
         arr_slice->push_back(data[index+2]);  // z axis
         arr_slice->push_back(data[index]);    // y axis
         arr_slice->push_back(data[index+1]);  // x axis
      }
   }

   for (int i = 0; i < slice_total; i ++)
   {
      printf("\nProcessing slice #%d, ", i);
      /*
      if (slice_data.size() == 0)
         continue;
      */

      double slice_min = i*slice_int + BOX_MIN;
      double slice_max = slice_min + slice_int;
      printf("found data/image, slice number: %d, slice_min: %f.", slice_number, slice_min);
      // z - height, x - width, y - 3D high
      BYTE **im = new_image(IMAGE_HEIGHT, IMAGE_WIDTH);
      // CHANGE: diff_x
      double dir_scale_diff = diff_z;
      double scale = (double)IMAGE_WIDTH / dir_scale_diff;
      vector<double> *temp_slice;
      st_lookup(images, (char *)i, (char **)&temp_slice);
      slice_data = *temp_slice;
      for (int i = 0; i < slice_data.size(); i+=3)
      {
         // CHANGE: BOX_MIN_Z
         int z_i = IMAGE_HEIGHT - (int)((slice_data[i+1] - BOX_MIN_Y) * scale + .5) - 1 - 40; // y
         if (z_i < 0)
            continue;

         int x_i = (int)(((slice_data[i] - BOX_MIN_Z)/diff_z)*IMAGE_WIDTH  + .5); // z
         if (x_i == IMAGE_WIDTH)
            x_i -= 1;
         
         im[z_i][x_i] = BLACK;

      }

      char ofn[30];
      char *ds = digit_string(4, slice_number++);
      sprintf(ofn, "result/point_cloud_images/image_slice_%s.png", ds);
      free(ds);
      Save2File(ofn, im, NULL, IMAGE_WIDTH, IMAGE_HEIGHT, 0, NULL);
      free_image(im);
      slice_data.clear();
      delete temp_slice;
   }

   st_free_table(images);
   
#endif
   
}

// Function: project_point_cloud_face_inside
//
// to mapping back from preprocessed key slice, using the following :
// z_i = IMAGE_HEIGHT - (int)((slice_data[i+1] - BOX_MIN_Y) * scale + .5) - 1 - 40; =>
// 1053 = 2048 - ((x - 0.0) * (1000/(45.0 - 20.0)) + .5) - 41
// slice # = x/(44.0 - 0.0) = .542...
//
// Symmetry line 507; // computed already.  (506 of 1024 - a few)
void project_point_cloud_face_inside()
{
   char *fn = "../data/Rotated.bpa";
   FILE *fid = fopen(fn, "r");
   if (!fid)
   {
      printf("Could not open the file %s\n", fn);
      return;
   }

   // CHANGE: IMAGE_HEIGHT
   int IMAGE_WIDTH  = 1024;
   int IMAGE_HEIGHT = 744;
   int pre_slice_num = -1;

   // direction: 0 - Y axis (bottom up), 1 - Z axis (face to face), 2 - X axis (left to right)
   int direction = 1;
   int has_set_size = 0;

   // BOX : -36.2841 82.3821 -2.4001 80.6962 -15.4828 136.499
   // TEST: -x 20 -X 90 -z 20 -Z 60 -y -3.0 -Y 44.0
   double BOX_MIN_X = 20.0;
   double BOX_MAX_X = 90.0;
   double BOX_MIN_Y =  0.0;
   double BOX_MAX_Y = 44.0;
   double BOX_MIN_Z = 20.0;
   double BOX_MAX_Z = 50.0; // 60.0
   
   if (global_opts)
   {
      int  argc = 0;
      char *argv[30];
      load_options(global_opts, argc, argv);
      
      char c;
      char *cmdline;
      util_getopt_reset();
      while ((c = util_getopt(argc, argv, "d:h:w:x:X:y:Y:z:Z:s:")) != EOF)
      {
         switch (c) {
            case 'd':
               direction = atoi(util_optarg);
               assert(direction == 0 || direction == 1 || direction == 2);
               break;
            case 'h':
               IMAGE_HEIGHT = atoi(util_optarg);
               has_set_size = 1;
               break;
            case 'w':
               IMAGE_WIDTH  = atoi(util_optarg);
               has_set_size = 1;
               break;
            case 's':
               pre_slice_num = atoi(util_optarg);
               break;
            case 'x':
               BOX_MIN_X = atof(util_optarg);
               break;
            case 'X':
               BOX_MAX_X = atof(util_optarg);
               break;
            case 'y':
               BOX_MIN_Y = atof(util_optarg);
               break;
            case 'Y':
               BOX_MAX_Y = atof(util_optarg);
               break;
            case 'z':
               BOX_MIN_Z = atof(util_optarg);
               break;
            case 'Z':
               BOX_MAX_Z = atof(util_optarg);
               break;
            default:
               break;
         }
      }
   }

   // COMPUTING THE SYMMETRY COORDINATE (based on x = 507 symmetry line)
   int symmetry_x = 507;
   double diff_x = BOX_MAX_X - BOX_MIN_X;
   double sym_x = BOX_MIN_X + ((double)symmetry_x/(double)1024)*diff_x;

   // LOADING data...
   char str_line[1000];
   char x_s[30], y_s[30], z_s[30];
   double x, y, z;
   vector<double> data;
   printf("loading data from file...\n");
   while (fgets(str_line, 1000, fid))
   {
      if (sscanf(str_line, "%s %s %s", z_s, y_s, x_s) == 3)
      {
         x = atof(x_s);
         y = atof(y_s);
         z = atof(z_s);

         // rule out of some points
         if (x < BOX_MIN_X || x > BOX_MAX_X ||
             y < BOX_MIN_Y || y > BOX_MAX_Y ||
             z < BOX_MIN_Z || z > BOX_MAX_Z)
            continue;

         // rule out noise for Y axis (direction == 0)
         // if (direction == 0 && !inside_3D_container_for_dir_0(x, y, z))
         if (!inside_3D_container_for_dir_0(x, y, z))
            continue;

         data.push_back(y); // push y first, so that index don't need to be add by 1 in the following.
         data.push_back(x);
         data.push_back(z);

         // reflect this point based on the symmetry line
         data.push_back(y); // push y first, so that index don't need to be add by 1 in the following.
         data.push_back(x + 2*(sym_x - x));
         data.push_back(z);
      }
   }

   fclose(fid);

   // starting write data into image;
   assert(data.size() % 3 == 0);

   double diff_z = BOX_MAX_Z - BOX_MIN_Z;
   int slice_total = pre_slice_num == -1 ? 1000 : pre_slice_num;
   int slice_number =  0;

   // CHANGE: diff_y
   double slice_int =  diff_z / slice_total;
   vector<double> slice_data;

#if 0
   
   for (int i = 0; i < slice_total; i ++)
   {
      printf("\nProcessing slice #%d, ", i);
      // find out the appropriate points
      // CHANGE: BOX_MIN_Y   
      double BOX_MIN = BOX_MIN_Z;
      double slice_min = i*slice_int + BOX_MIN;
      double slice_max = slice_min + slice_int;
      for (int index = 0; index < data.size(); index += 3)
      {
         // CHANGE: index
         int index_ = index+2;
         if ((data[index_] > slice_min && data[index_] < slice_max))
         {
            slice_data.push_back(data[index+2]);  // z axis
            slice_data.push_back(data[index]);    // y axis
            slice_data.push_back(data[index+1]);  // x axis
         }
      }

      /*
      if (slice_data.size() == 0)
         continue;
      */

      printf("found data/image, slice number: %d, slice_min: %f.", slice_number, slice_min);
      // z - height, x - width, y - 3D high
      BYTE **im = new_image(IMAGE_HEIGHT, IMAGE_WIDTH);
      // CHANGE: diff_x
      double dir_scale_diff = diff_x;
      double scale = (double)IMAGE_WIDTH / dir_scale_diff;
      for (int i = 0; i < slice_data.size(); i+=3)
      {
         // CHANGE: BOX_MIN_Z
         int z_i = IMAGE_HEIGHT - (int)((slice_data[i+1] - BOX_MIN_Y) * scale + .5) - 1 - 40; // y
         if (z_i < 0)
            continue;

         int x_i = (int)(((slice_data[i+2] - BOX_MIN_X)/diff_x)*IMAGE_WIDTH  + .5); // x
         if (x_i == IMAGE_WIDTH)
            x_i -= 1;
         
         im[z_i][x_i] = BLACK;

      }

      char ofn[30];
      char *ds = digit_string(4, slice_number++);
      sprintf(ofn, "result/point_cloud_images/image_slice_%s.png", ds);
      free(ds);
      Save2File(ofn, im, NULL, IMAGE_WIDTH, IMAGE_HEIGHT, 0, NULL);
      free_image(im);
      slice_data.clear();
   }

#else

   st_table *images = st_init_table(st_ptrcmp, st_ptrhash);
   vector<double> *arr_slice;
   double BOX_MIN = BOX_MIN_Z;
   for (int i = 0; i < slice_total; i ++)
   {
      arr_slice = new vector<double>();
      st_insert(images, (char *)i, (char *)arr_slice);
   }
   
   for (int index = 0; index < data.size(); index += 3)
   {
      // CHANGE: index
      int index_ = index+2;
      int arr_idx = (data[index_] - BOX_MIN )/slice_int;
      assert(arr_idx <= slice_total);
      if (arr_idx ==  slice_total)
         arr_idx --;

      assert(st_lookup(images, (char *)arr_idx, (char **)&arr_slice));
      {
         arr_slice->push_back(data[index+2]);  // z axis
         arr_slice->push_back(data[index]);    // y axis
         arr_slice->push_back(data[index+1]);  // x axis
      }
   }

   for (int i = 0; i < slice_total; i ++)
   {
      printf("\nProcessing slice #%d, ", i);
      /*
      if (slice_data.size() == 0)
         continue;
      */

      double slice_min = i*slice_int + BOX_MIN;
      double slice_max = slice_min + slice_int;
      printf("found data/image, slice number: %d, slice_min: %f.", slice_number, slice_min);
      // z - height, x - width, y - 3D high
      BYTE **im = new_image(IMAGE_HEIGHT, IMAGE_WIDTH);
      // CHANGE: diff_x
      double dir_scale_diff = diff_x;
      double scale = (double)IMAGE_WIDTH / dir_scale_diff;
      vector<double> *temp_slice;
      st_lookup(images, (char *)i, (char **)&temp_slice);
      slice_data = *temp_slice;
      for (int i = 0; i < slice_data.size(); i+=3)
      {
         // CHANGE: BOX_MIN_Z
         int z_i = IMAGE_HEIGHT - (int)((slice_data[i+1] - BOX_MIN_Y) * scale + .5) - 1 - 40; // y
         if (z_i < 0)
            continue;

         int x_i = (int)(((slice_data[i+2] - BOX_MIN_X)/diff_x)*IMAGE_WIDTH  + .5); // z
         if (x_i == IMAGE_WIDTH)
            x_i -= 1;
         
         im[z_i][x_i] = BLACK;

      }

      char ofn[30];
      char *ds = digit_string(4, slice_number++);
      sprintf(ofn, "result/point_cloud_images/image_slice_%s.png", ds);
      free(ds);
      Save2File(ofn, im, NULL, IMAGE_WIDTH, IMAGE_HEIGHT, 0, NULL);
      free_image(im);
      slice_data.clear();
      delete temp_slice;
   }

   st_free_table(images);
   
#endif   
}


// Function: load_point_cloud_left_right
//
// Description: The outer layer of the DXF generation of the left-right frames
//
void load_point_cloud_left_right()
{
   // step 1. Project the point cloud into image slices
   // project_point_cloud_left_right();  // LEFT-RIGHT IMAGES
   project_point_cloud_face_inside();    // FACE-INSIDE IMAGES

   // step 2. boundary

   // step 3. Similarity measurements

   // step 4. boundary vectorization

   // step 5. DXF generation.
}

// Function: infer_taper_get_related_info
//
// Description: infer the taper structure by segmenting tapered structure recognized from other
//              directions. 
//
void infer_taper_get_related_info()
{
#ifndef LINUX
   Bitmap* orig_img = NULL;
#else
   CImg<BYTE> *orig_img = NULL;
#endif
   char *in_pre = "result/point_cloud_images/slices_left_right_600_1024_recovered/image_slice_";
   char *ou_acc = "result/point_cloud_images/acc_template.png";
   char *ou_acc1 = "result/point_cloud_images/acc_template_1.png";
   char file_name[1000];
   BYTE **cur_im = NULL, **acc_im = NULL;

   // generate the template image
   // get 90 - 140 as template, height 380
   int THRES_HE = 380;
   int h, w;

#if 0   

   printf("Processing the slice : \n");
   for (int i = 90; i < 140; i ++)
   {
      printf(" %d ", i);
      char *ds = digit_string(4, i);
      sprintf(file_name, "%s%s.png", in_pre, ds);
      free(ds);

      orig_img = load_image(file_name, h, w);
      set_image(cur_im, orig_img);

      if (acc_im == NULL)
         acc_im = new_image(h, w);

      for (int k = 0; k < THRES_HE * w; k ++)
         acc_im[0][k] |= cur_im[0][k];

      if (orig_img != NULL) delete orig_img;
      if (cur_im != NULL) free_image(cur_im);
   }
   Save2File(ou_acc, acc_im, NULL, w, h, 0, NULL);
   
#endif

   // load the template file
   orig_img = load_image(ou_acc, h, w);
   set_image(acc_im, orig_img);

   // preprocess the template for the distance computation
   if (1)
   {
      // do dilation first
      image_dilation(acc_im);
      for (int x = 0; x < w; x ++)
         for (int y = 0; y < h; y ++) {
            if (acc_im[y][x] == WHITE)
               continue;
            for (int k = y; k < h; k ++)
               acc_im[k][x] = BLACK;
         }
      Save2File(ou_acc1, acc_im, NULL, w, h, 0, NULL);
   }

   if (orig_img)
      delete orig_img;

   // compute the distance LUT
   DIST **lut = set_distance_table(acc_im, h, w);
   
   // probe the tapered regions
   int status = 0;
   printf("\nProcessing the slice : \n");
   for (int i = 140; i < 1000; i ++)
   {
      printf(" %d ", i);
      char *ds = digit_string(4, i);
      sprintf(file_name, "%s%s.png", in_pre, ds);
      free(ds);

      orig_img = load_image(file_name, h, w);
      set_image(cur_im, orig_img);

      int total_num = 0;
      for (int k = 0; k < THRES_HE * w; k ++)
      {
         if (cur_im[0][k] == BLACK)
            total_num += lut[y_axis(k)][w+x_axis(k)];
      }

      if (total_num > 1000 && status == 0)
      {
         status = 1;
         printf("\n============ Leaving template at %d with total_num %d ===========\n", i, total_num);
      } else if (total_num < 40 && status == 1)
      {
         status = 0;
         printf("\n============ Entering template at %d ===========\n", i);
      }

      if (orig_img != NULL) delete orig_img;
      if (cur_im != NULL) free_image(cur_im);
   }
}

void infer_taper_get_related_info_face()
{
#ifndef LINUX
   Bitmap* orig_img = NULL;
#else
   CImg<BYTE> *orig_img = NULL;
#endif
   char *in_pre = "result/point_cloud_images/slices_face_inside_1024_744_recovered/image_slice_";
   char *ou_acc = "result/point_cloud_images/acc_template_face.png";
   char *ou_acc1 = "result/point_cloud_images/acc_template_1_face.png";
   char file_name[1000];
   BYTE **cur_im = NULL, **acc_im = NULL;

   // generate the template image
   // get 770 - 790 as template, height 260
   int THRES_HE = 260;
   int h, w;

#if 0

   printf("Processing the slice : \n");
   for (int i = 770; i < 790; i ++)
   {
      printf(" %d ", i);
      char *ds = digit_string(4, i);
      sprintf(file_name, "%s%s.png", in_pre, ds);
      free(ds);

      assert(file_exist(file_name));
      orig_img = load_image(file_name, h, w);
      set_image(cur_im, orig_img);

      if (acc_im == NULL)
         acc_im = new_image(h, w);

      for (int k = 0; k < THRES_HE * w; k ++)
         acc_im[0][k] |= cur_im[0][k];

      if (orig_img != NULL) delete orig_img;
      if (cur_im != NULL) free_image(cur_im);
   }
   Save2File(ou_acc, acc_im, NULL, w, h, 0, NULL);
   exit(0);
#endif

   // load the template file
   orig_img = load_image(ou_acc, h, w);
   set_image(acc_im, orig_img);

   // preprocess the template for the distance computation
   if (1)
   {
      // do dilation first
      image_dilation(acc_im);
      for (int x = 0; x < w; x ++)
         for (int y = 0; y < h; y ++) {
            if (acc_im[y][x] == WHITE)
               continue;
            for (int k = y; k < h; k ++)
               acc_im[k][x] = BLACK;
         }
      Save2File(ou_acc1, acc_im, NULL, w, h, 0, NULL);
      exit(0);
   }

   if (orig_img)
      delete orig_img;

   // compute the distance LUT
   DIST **lut = set_distance_table(acc_im, h, w);
   
   // probe the tapered regions
   int status = 0;
   printf("\nProcessing the slice : \n");
   for (int i = 900; i > 0; i --)
   {
      printf(" %d ", i);
      char *ds = digit_string(4, i);
      sprintf(file_name, "%s%s.png", in_pre, ds);
      free(ds);

      orig_img = load_image(file_name, h, w);
      set_image(cur_im, orig_img);

      int total_num = 0;
      for (int k = 0; k < THRES_HE * w; k ++)
      {
         if (cur_im[0][k] == BLACK)
            total_num += lut[y_axis(k)][w+x_axis(k)];
      }

      if (total_num > 1000 && status == 0)
      {
         status = 1;
         printf("\n============ Leaving template at %d with total_num %d ===========\n", i, total_num);
      } else if (total_num < 40 && status == 1)
      {
         status = 0;
         printf("\n============ Entering template at %d ===========\n", i);
     } 

      if (orig_img != NULL) delete orig_img;
      if (cur_im != NULL) free_image(cur_im);
   }
}

void assign_3D_box_X_dir(double &BOX_MIN_X,
                         double &BOX_MAX_X,
                         double &BOX_MIN_Y,
                         double &BOX_MAX_Y,
                         double &BOX_MIN_Z,
                         double &BOX_MAX_Z)
{
   BOX_MIN_X = 20.0;
   BOX_MAX_X = 90.0;
   BOX_MIN_Y =  0.0;
   BOX_MAX_Y = 44.0;
   BOX_MIN_Z = 20.0;
   BOX_MAX_Z = 50.0;  // 60.0
}

void assign_3D_box_Y_dir(double &BOX_MIN_X,
                         double &BOX_MAX_X,
                         double &BOX_MIN_Y,
                         double &BOX_MAX_Y,
                         double &BOX_MIN_Z,
                         double &BOX_MAX_Z,
                         int type)
{
   switch ( type ) {
      case 0: 
         BOX_MIN_X = 20.0;   // regular Thomas hunter building
         BOX_MAX_X = 90.0;
         BOX_MIN_Y =  0.0;
         BOX_MAX_Y = 44.0;
         BOX_MIN_Z = 20.0;
         BOX_MAX_Z = 60.0;
         break;
      case 1:
         BOX_MIN_X = -9.17;  // Hunter theater dataset
         BOX_MAX_X = 13.06;
         BOX_MIN_Y = -8.05;
         BOX_MAX_Y = 15.40;
         BOX_MIN_Z = -1.04;
         BOX_MAX_Z = 10.50;
         break;
      case 2:         
         BOX_MIN_X = x_min;  // Other scans
         BOX_MAX_X = x_max;
         BOX_MIN_Y = y_min;
         BOX_MAX_Y = y_max;
         BOX_MIN_Z = z_min;
         BOX_MAX_Z = z_max;
         break;
      default:
         break;
   }

   assert(BOX_MIN_X != INIT && BOX_MAX_X != INIT &&
          BOX_MIN_Y != INIT && BOX_MAX_Y != INIT &&
          BOX_MIN_Z != INIT && BOX_MAX_Z != INIT);
         
}

void assign_3D_box_Z_dir(double &BOX_MIN_X,
                         double &BOX_MAX_X,
                         double &BOX_MIN_Y,
                         double &BOX_MAX_Y,
                         double &BOX_MIN_Z,
                         double &BOX_MAX_Z)
{
   BOX_MIN_X = 20.0;
   BOX_MAX_X = 90.0;
   BOX_MIN_Y =  0.0;
   BOX_MAX_Y = 44.0;
   BOX_MIN_Z = 20.0;
   BOX_MAX_Z = 50.0;
}

void set_x_min(double x)
{
   x_min = x;
}

void set_x_max(double x)
{
   x_max = x;
}

void set_y_min(double y)
{
   y_min = y;
}

void set_y_max(double y)
{
   y_max = y;
}

void set_z_min(double z)
{
   z_min = z;
}

void set_z_max(double z)
{
   z_max = z;
}

// Function: get_3D_coordinate_from_2D_pixel_value
//
// Description: obtain the 3D coordinates from 2D pixle values
//              We have to specify the mode,
// dir - the slice is perperticlar to the X, Y, Z
// coord - coord of 3D which is to be computed.
double get_3D_coordinate_from_2D_pixel_value(int dir, int coord, int coord_value)
{
   double BOX_MIN_X;
   double BOX_MAX_X;
   double BOX_MIN_Y;
   double BOX_MAX_Y;
   double BOX_MIN_Z;
   double BOX_MAX_Z;
   int IMAGE_WIDTH;
   int IMAGE_HEIGHT;
   double result = 0.0;
   double scale = 0.0;

   switch (dir) {
      case 0:
         assign_3D_box_X_dir(BOX_MIN_X, BOX_MAX_X, BOX_MIN_Y, BOX_MAX_Y, BOX_MIN_Z, BOX_MAX_Z);
         IMAGE_WIDTH  =  600;
         IMAGE_HEIGHT = 1024;;
         break;
      case 1:
         assign_3D_box_Y_dir(BOX_MIN_X, BOX_MAX_X, BOX_MIN_Y, BOX_MAX_Y, BOX_MIN_Z, BOX_MAX_Z);
         IMAGE_WIDTH  = 1024;
         IMAGE_HEIGHT =  392;
         break;
      case 2:
         assign_3D_box_Z_dir(BOX_MIN_X, BOX_MAX_X, BOX_MIN_Y, BOX_MAX_Y, BOX_MIN_Z, BOX_MAX_Z);
         IMAGE_WIDTH  = 1024;
         IMAGE_HEIGHT =  744;
         break;
      default:
         printf("ERROR: dir must be 0, 1, or 2!!!\n");
   }

   double diff_x = BOX_MAX_X - BOX_MIN_X;
   double diff_y = BOX_MAX_Y - BOX_MIN_Y;
   double diff_z = BOX_MAX_Z - BOX_MIN_Z;

   /*
   int symmetry_x = 507;
   double diff_x = BOX_MAX_X - BOX_MIN_X;
   double sym_x = BOX_MIN_X + ((double)symmetry_x/(double)1024)*diff_x;
   // 1053 = 2048 - ((x - 0.0) * (1000/(45.0 - 20.0)) + .5) - 41
   */

   switch (coord) {
      case 0:  // X 3D coordinate
         assert(dir != 0);
         result = BOX_MIN_X + ((double)coord_value/(double)IMAGE_WIDTH)*diff_x;
         break;
      case 1:  // Y 3D 
         // int z_i = IMAGE_HEIGHT - (int)((slice_data[i+1] - BOX_MIN_Y) * scale + .5) - 1 - 40; // y
         assert(dir != 1);
         if (dir == 0) {
            scale = (double)IMAGE_WIDTH / diff_z;
            result = BOX_MIN_Y + (double)(IMAGE_HEIGHT - coord_value - 1 - 40 - .5)/scale;
        } else {
            scale = (double)IMAGE_WIDTH / diff_x;
            result = BOX_MIN_Y + (double)(IMAGE_HEIGHT - coord_value - 1 - 40 - .5)/scale;
         }
         break;
      case 2:  // Z 3D
         assert(dir != 2);
         // If Z dir, reverse height of image
         // int x_i = (int)(((slice_data[i] - BOX_MIN_Z)/diff_z)*IMAGE_WIDTH  + .5); // z
         // If X dir, width of image
         if (dir == 1) {
            scale = (double)IMAGE_WIDTH / diff_x;
            result = BOX_MIN_Z + ((double)(IMAGE_HEIGHT - coord_value - 1 - .5))/scale;
         }
         else  // maybe wrong too!
            result = BOX_MIN_Z + ((double)coord_value/(double)IMAGE_WIDTH)*diff_z;
         break;
      default:
         printf("ERROR: wrong coord!!!!!!\n");
   }
   
   return result;
   
}

// Function: infer_taper_get_segments
//
// Description: infer the taper structure by segmenting tapered structure recognized from other
//              directions. 
//
void infer_taper_get_segments(vector<double> params, vector<int> &ret_params)
{
#ifndef LINUX
   Bitmap* orig_img = NULL;
#else
   CImg<BYTE> *orig_img = NULL;
#endif

   char ou_reg[1000];
   sprintf(ou_reg, "%s/../region_box.png", gbl_opts->input_prefix);;
   
   // bounding box - x1 = 301; x2 = 688, z1 = 583
   // let's show the bounding box
   // const int x1 = 324; //301;
   // const int x2 = 667; //688;
   // const int z1 = 583;
   const int x1 = 301;
   const int x2 = 688;
   const int z1 = 588;

   // mapping the image coordinates back to 3D
   double x_3D_1, x_3D_2, z_3D_1;
   int x_2D_1, x_2D_2, z_2D_1;
   {
      double BOX_MIN_X;
      double BOX_MAX_X;
      double BOX_MIN_Y;
      double BOX_MAX_Y;
      double BOX_MIN_Z;
      double BOX_MAX_Z; // 60.0
      assign_3D_box_X_dir(BOX_MIN_X, BOX_MAX_X, BOX_MIN_Y, BOX_MAX_Y, BOX_MIN_Z, BOX_MAX_Z);
         
      double diff_x = BOX_MAX_X - BOX_MIN_X;
      double diff_y = BOX_MAX_Y - BOX_MIN_Y;
      double diff_z = BOX_MAX_Z - BOX_MIN_Z;

      // using project_point_cloud_left_right() to map x?
      // double slice_min = i*slice_int + BOX_MIN;
      double slice_total = 1000.0;
      double slice_int =  diff_x / slice_total;
      x_3D_1 = x1 * slice_int + BOX_MIN_X;
      x_3D_2 = x2 * slice_int + BOX_MIN_X;
      
      // using project_point_cloud_face_inside() to map z?
      slice_int = diff_z / slice_total;
      z_3D_1 = z1 * slice_int + BOX_MIN_Z;
   }

   // draw box in Y direction
   // z - deep, x - width, y - 3D high
   {
      double BOX_MIN_X;
      double BOX_MAX_X;
      double BOX_MIN_Y;
      double BOX_MAX_Y;
      double BOX_MIN_Z;
      double BOX_MAX_Z;
      assign_3D_box_Y_dir(BOX_MIN_X, BOX_MAX_X, BOX_MIN_Y, BOX_MAX_Y, BOX_MIN_Z, BOX_MAX_Z);

      double diff_x = BOX_MAX_X - BOX_MIN_X;
      double diff_y = BOX_MAX_Y - BOX_MIN_Y;
      double diff_z = BOX_MAX_Z - BOX_MIN_Z;
      
      int IMAGE_WIDTH  = 1024;
      int IMAGE_HEIGHT =  392;
      BYTE **im = new_image(IMAGE_HEIGHT, IMAGE_WIDTH);
      x_2D_1 = (int)(((x_3D_1 - BOX_MIN_X)/diff_x)*IMAGE_WIDTH  + .5); // x
      x_2D_2 = (int)(((x_3D_2 - BOX_MIN_X)/diff_x)*IMAGE_WIDTH  + .5); // x
      draw_line_on_2D_image(im, IMAGE_HEIGHT, IMAGE_WIDTH, x_2D_1, 0, x_2D_1, IMAGE_HEIGHT - 1);
      draw_line_on_2D_image(im, IMAGE_HEIGHT, IMAGE_WIDTH, x_2D_2, 0, x_2D_2, IMAGE_HEIGHT - 1);

      double scale = (double)IMAGE_WIDTH / diff_x;
      z_2D_1 = IMAGE_HEIGHT - (int)((z_3D_1 - BOX_MIN_Z) * scale + .5) - 1; // z
      draw_line_on_2D_image(im, IMAGE_HEIGHT, IMAGE_WIDTH, 0, z_2D_1, IMAGE_WIDTH - 1, z_2D_1);
      Save2File(ou_reg, im, NULL, IMAGE_WIDTH, IMAGE_HEIGHT, 0, NULL);
   }
   

   // compute the image between two layers
   double x_min, y_min, z_min, x_max, y_max, z_max;
   assign_3D_box_Y_dir(x_min, x_max, y_min, y_max, z_min, z_max);
   int index_min = (int)((params[1] - y_min)/(y_max - y_min) * 1000 + .5);
   int index_max = (int)((params[0] - y_min)/(y_max - y_min) * 1000 + .5);
   printf("The index is between %d and %d\n", index_min, index_max);
   ret_params.push_back(index_min);
   ret_params.push_back(index_max);

   if (0)
   {
      int h = 392;
      int w = 1024;
      char *pre_fix = "result/point_cloud_images/slices_1000_0.0_44.0_recovered/image_slice_0";
      char fn[1000];
      BYTE **acc_im = new_image(h, w);
      for (int i = index_min; i < index_max; i ++)
      {
         char *ds = digit_string(3, i);
         sprintf(fn, "%s%s.png", pre_fix, ds);
         free(ds);
         BYTE **im = NULL;
         orig_img = load_image(fn, h, w);
         set_image(im, orig_img);

         printf("Processing image %s\n", fn);
         for (int x = 0; x < w; x ++)
            for (int y = 0; y < h; y ++)
            {
               if ( y > z_2D_1 && x > x_2D_1 && x < x_2D_2)
                  acc_im[y][x] |= im[y][x];
            }

         if (orig_img) delete orig_img;
         if (im) free_image(im);
      }

      Save2File("result/point_cloud_images/acc_image_seg.png", acc_im, NULL, w, h, 0, NULL);
      free_image(acc_im);
   }
   
}

// Function: infer_taper_get_template
//
// Description: infer the taper structure template, including the intersection point, bottom lines.
//              
//
void infer_taper_get_template(char *fn, vector<int> &return_params)
{
#ifndef LINUX
   Bitmap* orig_img = NULL;
#else
   CImg<BYTE> *orig_img = NULL;
#endif

   assert(file_exist(fn));

   int h, w;
   BYTE **im;
   // load the template file
   orig_img = load_image(fn, h, w);
   set_image(im, orig_img);

   // preprocess
   // sweep from top to bottom
   if (0)
   {
      BYTE **new_im = new_image(h, w);
      for (int x = 0; x < w; x ++)
      {
         for (int y = 0; y < h; y++)
         {
            if (im[y][x] == BLACK)
            {
               new_im[y][x] = BLACK;
               break;
            }
         }
      }
      free_image(im);
      im = new_im;
   }
   
   // hough transform to compute the two crossing lines.
   {
      BYTE** Imbinary,** orig_im;
      
      Imbinary = im;
      orig_im = copy_image(Imbinary, h, w);   

      BYTE *** im_3D = NULL;
      set_3D_image(Imbinary, im_3D, h, w);

      // main loop
      vector<int> temp;
      vector<int> line_set; // line segment vector: x1, y1, x2, y2  ASSERT(line_set.size() % 4 == 0);
      int pdetect, tetadetect;

      int HT_counter = 0;
      vector<int> params;
      while (true)
      {
         my_HT(Imbinary, h, w, pdetect, tetadetect);
         params.push_back(pdetect);
         params.push_back(tetadetect);

         // draw line on the image
         draw_line_on_data(pdetect, tetadetect, Imbinary, h, w, im_3D, orig_im, line_set);
         HT_counter++;

         if (HT_counter == 2) break;

      }
      
      int x, y;
      int ret = compute_intersection_point(params[0], params[1], params[2], params[3], x, y);
      if (ret)
         printf("The intersection point is: %d, %d\n", x, y);
      else
         printf("The intersection point is not existed: %d, %d\n", params[1], params[3]);


      return_params.push_back(x);
      return_params.push_back(y);
      
      // the low end points of the two lines
      {
         assert(line_set.size() == 8);
         for (int i = 0; i < line_set.size(); i += 4) {
            int x, y;
            if (line_set[i+1] > line_set[i+3])
            {
               x = line_set[i];
               y = line_set[i+1];
            }
            else {
               x = line_set[i+2];
               y = line_set[i+3];
            }
            printf("The %d end point is [%d, %d]\n", i/4, x, y);
            return_params.push_back(x);
            return_params.push_back(y);
         }
      }
      
      // debug the HT
      if (0) {
         char *fname = "result/point_cloud_images/my_HT.png";
         write_lines_to_image(fname,line_set, h, w);
         Save2File("result/point_cloud_images/my_3D_HT.png", NULL, NULL, w, h, 1, im_3D);
      }
      
   }
   
}

//void infer_taper_get_extruded_slices(685, 787, 301, 688, 0, 588);
void infer_taper_get_extruded_slices(int y1, int y2, int x1, int x2, int z1, int z2)
{
#ifndef LINUX
   Bitmap* orig_img = NULL;
#else
   CImg<BYTE> *orig_img = NULL;
#endif
   int w,h;

   // get the coords of [x1, x2] and [z1, z2]
   // Note: the coords we are computing is based on the Y axis, namely bottom up slices.
   int box_x1 = (int)((x1/(double)1000)*1024 + .5);
   int box_x2 = (int)((x2/(double)1000)*1024 + .5);
   int box_y1 = 392 - (int)((z2/(double)1000)*392 + .5);
   int box_y2 = 392 - (int)((z1/(double)1000)*392 + .5);

   // get cropped image falling in this box.
   char in_fn[1000];
   char ou_fn[1000];

   // step 1, HD is used to compute the key slices
#if 1
   BYTE **ref_im = NULL;
   gbl_opts->KSD_HD_data_percentage = 0.2;
   for (int i = y1; i <= y2; i ++) {

      char *ds = digit_string(3, i);
      sprintf(in_fn, "%s/image_slice_0%s.png", gbl_opts->input_prefix, ds);
      sprintf(ou_fn, "%s/aaa_image_slice_0%s.png", gbl_opts->output_prefix, ds);
      free(ds);

      if (!file_exist(in_fn))
         printf("ERROR! File is not existed - %s\n", in_fn);

      BYTE **im = NULL;
      orig_img = load_image(in_fn, h, w);
      set_image(im, orig_img);
      BYTE **new_im = new_image(h,w);
      BYTE **free_im = NULL;

      printf("Processing image %s\n", in_fn);
      for (int x = 0; x < w; x ++)
         for (int y = 0; y < h; y ++)
         {
            if (im[y][x] == BLACK &&
                y < box_y2 && y > box_y1 &&
                x < box_x2 && x > box_x1)
               new_im[y][x] = BLACK;
         }

      if ( i == y1) {
         ref_im = new_im;
         Save2File(ou_fn, ref_im, NULL, w, h, 0, NULL);
         continue;
      }

      free_im = new_im;
      if (has_significant_change(ref_im, new_im, h, w, 0))
      {
         free_im = ref_im;
         ref_im  = new_im;
         printf("\n=======Key slice detected : ====%d====\n", i);
         Save2File(ou_fn, ref_im, NULL, w, h, 0, NULL);
      }

      if (orig_img) delete orig_img;
      if (im) free_image(im);
      if (free_im) free_image(free_im);
      
   }
#endif

#if 0   
   char *pst_bpa = "result/point_cloud_images/slices_1000_0.0_44.0_tapered/aaa_image_slice_0";
   // step 2, BPA algorithm, Note: here, we should take the longest line segment into considertation
   for (int i = y1; i <= y2; i ++) {
      char *ds = digit_string(3, i);
      sprintf(in_fn, "%s%s.png", pst_fix, ds);
      sprintf(ou_fn, "%s%s.png", pst_bpa, ds);
      free(ds);

      if (!file_exist(in_fn))
         continue;

      printf("\nProcessing image file: %s\n", in_fn);

      // call BPA algorithm.
      output_fn = ou_fn;
      char *opt = "-r 512 -p 1 -R -M -l 20";
      global_opts = strdup(opt);
      BPA_boundary(in_fn);
   }
#endif   
   
   exit(0);
}

// Function: infer_taper_dump_extruded_segs
//
// Description: Automatically dump the extruded segs
//
// called by infer_taper_use_other_dirs(), after infer_taper_get_extruded_slices();
void infer_taper_dump_extruded_segs(FILE *fd, int frame_num, vector<int> &seg_info)
{
   // get BPA results from the previous process.
   // char *pre_fix = "result/point_cloud_images/slices_1000_0.0_44.0_tapered/aaa_image_slice_0685_dumped_0.txt";
   char fn[1000];

   double pre_tap_height = seg_info[0]/(double)frame_num;
   double cur_tap_height = seg_info[1]/(double)frame_num;

   int theLastLayer = 0;
   for (int i = seg_info[1] ; i >= seg_info[0]; i --)
   {
      char file_name[1000];
      char *ds = digit_string(4, i);
      sprintf(file_name, "%s_%s_dumped_0.txt", gbl_opts->output_prefix, ds);
      free(ds);

      if (file_exist(file_name)) {
         theLastLayer = i;
         break;
      }
   }

   if ( theLastLayer == 0)
      return;
   
   // here, we just ignore the first slice, starting from the 2nd one. (start + 1)
   for (int i = seg_info[0] + 1; i <= seg_info[1]; i ++)
   {
      char *ds = digit_string(4, i);
      sprintf(fn, "%s_%s_dumped_0.txt", gbl_opts->output_prefix, ds);
      free(ds);

      if (!file_exist(fn))
         continue;

      printf("Processing %s ...\n", fn);

      // update the height info.
      cur_tap_height = i/(double)frame_num;

      if ( i == theLastLayer && gbl_opts->IR_do_not_extend_the_last_key_slice_to_one == 0 )
         cur_tap_height = seg_info[1]/(double)frame_num;

      // copy the fixed model code here.
      FILE *ext_fd = fopen(fn, "r");
      char str_line[1000];
      assert(ext_fd);
      char *pre_point;
      while (fgets(str_line, 1000, ext_fd))
      {
         if (strncmp(str_line, "END", 3) == 0)
         {
            fprintf(fd, "EXTRUSION\nHEIGHT\n");
            fprintf(fd, "%f %f\n", pre_tap_height, cur_tap_height);
            fprintf(fd, "END POLYGON\n");
         } else {
            // to avoid the duplicated point;
            if ( pre_point == NULL || strcmp(pre_point, str_line) != 0 ) {
               fprintf(fd, "%s", str_line);
                  
               if (pre_point)
                  free(pre_point);
               pre_point = strdup(str_line);
            }
         }
      }
      fclose(ext_fd);

      // update the height info.
      pre_tap_height = cur_tap_height;
      
   }

   
}

// Function: infer_taper_use_other_dirs
//
// Description: infer the taper structure by segmenting tapered structure recognized from other
//              directions. 
//
// project.exe -a -O "-r <path/to/parent> -p <path/input> -P <path/output>"
void infer_taper_use_other_dirs()
{

   // Init some vars
   gbl_opts->input_prefix = "result/point_cloud_process/slices_200_bottom_up_result";
   gbl_opts->output_prefix = "result/point_cloud_process/slices_200_bottom_up_result";
   char *root_dir="result/point_cloud_process";
   char param_str[1000];

   int  use_taper = 1;
   int  sample_start = 685;
   int  sample_end   = 787;
   
   int  sample_num   = 1000;
   int  sample_extruded_num = 200;
   int  use_manually_extruded = 1;
   int  do_get_extruded_slices = 0;
   
   if (global_opts)
   {
      int  argc = 0;
      char *argv[30];
      load_options(global_opts, argc, argv);
      
      char c;
      char *cmdline;
      util_getopt_reset();
      while ((c = util_getopt(argc, argv, "A:a:bc:C:en:N:p:P:Tr:")) != EOF)
      {
         switch (c) {
            case 'b':
               /* specify the way the extrusion of taper segment */
               use_manually_extruded = 0;
               break;
            case 'e':
               /* do get extruded slices */
               do_get_extruded_slices = 1;
               break;
            case 'T':
               /* no taper */
               use_taper = 0;
               break;
            case 'C':
               /* set thres hold */
               gbl_opts->KSD_HD_diff_threshold = atof(util_optarg);
               break;
            case 'c':
               /* set % */
               gbl_opts->KSD_HD_data_percentage = atof(util_optarg);
               break;
            case 'P':
               /* specify the output folder */
               gbl_opts->output_prefix = strdup(util_optarg);
               break;
            case 'p':
               /* specify the input folder */
               gbl_opts->input_prefix = strdup(util_optarg);
               break;
            case 'r':
               /* specify the root folder */
               root_dir = strdup(util_optarg);
               break;
            case 'a':
               /* specify start sample No. */
               sample_start = atoi(util_optarg);
               break;
            case 'A':
               /* specify end sample No. */
               sample_end = atoi(util_optarg);
               break;
            case 'n':
               /* specify the total number of samples */
               sample_num = atoi(util_optarg);
               break;
            case 'N':
               /* specify the total number of samples of previous steps, may not be the same as sample_num */
               sample_extruded_num = atoi(util_optarg);
               break;
            default:
               break;
         }
      }
   }

   if ( use_taper == 0 ) {
      sample_start = 0;
      sample_end = 0;
   }

   if ( do_get_extruded_slices ) {
      infer_taper_get_extruded_slices(685, 787, 301, 688, 0, 588);
   }
   // step 1. Predefine the possible taper structures, base on prior knowledge, which should be found automatically.
   //         the prior knowledge includes the bottom height of the taper structure, the collapsed template, etc.
   // return: the region of the tapered structure.
   // infer_taper_get_related_info();
   // infer_taper_get_related_info_face();
   // dump_boundary_points_to_IR();
#if 0  // so far, this part is manually one time job.
   vector<int> params;
   vector<double> seg_params;
   sprintf(param_str, "%s/acc_template_lr.png", root_dir);
   infer_taper_get_template(param_str, params);
   assert(params.size() == 6);
   // compute the height of the intersection point
   double t1, t2, t3;
   t1 = get_3D_coordinate_from_2D_pixel_value(0, 1, params[1]);
   t2 = get_3D_coordinate_from_2D_pixel_value(0, 1, params[3]);
   t3 = get_3D_coordinate_from_2D_pixel_value(0, 1, params[5]);
   seg_params.push_back(t1);
   seg_params.push_back(min(t2, t3));
   printf("The 3D height of top point %d is : %f\n", params[1], t1);
   printf("The 3D height of low point %d is : %f\n", params[3], t2);
   printf("The 3D height of low point %d is : %f\n", params[5], t3);

   params.clear();
   sprintf(param_str, "%s/acc_template_face.png", root_dir);
   infer_taper_get_template(param_str, params);

   t1 = get_3D_coordinate_from_2D_pixel_value(2, 1, params[1]);
   t2 = get_3D_coordinate_from_2D_pixel_value(2, 1, params[3]);
   t3 = get_3D_coordinate_from_2D_pixel_value(2, 1, params[5]);
   
   printf("The 3D height of top point %d is : %f\n", params[1], t1);
   printf("The 3D height of low point %d is : %f\n", params[3], t2);
   printf("The 3D height of low point %d is : %f\n", params[5], t3);

   t2 = min(t2, t3);
   
   t1 = max(seg_params[0], t1);
   t2 = min(seg_params[1], t2);
   seg_params.clear();
   seg_params.push_back(t1);
   seg_params.push_back(t2);
   printf("The layers are: [%f - %f]\n", seg_params[0], seg_params[1]);

#endif   

   // step 2. get the extruded segmentation of the building, integrate the tapered segmentation and extruded segs.
   vector<int> seg_info;
   //infer_taper_get_segments(seg_params, seg_info);
   
   seg_info.push_back(sample_start);
   seg_info.push_back(sample_end);

   // infer the key slices for extruded segmentation
   // here, this is only called once, the result will be called by \ref{INFER_SEG_EXTRUDE_IR}
   // \label{INFER_SEG_EXTRUDE}
   // infer_taper_get_extruded_slices(685, 787, 301, 688, 0, 588);
   
   // step 3. dump to IR foramt and generate the DXF file based on the language we defined before.
   sprintf(param_str, "%s/ir.txt", root_dir);
   char *out_fn = strdup(param_str);
   {
      FILE *fd = fopen(out_fn, "w+");
      assert(fd);

      int frame_num = sample_num;

      // compute the height
      double pre_tap_height = seg_info[0]/(double)frame_num;
      double cur_tap_height = seg_info[1]/(double)frame_num;
      
      // dump to IR format of the extruded layout
      if ( use_manually_extruded ) {
         char str_line[1000];
         sprintf(param_str, "%s/extrude.txt", root_dir);
         FILE *ext_fd = fopen(param_str, "r");
         assert(ext_fd);
         char *pre_point;
         while (fgets(str_line, 1000, ext_fd))
         {
            if (strncmp(str_line, "END", 3) == 0)
            {
               fprintf(fd, "EXTRUSION\nHEIGHT\n");
               fprintf(fd, "%f %f\n", pre_tap_height, cur_tap_height);
               fprintf(fd, "END POLYGON\n");
            } else {
               // to avoid the duplicated point;
               if ( pre_point == NULL || strcmp(pre_point, str_line) != 0 ) {
                  fprintf(fd, "%s", str_line);
                  
                  if (pre_point)
                     free(pre_point);
                  pre_point = strdup(str_line);
               }
            }
         }
         fclose(ext_fd);
      } else {
         // this is the true story - automatically generate the extruded structure.
         // \label{INFER_SEG_EXTRUDE_IR}
         // this func should be called after \ref{INFER_SEG_EXTRUDE}
         infer_taper_dump_extruded_segs(fd, frame_num, seg_info);
      }

      // dump the tapered structure X axis
      // x1_s = 66, x1_e = 301; x2_s = 688, x2_e = 923
      // compute the Z_3D of left-right images
      // output of the above computation:
      /*
        The intersection point is: 247, 290
        The 0 end point is [338, 380]
        The 1 end point is [158, 379]
      */

      if ( use_taper ) {   // 0: to use old extruded slice, 1: use new tapered one.
         int IMAGE_WIDTH = 1024;
         int IMAGE_HEIGHT = 392;
         double x_min, y_min, z_min, x_max, y_max, z_max;
         assign_3D_box_Y_dir(x_min, x_max, y_min, y_max, z_min, z_max);
         double z_3D_1 = get_3D_coordinate_from_2D_pixel_value(0, 2, 158);
         double z_3D_2 = get_3D_coordinate_from_2D_pixel_value(0, 2, 338);
         double scale = (double)IMAGE_WIDTH / (x_max - x_min);
         int y_2D_1 = IMAGE_HEIGHT - (int)((z_3D_1 - z_min) * scale + .5) - 1; // z
         int y_2D_2 = IMAGE_HEIGHT - (int)((z_3D_2 - z_min) * scale + .5) - 1; // z
         int x_2D_s_1 = (int)((66 /(double)1000) * IMAGE_WIDTH + .5);
         int x_2D_e_1 = (int)((301/(double)1000) * IMAGE_WIDTH + .5);
         int x_2D_s_2 = (int)((688/(double)1000) * IMAGE_WIDTH + .5);
         int x_2D_e_2 = (int)((923/(double)1000) * IMAGE_WIDTH + .5);
         printf("The y1 and y2 is %d, %d\n", y_2D_1, y_2D_2);
         printf("The x1_s and x1_e is %d, %d\n", x_2D_s_1, x_2D_e_1);
         printf("The x2_s and x2_e is %d, %d\n", x_2D_s_2, x_2D_e_2);

         // dump this tapered structure 1
         fprintf(fd, "BEGIN POLYGON\n");
         fprintf(fd, "%d %d\n", x_2D_e_1, y_2D_1);
         fprintf(fd, "%d %d\n", x_2D_s_1, y_2D_1);
         fprintf(fd, "%d %d\n", x_2D_s_1, y_2D_2);
         fprintf(fd, "%d %d\n", x_2D_e_1, y_2D_2);
         fprintf(fd, "TAPERED\n");
         int y_mid = (y_2D_1 + y_2D_2)/2;
         fprintf(fd, "%d %d\n", x_2D_e_1, y_mid);
         fprintf(fd, "%d %d\n", x_2D_s_1, y_mid);
         fprintf(fd, "%d %d\n", x_2D_s_1, y_mid);
         fprintf(fd, "%d %d\n", x_2D_e_1, y_mid);
         fprintf(fd, "HEIGHT\n");
         fprintf(fd, "%f %f\n", pre_tap_height, cur_tap_height);         
         fprintf(fd, "END POLYGON\n");

         // dump this tapered structure 2
         fprintf(fd, "BEGIN POLYGON\n");
         fprintf(fd, "%d %d\n", x_2D_e_2, y_2D_1);
         fprintf(fd, "%d %d\n", x_2D_s_2, y_2D_1);
         fprintf(fd, "%d %d\n", x_2D_s_2, y_2D_2);
         fprintf(fd, "%d %d\n", x_2D_e_2, y_2D_2);
         fprintf(fd, "TAPERED\n");
         //int y_mid = (y_2D_1 + y_2D_2)/2;
         fprintf(fd, "%d %d\n", x_2D_e_2, y_mid);
         fprintf(fd, "%d %d\n", x_2D_s_2, y_mid);
         fprintf(fd, "%d %d\n", x_2D_s_2, y_mid);
         fprintf(fd, "%d %d\n", x_2D_e_2, y_mid);
         fprintf(fd, "HEIGHT\n");
         fprintf(fd, "%f %f\n", pre_tap_height, cur_tap_height);         
         fprintf(fd, "END POLYGON\n");
         
      }

      // dump the tapered structure Z axis
      // z1_s = 803  , z1_e = 588
      // output of the above computation:
      /*
        The intersection point is: 508, 203
        The 0 end point is [562, 260]
        The 1 end point is [451, 258]
      */
      if ( use_taper ) {   // 0: to use old extruded slice, 1: use new tapered one.
         int IMAGE_WIDTH = 1024;
         int IMAGE_HEIGHT = 392;
         double x_min, y_min, z_min, x_max, y_max, z_max;
         assign_3D_box_Z_dir(x_min, x_max, y_min, y_max, z_min, z_max);
         double z_3D_1 = z_min + (803/(double)1000) * (z_max - z_min);
         double z_3D_2 = z_min + (588/(double)1000) * (z_max - z_min);
         double x_3D_1 = get_3D_coordinate_from_2D_pixel_value(2, 0, 451);
         double x_3D_2 = get_3D_coordinate_from_2D_pixel_value(2, 0, 562);

         // z_i = IMAGE_HEIGHT - (int)((slice_data[i] - BOX_MIN_Z) * scale + .5) - 1; // z
         // reset the 3D BOX !!!!!!!!!!
         assign_3D_box_Y_dir(x_min, x_max, y_min, y_max, z_min, z_max);

         int x_2D_1 = (int)(((x_3D_1 - x_min)/(x_max - x_min))*IMAGE_WIDTH  + .5); // x
         int x_2D_2 = (int)(((x_3D_2 - x_min)/(x_max - x_min))*IMAGE_WIDTH  + .5); // x
         printf("The x1 and x2 is %d, %d\n", x_2D_1, x_2D_2);

         double scale = (double)IMAGE_WIDTH / (x_max - x_min);
         printf("z_min: %f, z_max: %f, z_3D_1:%f\n", z_min, z_max, z_3D_1);
         int y_2D_s_1 = IMAGE_HEIGHT - (int)((z_3D_1 - z_min) * scale + .5) - 1; // z
         int y_2D_e_1 = IMAGE_HEIGHT - (int)((z_3D_2 - z_min) * scale + .5) - 1; // z
         printf("The y1 and y2 is %d, %d\n", y_2D_s_1, y_2D_e_1);
         
         // dump this tapered structure 2
         fprintf(fd, "BEGIN POLYGON\n");
         fprintf(fd, "%d %d\n", x_2D_1, y_2D_e_1);
         fprintf(fd, "%d %d\n", x_2D_1, y_2D_s_1);
         fprintf(fd, "%d %d\n", x_2D_2, y_2D_s_1);
         fprintf(fd, "%d %d\n", x_2D_2, y_2D_e_1);
         fprintf(fd, "TAPERED\n");
         int x_mid = (x_2D_1 + x_2D_2)/2;
         fprintf(fd, "%d %d\n", x_mid, y_2D_e_1);
         fprintf(fd, "%d %d\n", x_mid, y_2D_s_1);
         fprintf(fd, "%d %d\n", x_mid, y_2D_s_1);
         fprintf(fd, "%d %d\n", x_mid, y_2D_e_1);
         fprintf(fd, "HEIGHT\n");
         fprintf(fd, "%f %f\n", pre_tap_height, cur_tap_height);         
         fprintf(fd, "END POLYGON\n");
      }

      // dump the remaining parts      
      double pre_height = 0.0;
      bool is_low_covered = false;
      frame_num = sample_extruded_num;
      double s = (double)(sample_extruded_num)/(double)(sample_num);
      vector<double> seg_info_d(2);
      seg_info_d[0] = s * seg_info[0];
      seg_info_d[1] = s * seg_info[1];

      int theLastLayer = 0;
      for (int i = frame_num - 1 ; i >= 0; i --)
      {
         char file_name[1000];
         char *ds = digit_string(3, i);
         sprintf(file_name, "%s/bbb_image_slice_0%s_dumped_0.txt", gbl_opts->input_prefix, ds);
         free(ds);

         if (file_exist(file_name)) {
            theLastLayer = i;
            break;
         }
      }
      assert(theLastLayer != 0);
      
      for (int i = 0 ; i < frame_num; i ++)
      {
         if ((i < seg_info_d[0] || i > seg_info_d[1]) || !is_low_covered)
         {
            // reset the pre_height
            if (i > seg_info_d[1] && pre_height <= seg_info_d[0]/(double)frame_num)
               pre_height = seg_info_d[1]/(double)frame_num;
            
            char file_name[1000];
            char *ds = digit_string(3, i);
            sprintf(file_name, "%s/bbb_image_slice_0%s_dumped_0.txt", gbl_opts->input_prefix, ds);
            free(ds);

            if (!file_exist(file_name))
               continue;

            printf("Dumping the file index: %d\n", i);
            FILE *in_fd = fopen(file_name, "r");
            assert(in_fd);
         
            // compute the height
            double cur_height = i/(double)frame_num;

            if ( theLastLayer == i && gbl_opts->IR_do_not_extend_the_last_key_slice_to_one == 0 )
               cur_height = 1.0;   // find the last one
            
            // Make sure the layer between seg_info[0] and preview layer is filled.
            if (i > seg_info_d[0] && !is_low_covered && use_taper) {
               is_low_covered = true;
               cur_height = seg_info_d[0]/(double)frame_num;
            }
               
            // find a key slice, dump it into IR.
            char str_line[1000];
            char *pre_point = NULL;
            while (fgets(str_line, 1000, in_fd))
            {
               if (strncmp(str_line, "END", 3) == 0)
               {
                  fprintf(fd, "EXTRUSION\nHEIGHT\n");
                  fprintf(fd, "%f %f\n", pre_height, cur_height);
                  fprintf(fd, "END POLYGON\n");
                  pre_point = NULL;
               } else {
                  // to avoid the duplicated point;
                  if ( pre_point == NULL || strcmp(pre_point, str_line) != 0 ) {
                     fprintf(fd, "%s", str_line);

                     if (pre_point)
                        free(pre_point);
                     pre_point = strdup(str_line);
                  }
               }
            }
            fclose(in_fd);

            // update the vars
            pre_height = cur_height;
         
         }

      }

      fclose(fd);
      
   }

   // generate DXF
   // image_width = 1024;
   // image_height = 392;
   // generate_dxf_from_IR(out_fn);
}

