require 'sketchup'

# global settings
$x_min = -15.36;
$x_max =  80.23;
$y_min = -30.18;
$y_max =  89.93;
$z_min = -43.68;
$z_max =   5.69;
$z_scaler = 0.0

# function: draw_face_dir
# parameters:
#   dir: direction of the face - 0: y axis, 1: z axis, 2: x axis
#    fn: the file name containg the faces
#  im_w: the width of the image
#  im_h: the height of the image
# extrusion direction: 0 - y (bottom up), 1 - z (face inside), 2 - x (left right)
def draw_bpa_extruded_unit (layer, bottom_z, top_z, a, b, c, entities, needExt, dir)

            i = 0
	    pts = []
	    valid_num = 0
	    #print sprintf("EXTRUDED Layer %d: %f - %f, points: %d, c: %f - %f, needExt: %d\n", layer, bottom_z, top_z, a.size / 2, c[0], c[1], needExt)
            while i < a.size - 3 do
		# whe should extrude the duplicated points
		# dir: 0 - y (bottom up), 1 - z (face inside), 2 - x (left right)
		if ( dir == 0 ) # y axis
                	pts[valid_num] = Geom::Point3d.new($x_min + a[i]*$z_scaler, bottom_z, $z_min + a[i+1]*$z_scaler)
		elsif ( dir == 1 )  # z axis
               		pts[valid_num] = Geom::Point3d.new($x_min + a[i]*$z_scaler, $y_min + a[i+1]*$z_scaler, bottom_z)
		else  # x axis
               		pts[valid_num] = Geom::Point3d.new(bottom_z, $y_min + a[i+1]*$z_scaler, $z_min + a[i]*$z_scaler)
		end

                valid_num += 1

                i += 2
            end
            
            pts.uniq
             
            if (pts.size > 2 )
              face = entities.add_face pts
         
              if (!face)
                UI.messagebox sprintf("face created failed on layer %d.", layer)
              else 
              
                if ( true )
                    face.back_material = "white" # "lime" #
                    face.material = "white" # "lime" # 
		    face.material.alpha = 0.25
                end
		
                if ( needExt == 1 )
                	#normal = face.normal
                	dis = top_z - bottom_z
                	#if ( normal[2] < 0 )
                  	#	dis = -dis
                	#end
                	face.pushpull dis
                end

              end
            end
end

# function: draw_face_dir
# parameters:
#   dir: direction of the face - 0: y axis, 1: z axis, 2: x axis
#    fn: the file name containg the faces
#  im_w: the width of the image
#  im_h: the height of the image
def draw_face_dir ( dir, fn, im_w, im_h )

  model = Sketchup.active_model
  entities = model.entities

  if ( !File.exists?(fn)) 
     UI.messagebox sprintf("File %s is NOT existed!", fn)
  end

  t1 = Time.now.to_s

  if ( dir == 0 )  # y axis  
  	$z_scaler = ($x_max - $x_min)/im_w;
  	depth_bottom = $y_min ;
  	depth_top    = $y_max ;
  elsif ( dir == 1 ) # z axis
  	$z_scaler = ($x_max - $x_min)/im_w;
  	depth_bottom = $z_min ;
  	depth_top    = $z_max ;
  else
  	$z_scaler = ($z_max - $z_min)/im_w;
  	depth_bottom = $x_min ;
  	depth_top    = $x_max ;
  end

  depth_inter  = (depth_top - depth_bottom);  
  layer = 0
  a_indx = 0
  b_indx = 0
  c_indx = 0
  c_previous = -1.0
  a = []
  b = []
  c = []
  mode = 0
  total_line = 0
  unit_type = -1
  
  File.open(fn,"r").each { |line|

      line_str = line.clone
      total_line += 1
      if (line_str[0,3] == "BEG")
         mode = 0;
      elsif (line_str[0,9] == "EXTRUSION") 
          # copy old one here.
          b = a.clone
	  unit_type = 0;
      elsif (line_str[0,3] == "HEI")
         mode = 2;
      elsif (line_str[0,3] == "END")
         
         # construct the faces
        if ( c.size > 1)
          bottom_z = depth_bottom + c[0]*depth_inter;
          top_z    = depth_bottom + c[1]*depth_inter;
        end

        if ( a.size > 3 )
	  layer += 1
 	   c_previous = c[0]
 	   
 	   if ( unit_type == 0 )
 	   	needExt = 1
 	   else
 	   	needExt = 0
 	   end

           draw_bpa_extruded_unit(layer, bottom_z, top_z, a, b, c, entities, needExt, dir)
        end

        a_indx = 0
        b_indx = 0
        c_indx = 0
        a = []
        b = []
        c = []
	unit_type = -1;

         
        #UI.messagebox sprintf("found %d layer!\n", layer)
        #break if (layer > 10)
	
      else  #end of if "END"

        tmp_arr = line_str.split(' ');
        if (mode == 0 )
           a[a_indx] = tmp_arr[0].to_f()
           a_indx += 1
           a[a_indx] = im_h - tmp_arr[1].to_f()
           a_indx += 1
           
        elsif (mode == 1) 

           b[b_indx] = tmp_arr[0].to_f()
           b_indx += 1
           b[b_indx] = tmp_arr[1].to_f()
           b_indx += 1
           
        elsif (mode == 2)
           if ( a.size > 0) 
              c[c_indx] = tmp_arr[0].to_f()
              c_indx += 1
              c[c_indx] = tmp_arr[1].to_f()
              c_indx += 1  
           end
        end
        
      end

  }      # end of file open

  t2 = Time.now.to_s
  model.commit_operation

