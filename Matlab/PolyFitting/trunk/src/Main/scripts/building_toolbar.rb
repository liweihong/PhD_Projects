require 'sketchup'
require 'progress_bar.rb'

$gbl_b_index = 1
$gbl_p_index = 4
$gbl_b_or_p  = 0
$gbl_prefix  = "face"
$instance = nil
def boundaryAdj ( val, model, entities)

# from 1 to 128

   $gbl_b_or_p  = 0

   ratio = 0.5
   if ( val > 0)
	ratio = 2
   end

   $gbl_b_index *= ratio
  
   if ( $gbl_b_index < 1 )
	$gbl_b_index = 128
   end
  
   if ( $gbl_b_index > 128)
	$gbl_b_index = 1
   end

   entities.clear!

   fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_4_%d.skp", $gbl_prefix, $gbl_b_index);
   Sketchup.set_status_text fn
   pants_def = Sketchup.active_model.definitions.load(fn)
   #Sketchup.active_model.place_component(pants_def)
   pants_location = Geom::Point3d.new 0,0,0
   transform = Geom::Transformation.new pants_location
   $instance = entities.add_instance pants_def, transform
   #The axis is defined by a point and a vector. The angle is given in radians. 
   #$instance.transformation= Geom::Transformation.scaling/translation 1
   #$instance.transformation= Geom::Transformation.rotation [512, 400, 0], [0, 0, 1], 3.1415*0.7
   $instance.transform! Geom::Transformation.rotation [512, 200, 0], [0, 0, 1], 3.1415
   $instance.transform! Geom::Transformation.translation [-300, 0, 0]
   $instance.transform! Geom::Transformation.scaling 0.7
   #model.import fn[$gbl_index], true
   #dd = model.definitions
	
end

def projectAdj ( val, model, entities)

# from 4 to 32

   $gbl_b_or_p  = 1
   ratio = -4
   if ( val > 0)
	ratio = 4
   end

   $gbl_p_index += ratio

   if ( $gbl_p_index < 4 )
	$gbl_p_index = 32
   end
  
   if ( $gbl_p_index > 32)
	$gbl_p_index = 4
   end

   entities.clear!

   fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_%d_4.skp", $gbl_prefix, $gbl_p_index);
   Sketchup.set_status_text fn
   pants_def = Sketchup.active_model.definitions.load(fn)
   #Sketchup.active_model.place_component(pants_def)
   pants_location = Geom::Point3d.new 0,0,0
   transform = Geom::Transformation.new pants_location
   $instance = entities.add_instance pants_def, transform
   #The axis is defined by a point and a vector. The angle is given in radians. 
   #$instance.transformation= Geom::Transformation.scaling/translation 1
   #$instance.transformation= Geom::Transformation.rotation [512, 400, 0], [0, 0, 1], 3.1415*0.7
   $instance.transform! Geom::Transformation.rotation [512, 200, 0], [0, 0, 1], 3.1415
   $instance.transform! Geom::Transformation.translation [-300, 0, 0]
   $instance.transform! Geom::Transformation.scaling 0.7
   #model.import fn[$gbl_index], true
   #dd = model.definitions
	
end

def errorAdj ( val, model, entities)
   entities.clear!

   $gbl_prefix  = "error_face"

   fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_4_%d.skp", $gbl_prefix, $gbl_b_index);
   if ( $gbl_b_or_p > 0 )
	fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_%d_4.skp", $gbl_prefix, $gbl_p_index);
   end

   Sketchup.set_status_text fn
   pants_def = Sketchup.active_model.definitions.load(fn)
   #Sketchup.active_model.place_component(pants_def)
   pants_location = Geom::Point3d.new 0,0,0
   transform = Geom::Transformation.new pants_location
   $instance = entities.add_instance pants_def, transform
   #The axis is defined by a point and a vector. The angle is given in radians. 
   #$instance.transformation= Geom::Transformation.scaling/translation 1
   #$instance.transformation= Geom::Transformation.rotation [512, 400, 0], [0, 0, 1], 3.1415*0.7
   $instance.transform! Geom::Transformation.rotation [512, 200, 0], [0, 0, 1], 3.1415
   $instance.transform! Geom::Transformation.translation [-300, 0, 0]
   $instance.transform! Geom::Transformation.scaling 0.7
   #model.import fn[$gbl_index], true
   #dd = model.definitions
