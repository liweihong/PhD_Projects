require 'sketchup'

$gbl_color = 0
def draw_face_segments (layer, bottom_z, top_z, a, b, c, entities, dir, unit_type, colors)

	if ( unit_type == 0 || unit_type == 2) # for extruded structure
            i = 0
	    pts = []
	    valid_num = 0
	    #UI.messagebox sprintf("EXTRUDED Layer %d: %f - %f, points: %d, c: %f - %f, unit_type: %d\n", layer, bottom_z, top_z, a.size / 3, c[0], c[1], unit_type)
	    print sprintf("EXTRUDED Layer %d: %f - %f, points: %d, c: %f - %f, unit_type: %d\n", layer, bottom_z, top_z, a.size / 3, c[0], c[1], unit_type)
            while i < a.size do
            	pts[valid_num] = Geom::Point3d.new(a[i], a[i+1], a[i+2])
            	i += 3
                valid_num += 1
                
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
                    face.back_material = "white" # "lime" #
                    face.material = "white" # "lime" # 
		    face.material.alpha =  1.0 # 0.75
                end
		
                if ( top_z != bottom_z )
                	normal = face.normal
                	dis = top_z - bottom_z
                	
                	if ( normal.z < 0 )
                	   dis = -dis
                	end
                	
                	#UI.messagebox sprintf("normal: [%f, %f, %f].", normal.x, normal.y,normal.z)
                	
                	#face.pushpull dis
                end

              end
            end

	else  # for tapered structure

            i = 0
            i_gbl = 0
	    print sprintf("TAPERED Layer %d: %f - %f, points: %d, c: %f - %f, dir: %d\n", layer, bottom_z, top_z, a.size / 2, c[0], c[1], dir)
	    data_len = a.size
            while i < a.size - 3 do
	    	pts = []
	    	valid_num = 0
                pts[valid_num] = Geom::Point3d.new(a[i%data_len], a[(i+1)%data_len], a[(i+2)%data_len])
	        valid_num += 1;
                maybe_dup_pont = Geom::Point3d.new(a[(i+3)%data_len], a[(i+4)%data_len], a[(i+5)%data_len])
                if ( maybe_dup_pont != pts[valid_num - 1] )
                   pts[valid_num] = maybe_dup_pont
                   valid_num += 1;
                end	        
                pts[valid_num] = Geom::Point3d.new(b[(i+3)%data_len], b[(i+4)%data_len], b[(i+5)%data_len])
	        valid_num += 1;
                maybe_dup_pont = Geom::Point3d.new(b[i%data_len], b[(i+1)%data_len], b[(i+2)%data_len])
                if ( maybe_dup_pont != pts[valid_num - 1] )
                   pts[valid_num] = maybe_dup_pont
                end
	        
                i += 3

		         
		print sprintf("TAPERED Line %d\n", i/3)
	        if (pts.size > 2)
              	  face = entities.add_face pts
         
              	  if (!face)
                	UI.messagebox sprintf("face created failed on layer %d.", layer)
              	  else 
                	if ( $gbl_color == 1)
                    	face.back_material = colors[layer % (colors.size)]
                    	face.material = colors[layer % (colors.size)]
                	else              	  
                    	face.back_material = "white" # "lime" #
                    	face.material = "white" # "lime" # 
		    	face.material.alpha =  1.0 # 0.75
		    	end
                  end
            	end

            end #end of while
            
	end #end of else
end

# function: draw_bpa_all
def draw_model_from_IR

  Sketchup.active_model.entities.clear!

  model = Sketchup.active_model
  entities = model.entities
  colors = ["AliceBlue", "AntiqueWhite", "BlueViolet", "Cyan", "Fuchsia", "Gold", "BlueViolet", "IndianRed", "LightYellow", "Olive"]
#  colors = ["AliceBlue", "AntiqueWhite", "BlueViolet", "Cyan", "Fuchsia", "Gold", "IndianRed", "LightYellow", "Olive"]
#  colors = ["AliceBlue", "AntiqueWhite", "BlueViolet", "Cyan", "Fuchsia", "Gold", "BlueViolet"]
  $gbl_color = 0

#  fn = "D:\\Liwh\\Dev\\Matlab\\PolyFitting\\src\\result\\for_paper\\synthetic\\1528\\IR_taper_all.txt"
  fn = "D:\\Liwh\\Dev\\Matlab\\PolyFitting\\src\\Main\\Paper\\Dissertation\\Tapers\\ttl2\\IR_taper_all_Y_axis.txt"
  if ( !File.exists?(fn)) 
     UI.messagebox sprintf("File %s is NOT existed!", fn)
  end
  print sprintf("Load file %s\n", fn)

  t1 = Time.now.to_s

  layer = 0 #change the color
  a_indx = 0
  b_indx = 0
  c_indx = 0
  c_previous = -1.0
  a = []
  b = []
  c = []
  mode = 0
  total_line = 0
  unit_type = 0
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
	   if ( layer % 3 == 0 )
           draw_face_segments(layer, bottom_z, top_z, a, b, c, entities, dir, unit_type, colors)
           end
           layer += 1
        end

        a_indx = 0
        b_indx = 0
        c_indx = 0
        a = []
        b = []
        c = []
	unit_type = 0;

         
        #UI.messagebox sprintf("found %d layer!\n", layer)
        #break if (layer > 10)
	
      else  #end of if "END"

        tmp_arr = line_str.split(' ');
        if (mode == 3 )
           a[a_indx] = tmp_arr[0].to_f()
           a_indx += 1
           a[a_indx] = tmp_arr[1].to_f()
           a_indx += 1
           a[a_indx] = tmp_arr[2].to_f()
           a_indx += 1
           
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

	   # the last one
           draw_face_segments(layer, bottom_z, top_z, a, b, c, entities, dir, unit_type, colors)
           
           
  t2 = Time.now.to_s
  model.commit_operation

end # def draw_bpa_all


UI.menu("PlugIns").add_item("DISPLAY MODEL") { draw_model_from_IR }

#p1=Geom::Point3d.new(72.725609, -40.000000, 4.947105)
#p2=Geom::Point3d.new(72.725609, 100.000000, 4.947105)
#my_edge=Sketchup.active_model.entities.add_line p1, p2
#Sketchup.active_model.selection.toggle my_edge
#Sketchup.active_model.selection[0].normal