end # def draw_face_dir


# MAIN function: draw_bpa_face
# call each face reconstruction for each direction
# the interface of ruby
def draw_bpa_face

Sketchup.active_model.entities.clear!

# draw face based on direction, IR filename, 
#draw_bpa_face_left_right
fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\test_ir.txt"
draw_face_dir(0, fn, 1024, 512)

#draw_bpa_face_left_right
fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\test_ir_roof.txt"
draw_face_dir(0, fn, 1024, 512)

#draw_bpa_face_face_inside
fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\test_ir_face.txt"
draw_face_dir(2, fn, 480, 1024)

#draw_bpa_face_face_inside
fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\test_ir_face_roof.txt"
draw_face_dir(2, fn, 480, 1024)


end # draw_bpa_face



def draw_bpa_all_unit (layer, bottom_z, top_z, a, b, c, entities, dir, unit_type)

	if ( unit_type == 0 ) # for extruded structure
            i = 0
	    pts = []
	    valid_num = 0
	    print sprintf("EXTRUDED Layer %d: %f - %f, points: %d, c: %f - %f, dir: %d\n", layer, bottom_z, top_z, a.size / 2, c[0], c[1], dir)
            while i < a.size do
		if ( dir == 0 ) # y axis
                	pts[valid_num] = Geom::Point3d.new(a[i], bottom_z, a[i+1])
		elsif ( dir == 1 )  # z axis
               		pts[valid_num] = Geom::Point3d.new(a[i], a[i+1], bottom_z)
		else  # x axis
               		pts[valid_num] = Geom::Point3d.new(bottom_z, a[i+1], a[i])
		end
                valid_num += 1
                i += 2
            end
            
            pts.uniq
             
            if (pts.size > 2 )
              face = entities.add_face pts
         
              if (!face)
                UI.messagebox sprintf("face created failed on layer %d.", layer)
              else 
              
                if ( true )
                    face.back_material = "white" # "lime" #
                    face.material = "white" # "lime" # 
		    face.material.alpha =  0.75
                end
		
                if ( top_z != bottom_z )
                	#normal = face.normal
                	dis = top_z - bottom_z
                	face.pushpull dis
                end

              end
            end

	else  # for tapered structure

            i = 0
            i_gbl = 0
            pts_surface = []
            pts_surface[0] = Geom::Point3d.new(a[0], a[1], a[2])
	    print sprintf("TAPERED Layer %d: %f - %f, points: %d, c: %f - %f, dir: %d\n", layer, bottom_z, top_z, a.size / 2, c[0], c[1], dir)
            while i < a.size - 3 do
	    	pts = []
	    	valid_num = 0
                pts[valid_num] = Geom::Point3d.new(a[i  ], a[i+1], a[i+2])
	        valid_num += 1;
                pts[valid_num] = Geom::Point3d.new(a[i+3], a[i+4], a[i+5])
	        valid_num += 1;
                pts[valid_num] = Geom::Point3d.new(b[i+3], b[i+4], b[i+5])
	        valid_num += 1;
                pts[valid_num] = Geom::Point3d.new(b[i  ], b[i+1], b[i+2])
	        valid_num += 1;
                i += 3
                i_gbl += 1
                pts_surface[i_gbl] = Geom::Point3d.new(a[i  ], a[i+1], a[i+2])

	        if (pts.size > 2)
              	  face = entities.add_face pts
         
              	  if (!face)
                	UI.messagebox sprintf("face created failed on layer %d.", layer)
              	  else 
                    face.back_material = "white" # "lime" #
                    face.material = "white" # "lime" # 
		    face.material.alpha =  0.75
                  end
            	end

            end #end of while
            
            if ( pts_surface.size > 2 )
            	face = entities.add_face pts_surface
              	if (!face)
                	UI.messagebox sprintf("face created failed on layer %d.", layer)
              	else 
                    face.back_material = "white" # "lime" #
                    face.material = "white" # "lime" # 
		    face.material.alpha =  0.75
                end            	
            end

	end #end of else
