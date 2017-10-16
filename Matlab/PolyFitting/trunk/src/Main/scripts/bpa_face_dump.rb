require 'sketchup'



# function: draw_bpa_all1
def draw_bpa_all1

        of = File.new("C:\\Program Files\\Google\\Google SketchUp 7\\Plugins\\IR_all_groups.txt", "w")
        model = Sketchup.active_model
        entities = model.entities
        faceID = 0
        entities.each { | e |
            if e.class == Sketchup::Face
                loops = e.loops
                normal = e.normal
                of.print sprintf("\#{faceID: %d} \#{loops.length: %d} \#{%f} \#{%f} \#{%f}\n", faceID, loops.length, normal.x.to_f, normal.y.to_f, normal.z.to_f)
                for i in 0..loops.length-1
                    verts = loops[i].vertices
                    of.print sprintf("\#{verts.length: %d}\n", verts.length)
                    verts.each do |v|
                        pt = v.position
                        of.print sprintf("\#{%f} \#{%f} \#{%f}\n", pt.x.to_f, pt.y.to_f, pt.z.to_f)
                    end
                end
                faceID += 1
            end
        } 
        
        of.close

end # def draw_bpa_all1

# function: draw_bpa_all
def draw_bpa_all

        of = File.new("C:\\Program Files\\Google\\Google SketchUp 7\\Plugins\\IR_all_faces.txt", "w")
        model = Sketchup.active_model
        entities = model.entities
        faceID = 0
        entities.each { | e |
            if e.class == Sketchup::Face
                loops = e.loops
                normal = e.normal
                for i in 0..loops.length-1
                    verts = loops[i].vertices
                    of.print sprintf("BEGIN POLYGON\nDIRECTION\n1 %d\nHEIGHT\n-27.651632 -27.651632\nTYPE\n2\nNORMAL\n%f %f %f\nPOINTS\n", i, normal.x.to_f, normal.y.to_f, normal.z.to_f)
                    verts.each do |v|
                        pt = v.position
                        of.print sprintf("%f %f %f\n", pt.x.to_f, pt.y.to_f, pt.z.to_f)
                    end
                    of.print sprintf("EXTRUSION\nEND POLYGON\n")
                end
                faceID += 1
            end
        } 
        of.close

end # def draw_bpa_all

UI.menu("PlugIns").add_item("DUMP BPA") { draw_bpa_all }

#p1=Geom::Point3d.new(72.725609, -40.000000, 4.947105)
#p2=Geom::Point3d.new(72.725609, 100.000000, 4.947105)
#my_edge=Sketchup.active_model.entities.add_line p1, p2
#Sketchup.active_model.selection.toggle my_edge # add or remove entities from the selection
#Sketchup.active_model.selection[0].normal  # print out the normal of face
