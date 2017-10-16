require 'sketchup'
require 'building_face.rb'

if false
#  fn = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\error_comp_result.txt"
  fn = load_fn
  if ( !File.exists?(fn)) 
     UI.messagebox sprintf("File %s is NOT existed!", fn)
  end

  #start_test = 0
  File.open(fn,"r").each { |line|

      line_str = line.clone
      tmp_arr = line_str.split(' ');
      if (line_str[0,3] == "Lay")
         layer = tmp_arr[1].to_i
         #if (layer > 238 && layer < 311 )
	 #   start_test = 1
         #else
         #   start_test = 0
         #end
	 fhandle = File.open("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\log.txt", "a");
	 fhandle.puts(sprintf("Processing Error Layer %d at time %s... ", layer, Time.now.to_s ))
	 fhandle.close()
  #    elsif ( start_test == 1)
      else
         x1 = tmp_arr[0].to_i
         y1 = tmp_arr[1].to_i
         x2 = tmp_arr[2].to_i
         y2 = tmp_arr[3].to_i
         er = tmp_arr[4].to_f

	 if ( er > 0.01 )	 
	   bottom_z = depth_bottom + (Float(layer)/Float(sample_num))*depth_inter;
	   top_z    = depth_bottom + (Float(layer + 1)/Float(sample_num))*depth_inter;
           colorScale = er / error_max # Ave: 1.512, Max: 175.667, total:104405
           if ( colorScale > 1.0)
	     colorScale = 1.0
	   end
	   colorI = Integer(255 - 255*colorScale)
	  
           pts = [[x1, y1, bottom_z], [x2, y2, bottom_z], [x2, y2, top_z], [x1, y1, top_z]]
	   face = entities.add_face pts
	   if (face)
	        face.material = [colorI, colorI, colorI]
		edge1 = entities.add_line pts[2], pts[3]
		if (edge1)
			edge1.soft = true
		end
		edge1 = entities.add_line pts[0], pts[1]
		if (edge1)
			edge1.soft = true
		end
	   end
	 end
      end
  }      # end of file open
end

def draw_building_error_and_save(load_fn, save_fn, face_fn, prefix)

  model = Sketchup.active_model
  entities = model.entities
  entities.clear!
  model.start_operation "Building"

####################################################
  ## TEST ##

  if false

    pts = []
    pts[0] = [0,0,0]
    pts[1] = [100,0,0]
    pts[2] = [100,100,0]
    pts[3] = [0,100,0]
    face = entities.add_face pts
    face.pushpull -100

    # The following will not increase the # of entities BECAUSE they are existed.
    edge1 = entities.add_line [0,0,100], [100,0,100]
    edge2 = entities.add_line [100,0,100], [100,100,100]
    
    face = edge1.common_face edge2
    if ( face )
      materials = model.materials
      # Adds a material to the "in-use" material pallet.
      m = materials.add "Joe"
      m.texture="D:\\Liwh\\Dev\\Matlab\\PolyFitting\\src\\hunter_0.JPG"
      #m.texture="C:\\Liwh\\Dev\\Matlab\\PolyFitting\\src\\Hunter.JPG"
      t=m.texture
      t.size=75
      face.material = m
      #UI.messagebox "Texture added for common face!"
     
    end

return;

    # try sth new
    h = 10;    w = 10;    colorScale = Float(h * w);
    h_s = 100/h; w_s = 100/w;
    for i in 0..h-1
	for j in 0..w-1
		pts = [[j*w_s, 0, i*h_s], [(j+1)*w_s, 0, i*h_s],[(j+1)*w_s, 0, (i+1)*h_s],[j*w_s,0,(i+1)*h_s]]
		face = entities.add_face pts
		if (face)
		  col = Integer(Float(i*j)/colorScale * 255)
		  face.material = [col, col, col]
		end

		edge1 = entities.add_line pts[0], pts[1]
		if (edge1)
			edge1.soft = true
		end
		edge1 = entities.add_line pts[0], pts[3]
		if (edge1)
			edge1.soft = true
		end
	end
    end
#    pts = [[0,0,0],[100,0,0],[100,0,50],[0,0,50]]
#    face = entities.add_face pts
#    if (face)
#      face.material = [122, 122, 122]
#    end

  end

