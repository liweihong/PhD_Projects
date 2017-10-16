require 'sketchup'
require 'progress_bar.rb'

$gbl_wireframe = 0
$gbl_color = 0
def draw_tapered_unit(layer, bottom_z, top_z, a, b, c, colors,entities)
	    fhandle = File.open("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\log.txt", "a");
	    fhandle.puts(sprintf("Layer %d: bottom_z: %f, top_z:%f, points:%d", layer, bottom_z, top_z, a.size/2))
	    fhandle.close()

	    print sprintf("TAPER Layer %d: %f - %f, points: %d\n", layer, bottom_z, top_z, a.size / 2)
            i = 0
            while i < a.size - 3 do
                pts = []
                valid_num = 0
                pts[valid_num] = Geom::Point3d.new(a[i],a[i+1], bottom_z)
                valid_num += 1
                
                tmp_pt = Geom::Point3d.new(b[i],b[i+1], top_z)
                if ( !pts.include?(tmp_pt))  
                  pts[valid_num] = tmp_pt
                  valid_num += 1
                end
                  
                tmp_pt = Geom::Point3d.new(b[i+2],b[i+3], top_z)
                if ( !pts.include?(tmp_pt))  
                  pts[valid_num] = tmp_pt
                  valid_num += 1
                end
                tmp_pt = Geom::Point3d.new(a[i+2],a[i+3], bottom_z)
                if ( !pts.include?(tmp_pt))  
                  pts[valid_num] = tmp_pt
                  valid_num += 1
                end
                  
                if (valid_num > 2 )
                  face = entities.add_face pts
                  normal = face.normal
                  if ( normal[2] < 0 )
                     face = face.reverse!
                  end

                  if (face)
                  	if ( $gbl_color == 1)
                       face.back_material = colors[layer % (colors.size)]
                       face.material = colors[layer % (colors.size)]
                    else
                       face.back_material = "white" 
                       face.material = "white" 
                    end
		    if ( $gbl_wireframe == 1) 
		      face.material.alpha=0
		      face.back_material.alpha=0
		    end

                  end
                end
                
                i += 2
            end

	    # comment on the following code
	    if ( gbl_wireframe == 10 )

              i = 0
              while i < a.size - 1 do
                  pts1 = Geom::Point3d.new(a[i],a[i+1], bottom_z)
                  pts2 = Geom::Point3d.new(b[i],b[i+1], top_z)
	  	  if (pts1 != pts2)
	            edge = entities.add_line(pts1, pts2)
		    faces = edge.faces
		    if (faces)	
	              m = Sketchup.active_model.materials.add "Wei"
	              m.alpha=0
        	      j = 0
        	      while ( j < faces.size )
           	  	  faces[j].material = m
           		  j += 1
		      end
    		    end
		  end 
		
		  i += 2
              end

	    end  # end of if $gbl_wireframe

end

def draw_extruded_unit (layer, bottom_z, top_z, a, b, c, colors,entities)
	    fhandle = File.open("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\log.txt", "a");
	    fhandle.puts(sprintf("Layer %d: bottom_z: %f, top_z:%f, points:%d, c: %f - %f", layer, bottom_z, top_z, a.size/2, c[0], c[1]))
	    fhandle.close()

            i = 0
	    pts = []
	    valid_num = 0
	    print sprintf("EXTRUDED Layer %d: %f - %f, points: %d, c: %f - %f\n", layer, bottom_z, top_z, a.size / 2, c[0], c[1])
            while i < a.size - 3 do
		# whe should extrude the duplicated points
                pts[valid_num] = Geom::Point3d.new(a[i],a[i+1], bottom_z)
                valid_num += 1
                
                #tmp_pt = Geom::Point3d.new(b[i],b[i+1], top_z)
                #if ( !pts.include?(tmp_pt))  
                #  pts[valid_num] = tmp_pt
                #  valid_num += 1
                #end
                  
                i += 2
            end
            
            pts.uniq
             
            if (pts.size > 2 )
              face = entities.add_face pts
         
              if (!face)
                UI.messagebox sprintf("face created failed on layer %d.", layer)
              else 
              
                if ( $gbl_color == 1)
                    face.back_material = colors[layer % (colors.size)]
                    face.material = colors[layer % (colors.size)]
                else
                    face.back_material = "white" 
                    face.material = "white" 
                end
		
		if ( $gbl_wireframe == 1) 
		  face.material.alpha=0
		  face.back_material.alpha=0
		end

                normal = face.normal
                dis = top_z - bottom_z
                if ( normal[2] < 0 )
                  dis = -dis
                end
                face.pushpull dis
                
              end
            end
            
            #UI.messagebox sprintf("added %d layer!\n", layer)

	    # comment on the following code
	  if ( $gbl_wireframe == 10 )

            i = 0
            while i < a.size - 1 do
                pts1 = Geom::Point3d.new(a[i],a[i+1], bottom_z)
                pts2 = Geom::Point3d.new(b[i],b[i+1], top_z)
		if (pts1 != pts2)
	          edge = entities.add_line(pts1, pts2)
		  faces = edge.faces
		  if (faces)	
	            m = Sketchup.active_model.materials.add "Wei"
	            m.alpha=0
        	    j = 0
        	    while ( j < faces.size )
           		faces[j].material = m
           		j += 1
		    end
    		  end
		end 
		
		i += 2
            end

	  end   #if $gbl_wireframe == 1