end

def wfAdj ( val, model, entities)
   entities.clear!

   $gbl_prefix  = "wireframe"

   fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_4_%d.skp", $gbl_prefix, $gbl_b_index);
   if ( $gbl_b_or_p > 0 )
	fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_%d_4.skp", $gbl_prefix, $gbl_p_index);
   end

   Sketchup.set_status_text fn
   pants_def = Sketchup.active_model.definitions.load(fn)
   #Sketchup.active_model.place_component(pants_def)
   pants_location = Geom::Point3d.new 0,0,0
   transform = Geom::Transformation.new pants_location
   $instance = entities.add_instance pants_def, transform
   #The axis is defined by a point and a vector. The angle is given in radians. 
   #$instance.transformation= Geom::Transformation.scaling/translation 1
   #$instance.transformation= Geom::Transformation.rotation [512, 400, 0], [0, 0, 1], 3.1415*0.7
   $instance.transform! Geom::Transformation.rotation [512, 200, 0], [0, 0, 1], 3.1415
   $instance.transform! Geom::Transformation.translation [-300, 0, 0]
   $instance.transform! Geom::Transformation.scaling 0.7
   #model.import fn[$gbl_index], true
   #dd = model.definitions
end

def colorAdj ( val, model, entities)
   entities.clear!

   $gbl_prefix  = "color"

   fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_4_%d.skp", $gbl_prefix, $gbl_b_index);
   if ( $gbl_b_or_p > 0 )
	fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_%d_4.skp", $gbl_prefix, $gbl_p_index);
   end

   Sketchup.set_status_text fn
   pants_def = Sketchup.active_model.definitions.load(fn)
   #Sketchup.active_model.place_component(pants_def)
   pants_location = Geom::Point3d.new 0,0,0
   transform = Geom::Transformation.new pants_location
   $instance = entities.add_instance pants_def, transform
   #The axis is defined by a point and a vector. The angle is given in radians. 
   #$instance.transformation= Geom::Transformation.scaling/translation 1
   #$instance.transformation= Geom::Transformation.rotation [512, 400, 0], [0, 0, 1], 3.1415*0.7
   $instance.transform! Geom::Transformation.rotation [512, 200, 0], [0, 0, 1], 3.1415
   $instance.transform! Geom::Transformation.translation [-300, 0, 0]
   $instance.transform! Geom::Transformation.scaling 0.7
   #model.import fn[$gbl_index], true
   #dd = model.definitions
end

def faceAdj ( val, model, entities)
   entities.clear!

   $gbl_prefix  = "face"

   fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_4_%d.skp", $gbl_prefix, $gbl_b_index);
   if ( $gbl_b_or_p > 0 )
	fn = sprintf("C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\models\\IR_skp_%s_1000_%d_4.skp", $gbl_prefix, $gbl_p_index);
   end

   Sketchup.set_status_text fn
   pants_def = Sketchup.active_model.definitions.load(fn)
   #Sketchup.active_model.place_component(pants_def)
   pants_location = Geom::Point3d.new 0,0,0
   transform = Geom::Transformation.new pants_location
   $instance = entities.add_instance pants_def, transform
   #The axis is defined by a point and a vector. The angle is given in radians. 
   #$instance.transformation= Geom::Transformation.scaling/translation 1
   #$instance.transformation= Geom::Transformation.rotation [512, 400, 0], [0, 0, 1], 3.1415*0.7
   $instance.transform! Geom::Transformation.rotation [512, 200, 0], [0, 0, 1], 3.1415
   $instance.transform! Geom::Transformation.translation [-300, 0, 0]
   $instance.transform! Geom::Transformation.scaling 0.7
   #model.import fn[$gbl_index], true
   #dd = model.definitions
end

def draw_building_toolbar

#COMM
if true
  model = Sketchup.active_model
  entities = model.entities
  entities.clear!
  model.start_operation "Building"

  colors = ["AliceBlue", "AntiqueWhite", "BlueViolet", "Cyan", "Fuchsia", "Gold", "IndianRed", "LightYellow", "Olive"]
