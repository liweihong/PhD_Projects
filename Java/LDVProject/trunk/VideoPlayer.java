/*
 * VideoPlayer.java
 *
 * Created on August 30, 2004, 1:05 PM
 */

package LDVProject;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.*;
import javax.media.*;

/**
 *
 * @author  weihong
 */
public class VideoPlayer extends javax.swing.JPanel {
    
    private Player videoPlayer, audioPlayer;
    private JPanel video;
    private JButton playBtn, stopBtn, pauseBtn;
    
    /** Creates a new instance of VideoPlayer */
    public VideoPlayer() {

        setLayout(new BorderLayout());
        
        //Button for runniing the graph
        playBtn = new JButton("Play");
        playBtn.addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                videoPlayer.setMediaTime(new javax.media.Time(0));
                videoPlayer.start();
                //audioPlayer.start();
            }
        });
     
        //Button for runniing the graph
        stopBtn = new JButton("Stop");
        stopBtn.addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                videoPlayer.stop();
                //audioPlayer.stop();
            }
        });
        
        JPanel p = new JPanel ();
        p.add(playBtn);
        p.add(stopBtn);

        add("South",  p);
        
        try {
            File tempFile = new File("C:\\Documents and Settings\\Weihong Li\\Desktop\\Docs\\study\\Videos\\OUTPUT_test1.mpeg");
            //File tempAudioFile = new File("C:\\Program Files\\Sony\\MovieShaker\\Samples\\music001.mp3");
            //File tempAudioFile = new File("C:\\Program Files\\Sony\\MovieShaker\\Samples\\music001.mp3");
            videoPlayer = Manager.createRealizedPlayer(tempFile.toURL());
            //audioPlayer = Manager.createRealizedPlayer(tempAudioFile.toURL());
            video = new JPanel(new FlowLayout(FlowLayout.CENTER, 0, 100));
            //video = new JPanel();

            Runnable setupVideo = new Runnable() {
                public void run() { 
                    Component playerComponent = videoPlayer.getVisualComponent();
                    //playerComponent.setLocation(100,100);
                    //playerComponent.setSize(400, 300);
                    video.add(playerComponent);
                    video.setVisible(true);  
                }
            };
            SwingUtilities.invokeLater(setupVideo);
            add(video);
        }
        catch (Exception e) {
            System.err.println("video setup problem");
        }
        
    }
    
}