end

# function: draw_bpa_all
def draw_bpa_all

  Sketchup.active_model.entities.clear!

  model = Sketchup.active_model
  entities = model.entities

  fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\IR_part_roof_only.txt"
#  fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\IR_update_all.txt"
  if ( !File.exists?(fn)) 
     UI.messagebox sprintf("File %s is NOT existed!", fn)
  end
  print sprintf("Load file %s\n", fn)

  t1 = Time.now.to_s

  layer = 0
  a_indx = 0
  b_indx = 0
  c_indx = 0
  c_previous = -1.0
  a = []
  b = []
  c = []
  mode = 0
  total_line = 0
  unit_type = -1
  dir = -1
  
  File.open(fn,"r").each { |line|

      line_str = line.clone
      total_line += 1
      if (line_str[0,3] == "BEG")
         mode = 0;
      elsif (line_str[0,3] == "DIR")
         mode = 0;
      elsif (line_str[0,3] == "TYP")
         mode = 4;
      elsif (line_str[0,3] == "POI")
         mode = 3;
      elsif (line_str[0,9] == "EXTRUSION") 
          # copy old one here.
          b = a.clone
	  unit_type = 0;
      elsif (line_str[0,3] == "HEI")
         mode = 2;
      elsif (line_str[0,3] == "TAP")
         mode = 1;
      elsif (line_str[0,3] == "END")
         
         # construct the faces
        if ( c.size > 1)
          bottom_z =  c[0]
          top_z    =  c[1]
        end

        if ( a.size > 3 )
	   layer += 1
           draw_bpa_all_unit(layer, bottom_z, top_z, a, b, c, entities, dir, unit_type)
        end

        a_indx = 0
        b_indx = 0
        c_indx = 0
        a = []
        b = []
        c = []
	unit_type = -1;

         
        #UI.messagebox sprintf("found %d layer!\n", layer)
        #break if (layer > 10)
	
      else  #end of if "END"

        tmp_arr = line_str.split(' ');
        if (mode == 3 )
           a[a_indx] = tmp_arr[0].to_f()
           a_indx += 1
           a[a_indx] = tmp_arr[1].to_f()
           a_indx += 1
	   if ( unit_type == 1 ) 
           	a[a_indx] = tmp_arr[2].to_f()
           	a_indx += 1
	   end
           
        elsif (mode == 1) 

           b[b_indx] = tmp_arr[0].to_f()
           b_indx += 1
           b[b_indx] = tmp_arr[1].to_f()
           b_indx += 1
           b[b_indx] = tmp_arr[2].to_f()
           b_indx += 1
           
        elsif (mode == 0) 

           dir = tmp_arr[0].to_i()
           
        elsif (mode == 4) 

           unit_type = tmp_arr[0].to_i()
           
        elsif (mode == 2)
           c[c_indx] = tmp_arr[0].to_f()
           c_indx += 1
           c[c_indx] = tmp_arr[1].to_f()
           c_indx += 1  
        end
        
      end

  }      # end of file open

  t2 = Time.now.to_s
  model.commit_operation

end # def draw_bpa_all


UI.menu("PlugIns").add_item("BPA Barefaces") { draw_bpa_all }
