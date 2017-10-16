This file mainly describe the results of the project.

TODO on 3/6:

*. segment roof data based on walls detected in multiple faces.

   *. compute region for dumping
      *. compute the region of wall
      *. transform them to the new coordinates
      *. compute hard boundaries of the lines.
         *. for boundary lines, compute the intersection points (4 points)
         *. for internal lines, check for each end point, what is the intersection line.
            if the line is a boundary, get the point.
            if the line is another internal line (compute the extension point)
         *. compute polygon for each walls.

   *. roof structure inference
   *. roof reconstruction

*. window and doors detection on body.

   *. windows thickness
   *. extrusion on body

GENERAL GOAL:
*. Give a 3D dataset, produce a rough model (SketchUp) for texture mapping AUTOMATICALLY.
*. Model major structures for the 3D dataset. May ignore manority structures.

Goal for CU synthetic data:
*. model walls, window and door frames
*. model taper-to-point?

What we have:
*. key-slices based reconstruction (for body - roof segmentation)
   limitation: windows, doors, taper structures are noise
*. segmentation based taper-structure detection. (not automatical)
*. walls, windows, doors inference. (not automatical, for roof part segmentation)


IDEA:

*. LAYER BASED SEGMENTATION: 
    Layer 1: building body and roof
    Layer 2.1: facades, walls, doors
    Layer 2.2: segment roof to facades, taper structures if any, extrusion structures if any, and others.

    *. Layer 1 detection: the last ledger when tapered structure was detected.
    *. Layer 2.1: *. segment two attachments from two facades based on wall detection.
		  *. once walls and doors are detected, mask them for extrusion computation.
    *. Layer 2.2: segment based on slices from 4 directiions 
		  *. mark it if a wall detected from any direction.
		  *. for CU, we can obtain 4 walls and 5 segments.

*. WINDOWS/DOORS DETECTION: use existed code

*. THICKNESS of DOORS and WINDOWS: hard problem

*. MERGE of DIFFERENT SEGMENTS:
    *. Mark walls being separated due to segmentation.
    *. Merge edges/vertices with closed points/edges
   

IMPLEMENTATION:

*. To identify body/roof separation, search for special pattern ledgers show: thick big boundary
   ( not a massive dark, but mostly expansion of the boundary, how to compute?
     use HD distance from ledger slice to neighbor slices: $d_HD$ > 4.0 && $d_HD$ < 8.0 )
   -. If found multiple patterns, regards the highest one as separation.
   -. If none found, no body/roof separation for this dataset (such as sample model)

*. For either body or roof, do segmentation again.
   -. If a wall detected, mark the location in line.
   -. Draw the lines in bottom-up direction for debug.
   -. After this segmentation, we may still need to analysis each one with extrusion and tapered, such as example model.

*. HOW-TO:
   -. Bring all lines together and display them in an image?
   -. Eliminate gaps among different units?
   -. Carry out computation on particular unit surrounded by other unit?

*. Output:
   -. The image contains all ledge lines in all faces.
   -. The image contains all segmentations for both body and roof.

To-Do:
*. try the CU synthetic point cloud dataset: obtain sliced image, compute the windows and doors.
*. investigate the plane detection using Hadi's planar HT.

Status
*. 2.17.10: generated the bottom-up slices; 
            obtain the major axises for transformation by: A. Verticle line + B face normal, as r1,r2,compute T as in rectify_scans()

Status (2.16.10)
*. Think about how to improve adjacent layers (belongs to the same plane)
*. Roof part of the Hunter theater dataset.
*. Study the reviews from SIGGRAPH and ACCV, especially the comparison part.

Status (8.25.09):
*. Finished the local segmentation coding and tested on a small region.
*. Plans is: 
     A. Understand Graph Cut threshold to control regions, related it to mask.
     B. Use the small region (GCT: 367-470: 0.9-0.8) to study add/remove control points.
     C. Run this on more datasets.

Status (8.16.09):
*. Study Graph Cut and Level set, use this as systematic framework for writing.
*. Implement my own segmentation algorithm on real datasets.

Status (7.30.09):
*. Writing up the big O for each algorithms.
*. All dataset have been created via ras2vec, some improvements include:
   *. Binding ras2vec library to project executable.
   *. Re-design the masks files for new reso.
   *. Split inside scanner to roof and body parts.

Status (7.26.09):
*. All 2D slices have been generated after tuning the 3D bounding box.
*. The 2D slices have been resized, which means the masks have to be re-generated.
    -- The best way is to mapping the masks back to 3D space. How???
*. The CU dataset has problem of low-resolution on top part of the building (windows are not good).
*. The SH dataset has double boundary of the curves.
*. The SH dataset assertion failure on 1024x1024 on BPA computing.
*. The GCT dataset changed the code of model generation on horizontal and vertical directions.


 

3D Model Reconstruction from 4 more datasets:
1. Thomas Hunter and Hunter Theater can be done gradually based on \tau_d.
2. Worked on GCT, with very simple structure.
3. Another interior Great_Hall is not started.
4. Shepard_Hall is a low-resolution one.
5. Cooper_Union is a dataset similiar to Thomas Hunter.

SketchUp Ruby API:
A. to install the PlugsIn, add the building.rb in the c:\prog~\Goo~\Google SketchUp 6/7\PlugsIn
B. to print out the script file, use "project.exe -C result\point~\dumped_ir.txt"
C. check the building.rb for details of the language.

