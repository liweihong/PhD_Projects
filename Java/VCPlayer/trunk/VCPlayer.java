/*
 * VCPlayer.java
 *
 * This class is the main application class.  It contains two panels which 
 * represent the creator mode and the player mode.  When the user clicks on the 
 * appropriate menu item the player module (VCP) or creator module (VCC) is 
 * initialized and set as the current pane.
 *
 * 
 * The timing algorithm is currently split into three parts.  The first part 
 * involves collecting and formatting the timing information.  The second part
 * involves mapping multiple timelines onto a common presentation timeline and  
 * calculating the durations for each slide.  Th third part invlolves 
 * dynamically calculating the duration of a slide when the user jumps to 
 * different places with the table of contents.  
 *
 * The VCC module does the first part which is timing collection and formatting.
 * There is no collecting or formatting necessary for the slides component of a 
 * presentation because the PPT add-in does all of this.  However, when a 
 * presentation that contains whiteboard slides is created the inforamtion is 
 * buried in multiple html files.  When the whiteboard component is first added,
 * this module explores all of the pageXXX.html files and extracts the 
 * timestamp.  Each timestamp is converted from HH:MM:SS (AM/PM) format to 
 * a XXXXXXX seconds format.  Once formatted it is saved to a file named 
 * "whiteboard.txt" in the same directory as the html files.
 *
 * The VCP module does the second part.  First it reads the PowerPoint add-in
 * timing file and creates a list of the slide durations.  It also reads in the
 * start time which is the recorded system time at the very start of the 
 * PowerPoint presentation.  Next the "whiteboard.txt" file is read and the 
 * times are added to antoher list.  Next the first time in the whiteboard list,
 * which corrisponds to the last altered time of the first slide, is subtracted 
 * from the start time to calculate the duration of the first slide.  Then the
 * rest of the whiteboard slide durations are calculated from subtracting 
 * consecutive entries in the last altered whiteboard list.  Once this is done
 * there are now two lists, one corrisponding to the durations of each 
 * PowerPoint slide and one corrisponding to the duration of each Mimio slide,
 * truncating any time before the PowerPoint was started.  These durations are
 * used to play the presentation.
 *
 * The CustomMenu module does the third part.  When an entry in the TOC is 
 * clicked, the custom menu calculates the time that the start of the 
 * PowerPoint slide corrisponds in relation to the beginning of the 
 * presentation.  It then sends this time to each component in the presentation.
 * In the case of the slide based compoenents, they find the slide that should
 * be displayed at that time, and they calculate the new duration for that 
 * slide because the time may fall after a slides normal start time. 
 */

package VCPlayer;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.*;
import javax.media.*;


public class VCPlayer extends javax.swing.JFrame {
    private VCC myVCC = null;
    private VCP myVCP = null;
    // used to keep track of the last module loaded.
    private boolean creating = false;
    // used for selecting the desired vcp file.
    private JFileChooser chooser;
    private String configFile = "C:/VCPlayer.ini";
    private String currentDirectory = "C:/";
    private PrintWriter outputWriter;
    private BufferedReader inputReader;
    
    //private native void dotransform();
    
    //static {
    //System.loadLibrary("dlltest");
    //}
    
    public VCPlayer() {
        initComponents();
        setSize(895,710);
		//setSize(1280,1024);
        setTitle("VCPlayer");

        String userCD = System.getProperty("user.dir") + "\\";
        
        play.setIcon(new ImageIcon(userCD + "images\\play.JPG"));
        play.setVisible(false);
        stop.setIcon(new ImageIcon(userCD + "images\\stop.JPG"));
        stop.setVisible(false);
        pause.setIcon(new ImageIcon(userCD + "images\\pause.JPG"));
        pause.setVisible(false);
        ffwd.setIcon(new ImageIcon(userCD + "images\\ffwd.JPG"));
        ffwd.setVisible(false);
        rew.setIcon(new ImageIcon(userCD + "images\\rew.JPG"));
        rew.setVisible(false);
        jProgressBar1.setVisible(false);
        jLabel1.setVisible(false);
        setLocation(100,100);

        chooser = new JFileChooser();
        try {
            inputReader = new BufferedReader(new FileReader(configFile));
            currentDirectory =  inputReader.readLine();
        }
        catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        if (currentDirectory == null) currentDirectory = "C:/";
        File currDir = new File (currentDirectory);
        chooser.setCurrentDirectory(currDir);
        
    }    
   

    public String getCD() {
        return currentDirectory;
    }
    public JFileChooser getFC() {
        return chooser;
    }
    
