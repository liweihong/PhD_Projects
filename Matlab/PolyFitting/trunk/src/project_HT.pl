### For Interior Reconstruction.
### noise remove with 3D bounding box, remove the left small wall, 
### remove the chairs.

### The complicated 3D bounding box generation methods:
### USE MATALAB
### imshow("file");
### BW=roipoly();
### BW=imcomplement(BW);
### imwrite(BW, "file_mask");

use strict;
use warnings;
use POSIX qw(strftime);

my $os = 0; # 0: Windows, 1: Linux

my $del;
my $cop;
my $del_dir;
my $cop_dir;

if ( $os == 0 ) {
    $del = "del";
    $cop = "copy";
    $cop_dir = "xcopy /e /i";
    $del_dir = "rd /s /q";
} else {
    $del = "rm";
    $cop = "cp";
    $cop_dir = "cp -r";
    $del_dir = "rm -rf";
}

sub clear_sub_all {
    system ($del, "log_4_key_slice.txt");
    system ($del, "log_5_BPA.txt");
    system ($del, "log_7_error.txt");
    system ($del, "log_8_2_insert.txt");
    system ($del, "log_8_3_comp.txt");
    system ($del, "log_8_3_BPA.txt");
    system ($del, "log_9_2_infer.txt");
}

sub clear_sub_error {
#    system ($del, "log_5_BPA.txt");
    system ($del, "log_9_2_infer.txt");
#    system ($del, "log_8_3_comp.txt");
#    system ($del, "log_8_3_BPA.txt");
    system ($del, "log_7_error.txt");
}

sub clear_sub_face {
    system ($del, "log_2_noise.txt");
    system ($del, "log_3_recovered.txt");
    system ($del, "log_4_key_slice.txt");
    system ($del, "log_5_BPA.txt");
    system ($del, "log_6_IR_ex.txt");
#    system ($del, "log_7_error.txt");
#    system ($del, "log_9_2_infer.txt");
#    system ($del, "log_8_3_comp.txt");
#    system ($del, "log_8_3_BPA.txt");
}