end


def draw_building_face_name(fn)

#COMM
if true
  model = Sketchup.active_model
  entities = model.entities
  entities.clear!
  model.start_operation "Building"

  colors = ["AliceBlue", "AntiqueWhite", "BlueViolet", "Cyan", "Fuchsia", "Gold", "IndianRed", "LightYellow", "Olive"]
end


#  fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\ir.txt"
  if ( !File.exists?(fn)) 
     UI.messagebox sprintf("File %s is NOT existed!", fn)
  end

  t1 = Time.now.to_s
  
  #define Z axis
  #z_scaler = 15.604761;
  #depth_bottom = -2.116044 * z_scaler;
  #depth_top    = 30.216660 * z_scaler;
  z_scaler = 14.6285714;  # z_scaler = 1024/(BOX_MAX_X - BOX_MIN_X) = 1024/70 = 14.6285714
  depth_bottom = 0 * z_scaler;
  depth_top    = 44 * z_scaler;
  depth_inter  = (depth_top - depth_bottom);  
  layer = 0
  a_indx = 0
  b_indx = 0
  c_indx = 0
  a = []
  b = []
  c = []
  mode = 0
  total_line = 0
  unit_type = 0
  File.open(fn,"r").each { |line|

      line_str = line.clone
      total_line += 1
      if (line_str[0,3] == "BEG")
         mode = 0;
      elsif (line_str[0,3] == "TAP")
         mode = 1;
	 unit_type = 1;
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
	  top_z   -= 0.05
          print sprintf("Layer %d: %f - %f, points: %d\n", layer, bottom_z, top_z, a.size / 2)
        end
        
	#if( layer > 124 && layer < 126)
	
        if ( a.size > 3 && unit_type == 1)
           draw_tapered_unit(layer, bottom_z, top_z, a, b, c, colors,entities)
        end

        if ( a.size > 3 && unit_type == 0) # && c[0] > 0.4 && c[0] < 0.529)
           draw_extruded_unit(layer, bottom_z, top_z, a, b, c, colors,entities)
        end

	#aaa = UI.messagebox sprintf("layer %d\n", layer), MB_OKCANCEL
	#if (aaa == 2)
	#  return;
        #end
	
	#end   # end of if [124, 126] for debug

        a_indx = 0
        b_indx = 0
        c_indx = 0
        a = []
        b = []
        c = []
         

        layer = layer + 1
        #UI.messagebox sprintf("found %d layer!\n", layer)
        #break if (layer > 10)
	
      else  #end of if "END"

        tmp_arr = line_str.split(' ');
        if (mode == 0 )
           a[a_indx] = tmp_arr[0].to_f
           a_indx += 1
           a[a_indx] = tmp_arr[1].to_f()
           a_indx +=  1
           
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

#COMM
if true  
  #UI.messagebox sprintf("Start at: [%s], End at: [%s], total line is %d", t1, t2, total_line)
  model.commit_operation
end  
  
# save this model in a file
#status = model.save "C:/Program Files/Google/Google SketchUp 6/Plugins/models/IR_skp_test_auto.skp"
#if (status)
#	UI.messagebox "Model saved sucessfully!"
#else 
#	UI.messagebox "Model saved failed!"
#end

end # def draw_building_face_name

def draw_building_face

if true
 fn = "C:/Program Files/Google/Google SketchUp 6/Plugins/ir_1000_4_4.txt"
 draw_building_face_name(fn)

else

$gbl_wireframe = 0
$gbl_color = 1
i = 4
while i <= 128 do
# fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\ir_1000_%d_4.txt", i)
 fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\ir_1000_4_%d.txt", i)
# save_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/models/IR_skp_color_1000_%d_4.skp", i)
 save_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/models/IR_skp_color_1000_4_%d.skp", i)
 draw_building_face_name(fn)
 Sketchup.active_model.save save_fn
 Sketchup.active_model.entities.clear!

#  i = i + 4
  i = i * 2
end

$gbl_wireframe = 1
$gbl_color = 0
i = 1
while i <= 128 do
# fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\ir_1000_%d_4.txt", i)
 fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\ir_1000_4_%d.txt", i)
# save_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/models/IR_skp_color_1000_%d_4.skp", i)
 save_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/models/IR_skp_wireframe_1000_4_%d.skp", i)
 draw_building_face_name(fn)
 Sketchup.active_model.save save_fn
 Sketchup.active_model.entities.clear!

#  i = i + 4
  i = i * 2
end

end # of if, like #ifdef

UI.messagebox "Model saved sucessfully!"

end # def draw_building_face

UI.menu("PlugIns").add_item("Building Barefaces") { draw_building_face }