    private void initComponents() {//GEN-BEGIN:initComponents
        jProgressBar1 = new javax.swing.JProgressBar();
        jLabel1 = new javax.swing.JLabel();
        jMenuBar1 = new javax.swing.JMenuBar();
        creator = new javax.swing.JMenu();
        New = new javax.swing.JMenuItem();
        Open = new javax.swing.JMenuItem();
        Save = new javax.swing.JMenuItem();
        player = new javax.swing.JMenu();
        POpen = new javax.swing.JMenuItem();
        jMenu1 = new javax.swing.JMenu();
        Mp3Creator = new javax.swing.JMenuItem();
        imaget = new javax.swing.JMenu();
        imagetransform = new javax.swing.JMenuItem();
        play = new javax.swing.JMenu();
        stop = new javax.swing.JMenu();
        pause = new javax.swing.JMenu();
        rew = new javax.swing.JMenu();
        ffwd = new javax.swing.JMenu();

        getContentPane().setLayout(null);

        setTitle("VCPlayer");
        setBackground(new java.awt.Color(254, 254, 254));
        setFont(new java.awt.Font("Times New Roman", 0, 12));
        setName("VCPlayer");
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                exitForm(evt);
            }
        });

        jProgressBar1.setForeground(new java.awt.Color(255, 51, 102));
        jProgressBar1.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0)));
        getContentPane().add(jProgressBar1);
        jProgressBar1.setBounds(720, 10, 148, 14);

        jLabel1.setText("Loading...");
        getContentPane().add(jLabel1);
        jLabel1.setBounds(720, 30, 160, 18);

        creator.setText("Creator");
        New.setText("New");
        New.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NewActionPerformed(evt);
            }
        });

        creator.add(New);

        Open.setText("Open");
        Open.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                OpenActionPerformed(evt);
            }
        });

        creator.add(Open);

        Save.setText("Save");
        Save.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SaveActionPerformed(evt);
            }
        });

        creator.add(Save);

        jMenuBar1.add(creator);

        player.setText("Player");
        player.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                playerActionPerformed(evt);
            }
        });

        POpen.setText("Open");
        POpen.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                POpenActionPerformed(evt);
            }
        });

        player.add(POpen);

        jMenuBar1.add(player);

        jMenu1.setText("Tools");
        Mp3Creator.setText("Mp3 Creator");
        Mp3Creator.setToolTipText("Convert wav file to mp3");
        Mp3Creator.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                Mp3CreatorActionPerformed(evt);
            }
        });

        jMenu1.add(Mp3Creator);

        jMenuBar1.add(jMenu1);

        imaget.setText("Image Transform");
        imaget.setEnabled(false);
        imagetransform.setText("Transform the Image");
        imagetransform.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                imagetransformActionPerformed(evt);
            }
        });

        imaget.add(imagetransform);

        jMenuBar1.add(imaget);

        play.setToolTipText("play presentation");
        play.setMargin(new java.awt.Insets(0, 0, 0, 0));
        play.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                playMouseClicked(evt);
            }
        });

        jMenuBar1.add(play);

        stop.setToolTipText("stop presentation");
        stop.setMargin(new java.awt.Insets(0, 0, 0, 0));
        stop.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                stopMouseClicked(evt);
            }
        });

        jMenuBar1.add(stop);

        pause.setToolTipText("pause presentation");
        pause.setMargin(new java.awt.Insets(0, 0, 0, 0));
        pause.setPreferredSize(new java.awt.Dimension(35, 25));
        pause.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                pauseMouseClicked(evt);
            }
        });

        jMenuBar1.add(pause);

        rew.setToolTipText("jump to previous slide");
        rew.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                rewMouseClicked(evt);
            }
        });

        jMenuBar1.add(rew);

        ffwd.setToolTipText("jump to next slide");
        ffwd.setMargin(new java.awt.Insets(0, 0, 0, 0));
        ffwd.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                ffwdMouseClicked(evt);
            }
        });

        jMenuBar1.add(ffwd);

        setJMenuBar(jMenuBar1);

        pack();
    }//GEN-END:initComponents

    private void Mp3CreatorActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_Mp3CreatorActionPerformed
        // Add your handling code here:
        Mp3Creator temp = new Mp3Creator(this,true);
        temp.show();
    }//GEN-LAST:event_Mp3CreatorActionPerformed

    private void playerActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_playerActionPerformed
        // Add your handling code here:
    }//GEN-LAST:event_playerActionPerformed

    private void ffwdMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_ffwdMouseClicked
        if (ffwd.isEnabled()) myVCP.fwd();
    }//GEN-LAST:event_ffwdMouseClicked

    private void rewMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_rewMouseClicked
        if (rew.isEnabled()) myVCP.rew();
    }//GEN-LAST:event_rewMouseClicked

    private void pauseMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_pauseMouseClicked
        if (pause.isEnabled()) myVCP.pause();
    }//GEN-LAST:event_pauseMouseClicked

    private void stopMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_stopMouseClicked
        if (stop.isEnabled()) myVCP.stop();
    }//GEN-LAST:event_stopMouseClicked

    private void playMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_playMouseClicked
        if (play.isEnabled()) myVCP.play();
    }//GEN-LAST:event_playMouseClicked

    private void imagetransformActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_imagetransformActionPerformed
       //dotransform();
    }//GEN-LAST:event_imagetransformActionPerformed
