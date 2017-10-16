require 'sketchup'
require 'progress_bar.rb'

def draw_unit (layer, bottom_z, top_z, a, b, entities)
	    fhandle = File.open("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\log.txt", "a");
	    fhandle.puts(sprintf("Layer %d: bottom_z: %f, top_z:%f, points:%d", layer, bottom_z, top_z, a.size/2))
	    fhandle.close()

            i = 0
            while i < a.size - 1 do
                pts1 = Geom::Point3d.new(a[i],a[i+1], bottom_z)
                pts2 = Geom::Point3d.new(b[i],b[i+1], top_z)
		#if (pts1 != pts2)
	          #entities.add_line(pts1, pts2)
		#end 
		if i == 0
		   i += 2
		   next
		end
		
                pts3 = Geom::Point3d.new(a[i-2],a[i-1], bottom_z)
		if (pts1 != pts3 )
                  pts4 = Geom::Point3d.new(b[i-2],b[i-1], top_z)
	          #entities.add_line(pts1, pts3)
	          #entities.add_line(pts4, pts2)
		  face = entities.add_face (pts1, pts3, pts4, pts2)
		  if ( face ) 
                       face.back_material = "white" # "lime" # 
                       face.material = "white" # "lime" # 
		  end
		end
		i += 2
            end
            
end

def draw_building_wireframe

  model = Sketchup.active_model
  entities = model.entities
  entities.clear!
  model.start_operation "Building"

  fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\ir.txt"
  if ( !File.exists?(fn)) 
     UI.messagebox sprintf("File %s is NOT existed!", fn)
  end

  t1 = Time.now.to_s

  #define Z axis
#  z_scaler = 14.6285714;  # z_scaler = 1024/(BOX_MAX_X - BOX_MIN_X) = 1024/70 = 14.6285714
#  depth_bottom = 0 * z_scaler;
#  depth_top    = 44 * z_scaler;

  im_h = 800
  im_w = 640
  box_min_x = -9.17
  box_max_x = 13.06
  box_min_z = -1.04
  box_max_z = 10.50
  
  z_scaler = im_w/(box_max_x - box_min_x)
  depth_bottom = box_min_z * z_scaler;
  depth_top    = box_max_z * z_scaler;

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
        end
        
        if ( a.size > 3 )
           draw_unit(layer, bottom_z, top_z, a, b, entities)
        end

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
        if (mode == 0)
           a[a_indx] = tmp_arr[0].to_f
           a_indx += 1
           a[a_indx] = im_h - tmp_arr[1].to_f()
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
  UI.messagebox sprintf("Start at: [%s], End at: [%s], total line is %d", t1, t2, total_line)
  model.commit_operation
end  
  
end # def draw_building
UI.menu("PlugIns").add_item("Building Wireframe") { draw_building_wireframe }