#######################################################
## FUNCTION BODY ##

  t1 = Time.now.to_s

  #draw_building_face_name(face_fn)

  #define Z axis, Sample # for configuration.
  z_scaler = 14.6285714;  # z_scaler = 1024/(BOX_MAX_X - BOX_MIN_X) = 1024/70 = 14.6285714
  depth_bottom = 0 * z_scaler;
  depth_top    = 44 * z_scaler;
  #sample_num   = 200;
  sample_num   = 1000;
  error_max    = 15.0;  # maximum error bound

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
  total_face = 0

  File.open(face_fn,"r").each { |line|

      line_str = line.clone
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
        
	# draw the layer face
	layer_pts = [[a[0], a[1], bottom_z]];	
	layer_cnt = 1
	
        if ( a.size > 3 && unit_type == 0 ) # && c[0] < 0.128)

	  i = 0

            while i < a.size - 3 do
                 
		x1 = a[i]
		y1 = a[i+1]
		x2 = a[i+2]
		y2 = a[i+3] 
	
		layer_pts[layer_cnt] = [x2, y2, bottom_z];
		layer_cnt += 1
		

		#if (  (x2 - x1) < 10 && (x2 - x1) > -10)
		#	i += 2
		#	next
		#end
		#if ( x1 == 213 && y1 == 343 && x2 == 186 && y2 == 343)

           	#pts = [[x1, y1, bottom_z], [x2, y2, bottom_z], [x2, y2, top_z], [x1, y1, top_z]]
           	#pts = [[x1, y1, top_z], [x2, y2, top_z], [x2, y2, bottom_z], [x1, y1, bottom_z]]
           	pts = [[x1, y1, top_z], [x1, y1, bottom_z], [x2, y2, bottom_z], [x2, y2, top_z]]
	   	face = entities.add_face pts
	   	if (face)
			ti_fn = sprintf("%s/texture_%.3f_%.3f_%d-%d_%d-%d_image.tif", load_fn, c[0], c[1], x1, y1, x2, y2 )
  			if ( File.exists?(ti_fn)) 
			 	fhandle = File.open("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\log.txt", "a");
		 		fhandle.puts(sprintf("Texture mapping %s at time %s... ", ti_fn, Time.now.to_s ))
	 			fhandle.close()
				total_face += 1
				m_name = sprintf("weihong_%d_%s", total_face, prefix);
      				m = model.materials.add m_name
      				m.texture=ti_fn
				m.texture.size=m.texture.image_width
				face.position_material m, [[x1, y1, top_z], [0,0]], true
				face.position_material m, [[x1, y1, top_z], [0,0]], false
			else 
				face.back_material = "lime"
				face.material = "lime"
  			end
	   	end
		
		#end

                i += 2

            end

        end  # end of if

	if (layer_cnt > 3)
		face = entities.add_face layer_pts
		if ( face ) 
                       face.back_material = "lime" #"white" 
                       face.material = "lime" #"white" 
		end	
	end

        if ( a.size > 3 && unit_type == 1 )
	    colors = []
	    draw_tapered_unit(layer, bottom_z, top_z, a, b, c, colors,entities)	
	end


        a_indx = 0
        b_indx = 0
        c_indx = 0
        a = []
        b = []
        c = []

        layer = layer + 1
	
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

  # save this model in a file
  # model.save save_fn

  #UI.messagebox sprintf("Start at: [%s], End at: [%s]", t1, t2)
  model.commit_operation
  
end # def draw_building_error_and_save

def draw_building_error

if false  #switch here.

i = 32
while i >= 16 do

   load_fn = sprintf("d:/Liwh/Dev/Matlab/PolyFitting/src/result/error_comp/texture_1000_%d_4", i);
   face_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/ir_1000_%d_4.txt", i)
   save_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/models/IR_skp_error_face_1000_%d_4.skp", i)

   prefix = sprintf("1000_%d_4", i);
   draw_building_error_and_save(load_fn, save_fn, face_fn, prefix)

   # clear the model
   Sketchup.active_model.save save_fn
   Sketchup.file_new

   i = i / 2
end

else

i = 4
while i >= 4 do

   load_fn = sprintf("d:/Liwh/Dev/Matlab/PolyFitting/src/result/error_comp/texture_1000_4_%d", i);
   face_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/ir_1000_4_%d.txt", i)
   save_fn = sprintf("C:/Program Files/Google/Google SketchUp 6/Plugins/models/IR_skp_error_face_1000_4_%d.skp", i)

   prefix = sprintf("1000_4_%d", i);
   draw_building_error_and_save(load_fn, save_fn, face_fn, prefix)

   # clear the model
   Sketchup.active_model.save save_fn
   Sketchup.file_new

   i = i / 2
end

end

end #

UI.menu("PlugIns").add_item("Building Error") { draw_building_error }