end

# File lib/test/unit/ui/fox/testrunner.rb, line 211
# FXProgressBar.new(model, nil, 0, PROGRESSBAR_NORMAL | LAYOUT_FILL_X)
#pb = ProgressBar.new(100,"weihong") 


toolbar = UI::Toolbar.new "Test"

# This toolbar tool simply displays Hello World on the screen when clicked
cmd = UI::Command.new($tStrings.GetString("Test")) { boundaryAdj(-1, model, entities)  } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\b_plus.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\b_plus.PNG"
cmd.tooltip = $tStrings.GetString("Zoom in boundary...")
cmd.status_bar_text = $tStrings.GetString("Zoom in the building boundary...")
cmd.menu_text = $tStrings.GetString("Boundary")
toolbar = toolbar.add_item cmd

cmd = UI::Command.new($tStrings.GetString("Test")) { boundaryAdj(1, model, entities)  } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\b_minus.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\b_minus.PNG"
cmd.tooltip = $tStrings.GetString("Zoom out boundary...")
cmd.status_bar_text = $tStrings.GetString("Zoom out the building boundary...")
cmd.menu_text = $tStrings.GetString("Boundary")
toolbar = toolbar.add_item cmd
toolbar = toolbar.add_separator

cmd = UI::Command.new($tStrings.GetString("Test")) { projectAdj(-1, model, entities)  } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\l_plus.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\l_plus.PNG"
cmd.tooltip = $tStrings.GetString("Zoom in projection...")
cmd.status_bar_text = $tStrings.GetString("Zoom in the building projection...")
cmd.menu_text = $tStrings.GetString("Projection")
toolbar = toolbar.add_item cmd

cmd = UI::Command.new($tStrings.GetString("Test")) { projectAdj(1, model, entities)  } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\l_minus.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\l_minus.PNG"
cmd.tooltip = $tStrings.GetString("Zoom out projection...")
cmd.status_bar_text = $tStrings.GetString("Zoom out the building projection...")
cmd.menu_text = $tStrings.GetString("Projection")
toolbar = toolbar.add_item cmd
toolbar = toolbar.add_separator

cmd = UI::Command.new($tStrings.GetString("Test")) { faceAdj (1, model, entities) } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\icon_face.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\icon_face.PNG"
cmd.tooltip = $tStrings.GetString("White Face")
cmd.status_bar_text = $tStrings.GetString("White Face")
cmd.menu_text = $tStrings.GetString("Face")
toolbar = toolbar.add_item cmd

cmd = UI::Command.new($tStrings.GetString("Test")) { colorAdj (1, model, entities) } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\icon_color.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\icon_color.PNG"
cmd.tooltip = $tStrings.GetString("Color Face")
cmd.status_bar_text = $tStrings.GetString("Color Face")
cmd.menu_text = $tStrings.GetString("Color_Face")
toolbar = toolbar.add_item cmd

cmd = UI::Command.new($tStrings.GetString("Test")) { wfAdj (1, model, entities) } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\icon_wf.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\icon_wf.PNG"
cmd.tooltip = $tStrings.GetString("Show Wireframe")
cmd.status_bar_text = $tStrings.GetString("Show Wireframe")
cmd.menu_text = $tStrings.GetString("Wireframe")
toolbar = toolbar.add_item cmd

cmd = UI::Command.new($tStrings.GetString("Test")) { errorAdj (1, model, entities) } 
cmd.small_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\err.PNG"
cmd.large_icon = "C:\\Program Files\\Google\\Google SketchUp 6\\Plugins\\icons\\err.PNG"
cmd.tooltip = $tStrings.GetString("Error measurement")
cmd.status_bar_text = $tStrings.GetString("Error measurement")
cmd.menu_text = $tStrings.GetString("Error")
toolbar = toolbar.add_item cmd

toolbar.show


state = toolbar.get_last_state
if ( ! state)
  UI.messagebox "Failure"
end

end # def draw_building
UI.menu("PlugIns").add_item("Building Toolbar") { draw_building_toolbar }