sub proj_sub {

my $start_time = strftime "%H:%M:%S", localtime;
my $end_time;

# two sets of resolution:
# 1. # of samples for each axis.
# 2. image size for projection of each axis.
# 3. for error measurement, we should do it on 3D distance because of different image size/resolution.

# global settings
my $sample_num=1000;
my $root_dir="result/point_cloud_HT_$sample_num";
my $dataset="../data/pts/asm512.ptx";
my $im_w=640;
my $im_h=800;
my $lr_sample_num=$sample_num;
my $lr_im_w=512;
my $lr_im_h=1024;
my $fi_sample_num=$sample_num;
my $fi_im_w=640;
my $fi_im_h=800;

#my $ksd_HD_thres=10.0; # HD based similarity measurement
my $ksd_GC_thres=$_[3]; # Graph Cut based similarity measurement
my $ksd_HD_thres=$_[1]; # HD based similarity measurement
my $bpa_radius_min_of_ksd=1; #minimum radius for key slice detection. (work with $ksd_HD_thres);
my $ksd_BPA_ratio=0.05 * ($ksd_HD_thres/4.0);# The ratio that the curvature is regarded as key slice
my $bpa_radius_min=$_[0]; # minimum refinement radius size
#my $bpa_radius_min=1; # minimum refinement radius size
#my $bpa_radius_min=8; # minimum refinement radius size

if ( ! -e $root_dir ) {
    mkdir $root_dir;
}

# step 1&2: generate the sampled slices, see<load_point_cloud_as_images()>
# step 2: noise clean based on the bounding box has also been done in the loading step.
# Command options - project.exe -l ..\data\Rotated.bpa -O "-s total_slices -p -x -X -y -Y -z -Z -d 0/1/2"
# project.exe -l ..\data\Rotated.bpa -O "-s 1000 -d 0" // get 1k sliced image bottom up
# project.exe -l ..\data\Rotated.bpa -O "-s 1000 -w 1000 -h 2048 -Z 45.0 -d 2" // get 1k sliced image left - right

# -O "-p" set the output path of the sliced images
# -O "-w" set the image width
# -O "-h" set the image height
# -O "-d" set the slice direction: 0 - bottom-up, 1 - face-inside. 2 - left-right
# -O "-xXyYzZ" set the 3D bounding box

my $load_path_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up";
if ( ! -e $load_path_dir ) {
    mkdir $load_path_dir ;
}

my $load_lr_path_dir="$root_dir/slices_$lr_sample_num\_$lr_im_w\_$lr_im_h\_left_right";
if ( ! -e $load_lr_path_dir ) {
    mkdir $load_lr_path_dir ;
}

my $load_fi_path_dir="$root_dir/slices_$lr_sample_num\_$fi_im_w\_$fi_im_h\_face_inside";
if ( ! -e $load_fi_path_dir ) {
    mkdir $load_fi_path_dir ;
}

#exit; # to  manually improve the BPA results. 
my $compute_direction=$_[2];  # 0: bottom-up, 1: left-right, 2: face-inside
my $not_ignore_step_1=0;  # load data
my $not_ignore_step_2=0;  # noise removal
my $not_ignore_step_3=0;  # symmetry computaton
my $not_ignore_step_4=1;  # HD keyslices detection
my $not_ignore_step_5=0;  # BPA
my $not_ignore_step_6=1;  # model generation without taper
my $not_ignore_step_7=0;  # error computaton
my $not_ignore_step_8=0;  # curvature keyslices detection
my $not_ignore_step_9=0;  # taper structure model generation

my $x_min = -9.17;
my $x_max = 13.06;
my $y_min = -8.05;
my $y_max = 15.40;
my $z_min = -1.04;
my $z_max = 10.50;
# -x -9.17 -X 13.06 -y -8.05 -Y 15.40 -z -1.04 -Z 10.50

my $log_load="log_1_2_load.txt";
if ( $not_ignore_step_1 && ( ! -e $log_load || -z $log_load)) {
# project -l ..\data\asm512.ptx -O "-p result\point_cloud_interior_1000\slices_1000_1024_392_fi -x -9.17 -X 13.06 -y -8.05 -Y 15.40 -z -1.04 -Z 10.50 -d 1 -w 640 -h 800 -s 1000"
    my $cmd_load_point_cloud="project -l ../data/pts/asm512.ptx -O \"-s $sample_num -p $load_path_dir -w $im_w -h $im_h -d 1 -x -9.17 -X 13.06 -y -8.05 -Y 15.40 -z -1.04 -Z 10.50 -t 1\" > $log_load";
    print "$cmd_load_point_cloud\n";
    system $cmd_load_point_cloud;
}

my $log_ksd_prep_load="log_1_8_load.txt";
if ( $not_ignore_step_1 && ( ! -e $log_ksd_prep_load || -z $log_ksd_prep_load)) {
    my $cmd_prep_load_point_cloud="project -l ../data/pts/asm512.ptx -O \"-s $lr_sample_num -p $load_lr_path_dir -w $lr_im_w -h $lr_im_h -d 2 -x -9.17 -X 13.06 -y -8.05 -Y 15.40 -z -1.04 -Z 10.50 -t 1\" > $log_ksd_prep_load";
    print "$cmd_prep_load_point_cloud\n";
    system $cmd_prep_load_point_cloud;
}

my $log_taper_load="log_1_9_load.txt";
if ( $not_ignore_step_1 && ( ! -e $log_taper_load || -z $log_taper_load)) {
    my $cmd_taper_load="project -l ../data/pts/asm512.ptx -O \"-s $fi_sample_num -p $load_fi_path_dir -w $fi_im_w -h $fi_im_h -d 0 -x -9.17 -X 13.06 -y -8.05 -Y 15.40 -z -1.04 -Z 10.50 -t 1\" > $log_taper_load";
    print "$cmd_taper_load\n";
    system $cmd_taper_load;
}

#step 2 : noise removal based on mask images, see <noise_removal_by_mask()>
my $clear_path_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up_clear";

my $log_noise="log_2_noise.txt";
if ( $not_ignore_step_2 && ( ! -e $log_noise || -z $log_noise)) {
    if ( ! -e $clear_path_dir ) {
	mkdir $clear_path_dir ;
    }
    my $cmd_clear="project -M -O \"-s 0 -e $sample_num -r $load_path_dir/image_slice -w $clear_path_dir/image_slice -m $load_path_dir\_masks\" > $log_noise";
    print "$cmd_clear\n";
    system $cmd_clear;
}


# step 3: symmetry computing for data recovering. see <compute_simple_symmetry_test()>
# project.exe -O "-s 0 -e 999 -p <out_path> -P <in_path>" -1
# Comment: This command assume the symmetry line is x = 507. To compute this value,
# we have to re-shape the function. 
my $reco_path_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up_recovered";

my $log_recover="log_3_recovered.txt";
if ( $not_ignore_step_3 && ( ! -e $log_recover || -z $log_recover)) {
    if ( ! -e $reco_path_dir ) {
	mkdir $reco_path_dir ;
    }
    my $cmd_recover="project -O \"-s 0 -e $sample_num -p $reco_path_dir -P $load_path_dir\" -1 > $log_recover";
    print "$cmd_recover\n";
    system $cmd_recover;
} else {
	if ( $not_ignore_step_2 ) {
	    my $cmd_del = "$del_dir $reco_path_dir";
	    print "$cmd_del\n";
	    system $cmd_del;
	}

	if ( ! -e $reco_path_dir ) {
	    # we have to move files, which the following steps rely on
	    my $cmd_mv = "$cop_dir $clear_path_dir $reco_path_dir";
	    if ( $os == 0 ) {
		$cmd_mv = "$cop_dir \"$clear_path_dir\" \"$reco_path_dir\" > $log_recover";
	    }
	    print "$cmd_mv\n";
	    system $cmd_mv;
	}
}

# step 4: key slices detection, See the function <key_slices_detection> for more options.
# project.exe -K -O "-L 0 -R 999 -P <path/to/prefix>"
my $bnd_path_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up_boundary";
if ( ! -e $bnd_path_dir ) {
    mkdir $bnd_path_dir ;
}
my $result_path_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up_result_$sample_num\_$ksd_HD_thres\_$bpa_radius_min";
if ( ! -e $result_path_dir ) {
    mkdir $result_path_dir ;
}

my $result_seg_path_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up_seg_result_$sample_num\_$ksd_HD_thres\_$bpa_radius_min";
if ( ! -e $result_seg_path_dir ) {
    mkdir $result_seg_path_dir ;
}

my $log_ksd="log_4_key_slice.txt";
my $ksd_ref=60;
my $ksd_end=880;
my $ksd_path_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up";
if ( $not_ignore_step_4 && (! -e $log_ksd || -z $log_ksd)) {
#    my $cmd_ksd="project -K -O \"-a 0 -d $ksd_HD_thres -r $ksd_ref -R $ksd_end -P $ksd_path_dir -O $result_path_dir\" > $log_ksd";
    my $cmd_ksd="project -K -O \"-L $ksd_ref -R $ksd_end -P $reco_path_dir/image_slice -O $result_seg_path_dir -f seg_ir.txt -A 1 -s 1000 -b 5 -B 0.5 -c 0.4 -t $ksd_GC_thres\" > $log_ksd";
    print "$cmd_ksd\n";
    system $cmd_ksd;
}
$end_time = strftime "%H:%M:%S", localtime;
print "Start at: $start_time, and end at: $end_time\n";

# step 8: introduce BPA based key slice detection. see <key_slices_detection_prep()>
my $load_lr_path_result="$load_lr_path_dir\_result/";
if ( ! -e $load_lr_path_result ) {
    mkdir $load_lr_path_result ;
}

my $old_ksd_samples=1000;
my $ksd_ratio=$sample_num/$old_ksd_samples;
my $ksd_curv_start=700*$ksd_ratio;
my $ksd_curv_end=888*$ksd_ratio;
my $log_ksd_prep_insert="log_8_2_insert.txt";
if ( $not_ignore_step_8 && (! -e $log_ksd_prep_insert || -z $log_ksd_prep_insert) ){
    my $cmd_prep_insert="project -k -O \"-a 0 -w $lr_im_w -h $lr_im_h -p $load_lr_path_dir/ -P $load_lr_path_result -s $ksd_curv_start -e $ksd_curv_end -R $bpa_radius_min_of_ksd -t $lr_sample_num -O $result_path_dir -Q $load_path_dir -N -A $ksd_BPA_ratio\" > $log_ksd_prep_insert";
    print "$cmd_prep_insert\n";
    system $cmd_prep_insert;

}

# step 5: BPA for boundary vectorization. see <system_batch_process()> and <BPA_boundary()>
#  The following is for individual BPA algorithm
#   Command: project -o output filename -B filename -O "" 
#      project -S -O "-a 198 -A 199 -m 0 -P result/point_cloud_process/slices_200_1024_392_bottom_up_result -l 512 -p 1 -r 512 -o 0 -f -R -t 4 -H"
#  The following is for batch process:
# -O "-a" : the starting number;
# -O "-A" : the ending number;
# -O "-f" : do NOT clear the data in the first circle.
# -O "-l" : the length of gap
# -O "-o" : the starting point sweeping direction 0 : bottom-up, 1 : left-right, 2: top-down, 3: right-left
# -O "-m" : do NOT ignore the multiple boundaries
# -O "-P" : the path to the result;
# -O "-R" : do the refinement on the number of -t
# -O "-t" : the number of refinement
# -O "-H" : combine the BPA with HT
# -O "-E" : if the "xxx_dump_0.txt" is existed, do not process this slice. This is used for later inserted key slices.
# -O "-I <THRES>" : whether to reshape the slope line, like / to _|, <THRES> is the minimun length threshold
# -O "-C <LEN>" : The minimum length to distinguish a new circle's centroid with existed ones (for 8-circle pillar). This is effect only when $bpa_ignore_multi_bnds is false.
# -O "-b <N>" : The maxinum number of refinement. for small unit, use smaller one. default one is 40. 
#               This option is particularly useful for segmented extruded unit boundary inference.
# -O "-L <N>" : Mininum refinement radius length

my $log_bpa="log_5_BPA.txt";
my $bpa_starting_dir=0; # sweeping direction; 0 : bottom-up, 1 : left-right, 2: top-down, 3: right-left
my $bpa_gap_length=512; # minumun gap length;
my $bpa_ignore_multi_bnds=0; # whether ignore multiple boundaires, like 8 circles. 1: ignore 0: keep multiple boundaries.
                             # for special structure, set this to 0. (redo)
my $bpa_radius_size=700;
my $bpa_reshape_len=64;
my $bpa_centroid_diff=40;    # test is good for 1024x392
my $bpa_do_ras2vec=1;
if ( $not_ignore_step_5 && (! -e $log_bpa || -z $log_bpa)) {
#project.exe -S -O "-a 70 -A 71 -f -P result/point_cloud_process_1000/slices_1000_1024_392_bottom_up_result_1000_4_1 -l 512 -m 0 -o 0 -p 1 -r 700 -R -L 4 -H -I 64 -C 40"
#    my $cmd_bpa="project.exe -S -O \"-a 0 -A $sample_num -f -P $result_path_dir -l $bpa_gap_length -m $bpa_ignore_multi_bnds -o $bpa_starting_dir -p 1 -r $bpa_radius_size -R -L $bpa_radius_min -H -I $bpa_reshape_len -E -C $bpa_centroid_diff\" > $log_bpa";
    my $cmd_bpa="project -L param.ini -S -O \"-a 0 -A $sample_num -P $result_path_dir -r $bpa_do_ras2vec\" > $log_bpa";
    print "$cmd_bpa\n";
    system $cmd_bpa;
}
$end_time = strftime "%H:%M:%S", localtime;
print "Start at: $start_time, and end at: $end_time\n";

# step 8_3: create a new set of tapered seg
my $load_seg_extruded_path_result="$root_dir/slices_1000_1024_392_tapered_result_$sample_num\_$ksd_HD_thres\_$bpa_radius_min";
if ( ! -e $load_seg_extruded_path_result ) {
    mkdir $load_seg_extruded_path_result ;
}
$log_ksd_prep_insert="log_8_3_comp.txt";
if ( $not_ignore_step_8 && (! -e $log_ksd_prep_insert || -z $log_ksd_prep_insert) ){

    # re-compute the key slice of the segments.
   my $cmd_bpa="project -a -O \"-e -C $ksd_HD_thres -P $load_seg_extruded_path_result -p $reco_path_dir\" > $log_ksd_prep_insert";
   print "$cmd_bpa\n";
   system $cmd_bpa;
}

$log_ksd_prep_insert="log_8_3_BPA.txt";
if ( $not_ignore_step_8 && (! -e $log_ksd_prep_insert || -z $log_ksd_prep_insert) ){

    # we are about to do BPA on new inserted slices ONLY (-E).
    my $cmd_bpa="project -S -O \"-a 0 -A $sample_num -f -P $load_seg_extruded_path_result -O $load_seg_extruded_path_result -l $bpa_gap_length -m 1 -o $bpa_starting_dir -p 1 -r $bpa_radius_size -R -L $bpa_radius_min -H -I $bpa_reshape_len -E -C $bpa_centroid_diff\" >> $log_ksd_prep_insert";
   print "$cmd_bpa\n";
   system $cmd_bpa;

}

# step 9: introduce taperes structure inference. see <infer_taper_use_other_dirs()>
# -O "-b" : do NOT use manually created tapered segmented extrusion unit
#my $old_taper_dir="result/point_cloud_images_old/slices_1000_0.0_44.0_tapered/bbb_image_slice";
my $old_taper_dir="$load_seg_extruded_path_result/bbb_image_slice";
my $old_taper_samples=1000;
my $taper_sample_start=685; #a:s A:e
my $taper_sample_end=787;
my $log_taper_infer="log_9_2_infer.txt";
if ( $not_ignore_step_9 && (! -e $log_taper_infer || -z $log_taper_infer)) {
#project.exe -a -O "-r result/point_cloud_process_1000 -b -p result/point_cloud_process_1000/slices_1000_1024_392_bottom_up_result_1000_4_1 -P result/point_cloud_process_1000/slices_1000_1024_392_tapered_result_1000_4_1/bbb_image_slice -n 1000 -N 1000"
    my $cmd_taper_infer="project -a -O \"-r $root_dir -b -p $result_path_dir -P $old_taper_dir -n $old_taper_samples -N $sample_num\" > $log_taper_infer";
    print "$cmd_taper_infer\n";
    system $cmd_taper_infer;
}

# step 6: generate the IR representation. see <dump_boundary_points_to_IR()>
# Project.exe -E -O "-s 0 -e 200 -w 1024 -h 392 -p result/point_cloud_process/slices_200_1024_392_bottom_up_result"
my $ir_ex_path_dir=$result_path_dir;
my $log_ir_ex="log_6_IR_ex.txt";
if ( $not_ignore_step_6 && (! -e $log_ir_ex || -z $log_ir_ex) ) {
#    my $cmd_ir_ex="project -E -L param.ini -O \"-a 0 -s 0 -e $sample_num -w $im_w -h $im_h -p $result_path_dir\" > $log_ir_ex";
#    print "$cmd_ir_ex\n";
#    system $cmd_ir_ex;

    # generate iv from IR
#    my $cmd_iv="project -j -O \"-p $root_dir/ir.txt -P $root_dir/model.iv -t 7 -w $im_w -M 1 -x $x_min -X $x_max -y $y_min -Y $y_max -z $z_min -Z $z_max -D 0 -o 1\" >> $log_ir_ex";
    my $cmd_iv="project -j -O \"-p $result_seg_path_dir/seg_ir.txt -P $result_seg_path_dir/seg_ir_$ksd_GC_thres.iv -t 7 -w $im_w -M 1 -x $x_min -X $x_max -y $y_min -Y $y_max -z $z_min -Z $z_max -D 0 -o 1\" >> $log_ir_ex";
   print "$cmd_iv\n";
    system $cmd_iv;
}


# step 7: error measurement generation. see <compute_error_for_IR()>
# project.exe -D IR.txt -O "-s <N> -r <range> -f <face_width> -p <path> -w -h"
# Project.exe -D result\point_cloud_process_1000\ir_1000_8_4.txt -O "-P result\error_comp\texture_1000_8_4 -p ../data/Rotated.bpa"
my $err_dir="$root_dir/slices_$sample_num\_$im_w\_$im_h\_bottom_up_recovered";
my $log_err="log_7_error.txt";
if ( $not_ignore_step_7 && (! -e $log_err || -z $log_err)) {

    # create a folder for texture images.
    my $err_texture_path_dir="$root_dir/texture_$sample_num\_$ksd_HD_thres\_$bpa_radius_min";
    if ( ! -e $err_texture_path_dir ) {
	mkdir $err_texture_path_dir ;
    }

    my $cmd_err="project -D $root_dir/ir.txt -O \"-s $sample_num -w $im_w -h $im_h -a 3 -e 4 -m 1 -L ../data/pts/asm512.ptx -P $root_dir/texture_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\" > $log_err";
    print "$cmd_err\n";
    system $cmd_err;

    # generate the .iv file for ivview with texture mapping
    my $cmd_texture="project -j -O \"-t 10 -w 640 -M 1 -p $root_dir/ir.txt -P $root_dir/texture_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\" >> $log_err";
    print "$cmd_texture\n";
    system $cmd_texture;
}

# move the result to a new folder
my $need_copy=1;

if ( $need_copy == 1) {
    if ( $os == 0 ) {
	$root_dir =~ s/\//\\/g;
	system ($cop, "$root_dir\\ir.txt", "$root_dir\\ir_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\.txt");
	system ($cop, "$root_dir\\model.iv", "$root_dir\\model_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\.iv");
#    system ($cop, "$root_dir\\error_comp_result.txt", "$root_dir\\error_comp_result_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\.txt");
#    system ($cop, "$root_dir\\ir.txt", "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\ir_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\.txt");
#    system ($cop, "$root_dir\\error_comp_result.txt", "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\error_comp_result_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\.txt");
    } else {
	system ($cop, "$root_dir/ir.txt", "$root_dir/ir_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\.txt");
	system ($cop, "$root_dir/model.iv", "$root_dir/model_$sample_num\_$ksd_HD_thres\_$bpa_radius_min\.iv");
    }
}

$end_time = strftime "%H:%M:%S", localtime;
print "Start at: $start_time, and end at: $end_time\n";

############################# PROBLEMS #####################
##
##  REMAINING:
##  *. generate different resolution, as well as BPA size.
##  *. generate WIREFRAME, ERROR, COLOR_FACE, WHITE_FACE
##  *. put the error and wireframe into the paper.
## 
##  URGENT:
##  *. step 8, the mapping from curvatures to slices is not correct!
##
##  LOW PRIORITY:
##  *. tapered extruded part is not re-generated yet.
##

}

my $i;
for ( $i = 1 ; $i >=1; $i = $i / 2 ) {
#    clear_sub_error;
#    if ( $i == 2 ) {
#	next;
#    }
    clear_sub_face;
    proj_sub 1, $i, 0, 0.2;  # argv[0]: BPA radius | argv[1]: HD threshold
}
