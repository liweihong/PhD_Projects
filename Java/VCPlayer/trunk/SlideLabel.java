/*
 * SlideLabel.java
 *
 * This module encapsulates an image-based slideshow for the purposes of this 
 * program.  It is used to replay a PowerPoint presentation, and a Mimio Ink 
 * recorded whiteboard session.  It takes and resizes all of the images to be 
 * used in the presentation so that they can be displayed at arbitrary speeds 
 * throughout the presentation.  Each SlideLabel runs on it’s own thread so that 
 * multiple components can be simultaneously played in a given presentation.
 */

package VCPlayer;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;


public class SlideLabel extends javax.swing.JLabel implements Runnable{
    private Vector imageList, duration;
    private int totalSlides, place, nextPause;
    private VCP parent;
    private Thread playthread;
    private String ID;
    
    public SlideLabel(Vector IL, Vector DU, int TS, int wdth, int higt, VCP pnt, String id) {
        super();
        imageList = IL;
        duration = DU;
        totalSlides = TS;
        parent = pnt;
        place = 0;
        ID = new String(id);

        ((ImageIcon)imageList.elementAt(0)).setImage(((ImageIcon)imageList.elementAt(0)).getImage().getScaledInstance(wdth,higt,Image.SCALE_SMOOTH));
        this.setIcon((ImageIcon)imageList.elementAt(0));
        nextPause = ((Integer)duration.elementAt(0)).intValue();
    }    
    
    public void resetSlides() {
        stop();
        setIcon((ImageIcon)imageList.elementAt(0));
        place = 0;
        nextPause = ((Integer)duration.elementAt(0)).intValue();
        if (ID.equals("slide")) 
            parent.getParentHandler().setTitle((String)(parent.getSlideFullNames()).elementAt(0));
        
    }
    
    public int getPlace() {
        return place;
    }
    
    public void start() {
        if( playthread == null || !playthread.isAlive() ) {
            playthread = new Thread(this);
            playthread.setDaemon(true);
            playthread.start();
        }
    }
    
    public void stop() {
    	/*  11/28/04: Need to check the stop().
        if(playthread != null)
            playthread.stop();
        */
        playthread = null;
    }
    
    // By Liwh: 9/26/04 to get corresponding position for the particular slide.
    public int getPosition (int placeToJumpTo) {
        int positionNum = 0, totalTimeSoFar = 0;
        while(totalTimeSoFar <= placeToJumpTo) {
            totalTimeSoFar += ((Integer)duration.elementAt(positionNum)).intValue();
            positionNum++;
        }
        
        return -- positionNum;
    }
    
    public void setPosition(int placeToJumpTo) {
        int totalTimeSoFar = 0;
        place = 0;
        if( playthread != null )
            stop();
        while(totalTimeSoFar <= placeToJumpTo) {
            totalTimeSoFar += ((Integer)duration.elementAt(place)).intValue();
            place++;
        }
        nextPause = totalTimeSoFar - placeToJumpTo;
        if (place > 0) place --; // Add by Liwh 10/2/04 for pause problem and change to "<=" on the while loop.
        start();
    }
    
    public void run() {
        for(int j = place; j < totalSlides; j++) {
            System.out.println(ID + " enter new iteration:" + " next: " + nextPause/1000 + "place: " + j);
            if (ID.equals("slide")) {
                System.out.println("place is:" + j);
                parent.waitForCurCursorReady();
                parent.getParentHandler().setTitle((String)(parent.getSlideFullNames()).elementAt(j));
            }
            setIcon((ImageIcon)imageList.elementAt(parent.getCurCursor(ID)));
            int thisPause = nextPause;
            place = j;
            try { 
               Thread.sleep(thisPause); // Must first sleep, then move to the next frame.
               if(j < totalSlides - 1) {
                    nextPause = ((Integer)duration.elementAt(j+1)).intValue();
                    parent.moveCurCursor(ID);
                }
            }
            catch( InterruptedException e) {System.out.println("thread sleep problem");}
        }
        // 11/28/04: Any use?
        Runnable stopper = new Runnable() {
            public void run() { parent.stop(); }
        };
        SwingUtilities.invokeLater(stopper);
    }
    
}