A. Experiences:
====. To debug a problem, we can use mspaint.exe to copy a portion of the image, 
		rotate it and then paste it onto another part (from vertical to horizontal).
====. To print out debug images, we can use: -O "-d -h <int> -e <int>", print out image counter > h, < e, or just > h or 
		just < e. "-c <n>", the nth refinement.

B. Improvements:
(Rev 282) * Try to connect automatically the point on the same lines. (vertical, horizontal, or a slip line)
(adjust the threshold to .9 in function BPA_check_local_match()) * Try to fix the problem of combining HT and BPA, where HT lines are used to replace the BPA lines inappropriately, namely it elimated a lot of details.
(Rev 229) * Try to match the colors for the lines when doing refinement, namely if the line does not change, use the same color for different radius.
(Rev 233) * If we adapted Rev 231, sometimes it is too tiny, too noisy. But if not, sometime, it is not accurate. Consider to match the line back to the image, if it doesn't match good, go for Rev 231, otherwise, leave it be.

C. Bugs found:
***. tiny bug found for "aaa_image_slice_1024_392_0456.tif", where the right side board has tiny unmatched boundary.
***. bug found when the global vars image_width and image_height are changed, the image saved is messed up.
***. bug found when running: "Project.exe -B slice_1024_255.tif -O "-r 20 -H""
(Rev 231) *. Found a problem for slice_1024_146.tif with radius 4. It cannot get the tiny boundary on both verticle sides, some parts of bottom side.
***. for image slice_1024_146.tif, if we copy the bottom center part to the left vertical, two boundary are found.
***. Found a problem for slice_107.tif where the radius is set to be 64 and doing refinement later on. When reduce the ball size, it cannot seperate a big boundary to smaller boundaries.
(Rev 226) *. Found a problem for slice_1024_225.tif where the radius is set to be 64
(Rev xxx) *. Found a bug where the file name was messed up. The reason is the length of the name buffer in Save2File is too small.(100 -> 1000)

D. Some implementation:
====. combine HT and ras2vec results: choose long HT lines, blacking all pixels inside these lines. If both 2 end points of the ras2vec lines falling inside 
      this black region, remove it.
====. the HT horizantol line is a little bit low (1 pixel error) than real data, change the my_HT() for loop i = 1 => i = 0;

Rev 288:
Generate dxf based on HT + BPA, command:
-R -O "-a <approach> -n <total_slices>"

Rev 275:
Updated Rev268, so that we can slice the data in 3 directions, X, Y, Z axis.

Rev 268:
Load the bpa data into images. command:
-l ..\data\Rotated.bpa -O "-s total_slices -x -X -y -Y -z -Z -d 0/1/2"

Rev 266:
1. Add system time to monitor the time spend on commands.
2. Add some threshold for big radius cases, which should not consider the turning round case.

Rev 262:
To detect memory issues: Run valgrind on Linux (ver1) with command: 
valgrind --tool=memcheck --leak-check=yes(full) --show-reachable=yes project -B slice_082.tif -O "-r 400"
Found two issues:
1. the free/delete/delete[] are not matching to each other.
2. (MAIN problem), a vector<int> type (data_points) is used for find(), should use its pointer, otherwise, the OS might not be able to
                   recycle the memory because of potentical efficiency machinism. After change it to pointer, use data_points = (*find(...)), and delete (&data_points)
                   

Rev 255:
HT + BPA as well as color matching.

Rev 233:
Check whether the refinement is needed or not for a long enough line segment based on real data. GOT very good results.
This improved Rev 231 and the requirement in B. so that the the lines are not too tiny or noisy and also accurate enough (slice_1024_146.tif)
The idea is to check how many points landing on the lines between 2 relative large seperated points, if < .5, try to refine it, otherwise let it be.
1. Regular good figure - slice_1024_225.tif
2. Tapered good figure - slice_1024_255.tif
3. Broken boundary     - slice_1024_146.tif
4. Seperated figure    - slice_107.tif
COMMAND: Project.exe -B slice_1024_255.tif -O "-r 32 -R -t 3"

Rev 231:
- add refine_counter : "-c <n>", debug information when debug the nth refinement
- enhance -h -e, so that they can be use independently.
- fix a bug in C, where the tiny boundary could not be found. The problem is cause by the refinement condition, i.e., x1 - x2 < 2 or y1 - y2 < 2, then do not refine, this is incorrect.

Rev 229:
Keep the color for different ball-size while doing refinement. That is, match the colors for the lines when doing refinement, namely if the line does not change, use the same color for different radius.
The idea is to use a extra field in POINTS_AUX - colors to store color information for each boundary points. when inserting new points, choose a color different from its two neighbors.

Rev 226:
Fixing the problem 1 of Rev223 by changing the pace_degree, and put it into gbl_opts. use "-p xx" to set the pace.
-p <int> pace degree
-R       do refinement
-t <int> repeat how many time for refinement
-r <int> radius size
-s <int> intervals between debug images
-h <int> head of the debug images (starting)
-e <int> end of the debug images (ending)

Rev 225:
Add a global structure variable gbl_opts for passing parameters between functions.

Rev 223:
Successful on adaptive BPA, i.e., multiple resolution on image slice_1024_225.tif file.
..\Project.exe -B slice_1024_225.tif -O "-r 32 -R -t 3"

Rev 208:
For image "step1_integrated_image_008.tif", the ball might be turn back incorrectly because of
a noise data at the bottom of the integrated image.

Rev 206:
Added sub option: -O "-d -s 1..."