/*
 * This function loads the VCC module from an existing presentation.
 */
    private void OpenActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_OpenActionPerformed
        if(myVCC != null)
            this.remove(myVCC);
        if(myVCP != null)
            this.remove(myVCP);
        play.setVisible(false);
        stop.setVisible(false);
        pause.setVisible(false);
        ffwd.setVisible(false);
        rew.setVisible(false);
        myVCC = new VCC(this,chooser);
        this.getContentPane().add(myVCC);
        this.repaint();
        creating = true;
        myVCC.open();
    }//GEN-LAST:event_OpenActionPerformed

    private void SaveActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SaveActionPerformed
        if(myVCC != null)
            myVCC.save();
    }//GEN-LAST:event_SaveActionPerformed
/*
 * This function displays the file chooser for the user to select the desired
 * .vcp file for a presentation.
 */
    private void POpenActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_POpenActionPerformed
        if(creating && myVCC != null)
            this.remove(myVCC);
        if(myVCP != null)
            this.remove(myVCP);
        this.repaint();
        myVCP = new VCP(chooser,this);
        this.getContentPane().add(myVCP);
        this.repaint();
        creating = false;
    }//GEN-LAST:event_POpenActionPerformed
/*
 * This loads the VCC module for the creation of a new presentation.
 */
    private void NewActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NewActionPerformed
        if(!creating && myVCP != null)
            this.remove(myVCP);
        if(myVCC != null)
            this.remove(myVCC);
        play.setVisible(false);
        stop.setVisible(false);
        pause.setVisible(false);
        ffwd.setVisible(false);
        rew.setVisible(false);
        this.repaint();
        myVCC = new VCC(this, chooser);
        this.getContentPane().add(myVCC);
        this.repaint();
        creating = true;
    }//GEN-LAST:event_NewActionPerformed

    private void stopButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stopButtonActionPerformed
        this.repaint();
    }//GEN-LAST:event_stopButtonActionPerformed

    private void playButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_playButtonActionPerformed
        this.repaint();
    }//GEN-LAST:event_playButtonActionPerformed

    /** Exit the Application */
    private void exitForm(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_exitForm
        try{ // Save the directory of the tid files.
            outputWriter = new PrintWriter(new FileOutputStream(configFile));
            if (myVCC != null)
                currentDirectory = myVCC.getCD();
            else if (myVCP != null)
                currentDirectory = myVCP.getCD();
            outputWriter.println(currentDirectory);
            outputWriter.close();
        }
        catch (Exception e) { 
            System.err.println("Error: " + e.getMessage()); 
        }
        System.exit(0);
    }//GEN-LAST:event_exitForm
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        new VCPlayer().show();
    }
    
    public JProgressBar getProgressBar() {
        return jProgressBar1;
    }
    
    public JLabel getLabel() {
        return jLabel1;
    }

    public void setCtlBtnVisible(boolean isVisible) {
        play.setVisible(isVisible);
        pause.setVisible(isVisible);
        stop.setVisible(isVisible);
        rew.setVisible(isVisible);
        ffwd.setVisible(isVisible);
    }   
    
    public void setCtlBtnEnabled(boolean isEnabled) {
        play.setEnabled(isEnabled);
        pause.setEnabled(isEnabled);
        stop.setEnabled(isEnabled);
        rew.setEnabled(isEnabled);
        ffwd.setEnabled(isEnabled);
    }   
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JMenuItem Mp3Creator;
    private javax.swing.JMenuItem New;
    private javax.swing.JMenuItem Open;
    private javax.swing.JMenuItem POpen;
    private javax.swing.JMenuItem Save;
    private javax.swing.JMenu creator;
    private javax.swing.JMenu ffwd;
    private javax.swing.JMenu imaget;
    private javax.swing.JMenuItem imagetransform;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JMenu jMenu1;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JProgressBar jProgressBar1;
    private javax.swing.JMenu pause;
    private javax.swing.JMenu play;
    private javax.swing.JMenu player;
    private javax.swing.JMenu rew;
    private javax.swing.JMenu stop;
    // End of variables declaration//GEN-END:variables
    
    
}

