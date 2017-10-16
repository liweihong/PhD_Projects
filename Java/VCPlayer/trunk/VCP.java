/*
 * VCP.java
 *
 * This serves as the main window during a presentation.  It loads all of the 
 * images, video(s), audio, timing information, etc, from a .vcp file.  If 
 * there are either PowerPoint slides or merged images it will setup a popup 
 * TOC menu for the presentation.  After it loads all of the information it 
 * displays it as it was designed in the VCC.  The play button and the TOC 
 * popup menu can be used to start the presentation and the stop button can be 
 * used to stop it.
 */

package VCPlayer;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.*;
import javax.media.*;


public class VCP extends javax.swing.JPanel implements Runnable{
    private SlideLabel slides, whiteboard, both,camera;
    private Panel video, av;
    private CustomMenu theMenu;
    private Player videoPlayer, audioPlayer, avPlayer;
    //private javax.media.Time converter;
    private Vector slideImageList, slideDuration, slideNames, slideFullNames, whiteboardDuration, 
            whiteboardImageList, bothImageList, bothDuration, cameraImageList, cameraDuration;
    private int slidesTotalSlides, whiteboardTotalSlides, bothTotalSlides, cameraTotalSlides, startTime, pausedPlace;
    private double totalDuration;
    private JFileChooser chooser;
    private String currentVCPDirectory = null;
    private String slideCD = null, whiteboardCD = null, bothCD = null, cameraCD = null;
    private String names[];
    private int sizeAndLoc[][];
    private boolean paused;
    private Thread loadThread;
    private VCPlayer parent;
    // Used to circling buffer.
    private int CIRCLE_BUFFER_SIZE = 3;
    public static final int MENU_WIDTH = 30; // the CustomMenu will use this.
    public static final int VOID = -1;
    private int slideCur = VOID, whiteboardCur = VOID, bothCur = VOID, cameraCur = VOID, clearNumber = VOID;
    private boolean slideCircleStatus[] = new boolean [CIRCLE_BUFFER_SIZE];
    private boolean whiteboardCircleStatus[] = new boolean [CIRCLE_BUFFER_SIZE];
    private boolean bothCircleStatus[] = new boolean [CIRCLE_BUFFER_SIZE];
    private boolean cameraCircleStatus[] = new boolean [CIRCLE_BUFFER_SIZE];
    // *Cur is range from 0,1,2:  0 - backward, 1 - current, 2 - forward
    // *ImageList[*Cur] contains the current valid transformed image.
    private int slidePositionNum = 0;
    // Slide postion, start at 0, end with the last slide number.
    private ReadingThread readingThread = null;
    // To save the file name in the vector list.
    private Vector slideFileNameList, whiteboardFileNameList, bothFileNameList, cameraFileNameList;
    
    /** Creates a new instance of VCP */
    public VCP(JFileChooser ch,VCPlayer vcp) {
        super();
        parent =  vcp;
        setBackground(new java.awt.Color(224, 224, 224));
        //setBounds(0,0,895,710);
		setBounds(0,0,1280,1024);
        setLayout(null);
        //converter = new javax.media.Time(0);
        slideImageList = new Vector();
        whiteboardImageList = new Vector();
        bothImageList = new Vector();
        cameraImageList = new Vector();
        slideDuration = new Vector();
        whiteboardDuration = new Vector();
        bothDuration = new Vector();
        cameraDuration = new Vector();
        slideNames = new Vector();
        // By Liwh: 9/24/04 Initialize Vectors. the size 100 doesn't matter.
        slideFileNameList = new Vector(100);
        whiteboardFileNameList = new Vector(100);
        bothFileNameList = new Vector(100);
        cameraFileNameList = new Vector(100);
        slideFullNames = new Vector(100);
        slidesTotalSlides = 0;
        whiteboardTotalSlides = 0;
        bothTotalSlides = 0;
        cameraTotalSlides = 0;
        totalDuration = 0;
        startTime = 0;
        paused = false;
        names = new String[7];
        sizeAndLoc = new int[7][4];
        chooser = ch;
        currentVCPDirectory = ch.getCurrentDirectory().getPath();
        open();
        if(videoPlayer != null)
            videoPlayer.setMediaTime(new javax.media.Time(0));
        if(audioPlayer != null)
            audioPlayer.setMediaTime(new javax.media.Time(0));
        if(avPlayer != null)
            avPlayer.setMediaTime(new javax.media.Time(0));
    }
    
    public String getCD() {
        return currentVCPDirectory;
    }
    
    public void readBothFile(String bothFile, boolean firstRead) {
        BufferedReader input = null;
        bothCD = (new File(bothFile)).getParent();
        
        try {
            if (firstRead) {
                input = new BufferedReader ( new InputStreamReader((new FileInputStream(bothCD + "\\" + "both.txt"))));
                bothTotalSlides = Integer.parseInt(input.readLine());
                input.close();

                input = new BufferedReader ( new InputStreamReader((new FileInputStream(bothCD + "\\" + "whiteboard.txt"))));
                String temp = input.readLine();
                int previous = startTime, thisDuration, tempInt;
                int onlyOne = 1;            
                int counter = 0;
                while(temp != null) {
                    // Just make sure the amount is not out of range. by liwh: 9/27/04
                    if (counter >= bothTotalSlides) break; 
                    
                    tempInt = Integer.parseInt(temp);
                    thisDuration = tempInt - previous;
                    previous = tempInt;
                    bothDuration.addElement(new Integer(thisDuration));
                    bothFileNameList.addElement(counter + ".jpg");
                    if (onlyOne == 1) {
                        ImageIcon theImage = new ImageIcon(bothCD + "\\" + counter + ".jpg");
                        bothImageList.addElement(theImage);
                        onlyOne ++;
                    }
                    counter ++;
                    temp = input.readLine();
                }
                
                input.close();
            }
            else { // Read the file for first loading. 
                parent.getProgressBar().setVisible(true);
                parent.getLabel().setVisible(true);
                parent.getProgressBar().setMaximum(CIRCLE_BUFFER_SIZE);
                parent.getProgressBar().setValue(1);
                parent.getLabel().setText("Loading aligned images...");

                for (int i = 1; i < CIRCLE_BUFFER_SIZE; i++)
                {   
                    String flname = (String)bothFileNameList.get(i);
                    ImageIcon theImage = new ImageIcon(bothCD + "\\" + flname);
                    theImage.setImage(theImage.getImage().getScaledInstance(sizeAndLoc[5][2],sizeAndLoc[5][3],Image.SCALE_SMOOTH));
                    bothImageList.addElement(theImage);
                    parent.getProgressBar().setValue(i+1);
                }
            } // End of else
        } // end of try.
        catch (Exception e) {
            JOptionPane.showMessageDialog(this,e.getMessage(),"Exception Catched:", JOptionPane.ERROR_MESSAGE);
        }
        
    }
    
    public void readWhiteboardFile(String whiteboardDirectory, boolean firstRead) {
		BufferedReader input = null;
        whiteboardCD = whiteboardDirectory;
        
        try {
            if (firstRead) {
                input = new BufferedReader ( new InputStreamReader ((new FileInputStream(whiteboardDirectory + "\\" + "whiteboard.txt"))));
                String temp = input.readLine();
                int ones = 0, tens = 0, hundreds = 0, previous = startTime, thisDuration, tempInt;
                ImageIcon img;
                int onlyOne = 1;            
                while(temp != null) {
                    whiteboardTotalSlides++;
                    tempInt = Integer.parseInt(temp);
                    thisDuration = tempInt - previous;
                    previous = tempInt;
                    whiteboardDuration.addElement(new Integer(thisDuration));
                    whiteboardFileNameList.addElement("img" + hundreds + tens + ones + ".jpg");
                    if (onlyOne == 1) {
                        img = new ImageIcon(whiteboardDirectory + "\\" + "img" + hundreds + tens + ones + ".jpg");
                        img.setImage(img.getImage().getScaledInstance(sizeAndLoc[4][2],sizeAndLoc[4][3],Image.SCALE_SMOOTH));
                        whiteboardImageList.addElement(img);
                        onlyOne ++;
                    }
                    if(ones == 9) {
                        if(tens == 9) {
                            ones = 0;
                            tens = 0;
                            hundreds++;
                        }
                        else {
                            ones = 0;
                            tens++;
                        }
                    }
                    else
                        ones++;
                    temp = input.readLine();
                }
            }
            else { // Read the file for first loading. 
                parent.getProgressBar().setVisible(true);
                parent.getLabel().setVisible(true);
                parent.getProgressBar().setMaximum(CIRCLE_BUFFER_SIZE);
                parent.getProgressBar().setValue(1);
                parent.getLabel().setText("Loading whiteboard images...");

                for (int i = 1; i < CIRCLE_BUFFER_SIZE; i++)
                {   
                    String flname = (String)whiteboardFileNameList.get(i);
                    ImageIcon theImage = new ImageIcon(whiteboardCD + "\\" + flname);
                    theImage.setImage(theImage.getImage().getScaledInstance(sizeAndLoc[4][2],sizeAndLoc[4][3],Image.SCALE_SMOOTH));
                    whiteboardImageList.addElement(theImage);
                    parent.getProgressBar().setValue(i+1);
                }
            } // End of else
        } // end of try.
        catch (Exception e) {
            JOptionPane.showMessageDialog(this,e.getMessage(),"Exception Catched:", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    private void adjustDuration() {
        int totalTimeSlides = ((Integer)slideDuration.elementAt(0)).intValue();
        int whiteboardTimeDuration = ((Integer)whiteboardDuration.elementAt(0)).intValue();
        int place = 1, tempTime = 0;
        int curWBTimeDuration = whiteboardTimeDuration;
        int whiteboardNum = whiteboardDuration.size();
        
        for (int i=0; i < whiteboardNum; i++) {
            while(totalTimeSlides < whiteboardTimeDuration) {
                totalTimeSlides += ((Integer)slideDuration.elementAt(place)).intValue();
                place++;
            }

            whiteboardDuration.setElementAt(new Integer ((totalTimeSlides - tempTime)), i);
            tempTime = totalTimeSlides;
            if ( i+1 < whiteboardNum) {
                curWBTimeDuration = ((Integer)slideDuration.elementAt(i+1)).intValue();
                whiteboardTimeDuration += curWBTimeDuration;
            }
        }
    }
    
    private void readSlideFile(String timeFile, boolean firstRead) {
		BufferedReader input = null;
        slideCD = (new File(timeFile)).getParent();
        
        try {
            if (firstRead) {
                input = new BufferedReader (new InputStreamReader((new FileInputStream(timeFile))));
                input.readLine();
                StringTokenizer tempTokens = new StringTokenizer(input.readLine());
                int tempHrs = Integer.parseInt(tempTokens.nextToken(": "));
                int tempMin = Integer.parseInt(tempTokens.nextToken(": "));
                double tempSec = Double.parseDouble(tempTokens.nextToken(": "));
                tempSec = tempSec*1000;
                startTime = tempHrs*3600000 + tempMin*60000 + (int)tempSec;
                input.readLine();
                input.readLine();
                input.readLine();
                int onlyOne = 1;
                while(true) {
                    String temp = input.readLine();
                    if(temp == null) break;
                    else {
                        StringTokenizer tokens = new StringTokenizer(temp);
                        tokens.nextToken();
                        tokens.nextToken();
                        int hours = Integer.parseInt(tokens.nextToken(" \n\t\r:"));
                        int minutes = Integer.parseInt(tokens.nextToken(" \n\t\r:"));
                        double seconds = Double.parseDouble(tokens.nextToken(" \n\t\r:"));
                        seconds = seconds*1000;
                        int inSeconds = 3600000*hours + 60000*minutes + (int)seconds;
                        slideDuration.addElement(new Integer(inSeconds));   
                        totalDuration += (double)inSeconds / 1000;
                        String flname = tokens.nextToken();
                        slideFileNameList.addElement(flname);
                        if (onlyOne == 1) {
                            ImageIcon theImage = new ImageIcon(slideCD + "\\" + flname);
                            slideImageList.addElement(theImage);
                            onlyOne ++;
                        }
                        tokens.nextToken();
                        String tempString = new String(tokens.nextToken("\n\t\r"));
                        slideFullNames.addElement(slidesTotalSlides+1 + " -- " + tempString);
                        if(tempString.length() > MENU_WIDTH)
                            tempString = tempString.substring(0,MENU_WIDTH);
                        slideNames.addElement(slidesTotalSlides+1 + " -- " + tempString);
                        slidesTotalSlides++;
                    }
                } // End of while.
            }
            else { // Read the file for first loading. 
                parent.getProgressBar().setVisible(true);
                parent.getLabel().setVisible(true);
                parent.getProgressBar().setMaximum(CIRCLE_BUFFER_SIZE);
                parent.getProgressBar().setValue(1);
                parent.getLabel().setText("Loading slides...");

                for (int i = 1; i < CIRCLE_BUFFER_SIZE; i++)
                {   
                    String flname = (String)slideFileNameList.get(i);
                    ImageIcon theImage = new ImageIcon(slideCD + "\\" + flname);
                    theImage.setImage(theImage.getImage().getScaledInstance(sizeAndLoc[3][2],sizeAndLoc[3][3],Image.SCALE_SMOOTH));
                    slideImageList.addElement(theImage);
                    parent.getProgressBar().setValue(i+1);
                }
            } // End of else
        } // end of try.
        catch (Exception e) {
            JOptionPane.showMessageDialog(this,e.getMessage(),"Exception Catched:", JOptionPane.ERROR_MESSAGE);
        }
    }

    private void readCameraFile(String timeFile, boolean firstRead) {
		BufferedReader input = null;
        cameraCD = (new File(timeFile)).getParent();
        
        try {
            if (firstRead) {
                input = new BufferedReader ( new InputStreamReader((new FileInputStream(timeFile))));
                StringTokenizer tempTokens = new StringTokenizer(input.readLine());
                String flname = tempTokens.nextToken();
                int tempHrs = Integer.parseInt(tempTokens.nextToken(" \n\t\r:"));
                int tempMin = Integer.parseInt(tempTokens.nextToken(" \n\t\r:"));
                double tempSec = Double.parseDouble(tempTokens.nextToken(" \n\t\r:"));
                tempSec = tempSec*1000;
                int cameraStartTime = tempHrs*3600000 + tempMin*60000 + (int)tempSec;
                int preSeconds = cameraStartTime;
                int onlyOne = 1;
                while(true) {
                    String temp = input.readLine();
                    if(temp == null) break;
                    else {
                        StringTokenizer tokens = new StringTokenizer(temp);
                        String nextFileName = tokens.nextToken();
                        int hours = Integer.parseInt(tokens.nextToken(" \n\t\r:"));
                        int minutes = Integer.parseInt(tokens.nextToken(" \n\t\r:"));
                        double seconds = Double.parseDouble(tokens.nextToken(" \n\t\r:"));
                        seconds = seconds*1000;
                        int inSeconds = 3600000*hours + 60000*minutes + (int)seconds;
                        cameraDuration.addElement(new Integer(inSeconds - preSeconds));   
                        preSeconds = inSeconds;
                        cameraFileNameList.addElement(flname);
                        if (onlyOne == 1) {
                            ImageIcon theImage = new ImageIcon(cameraCD + "\\" + flname);
                            cameraImageList.addElement(theImage);
                            onlyOne ++;
                        }
                        flname = nextFileName;
                        cameraTotalSlides++;
                    }
                } // End of while.
            }
            else { // Read the file for first loading. 
                parent.getProgressBar().setVisible(true);
                parent.getLabel().setVisible(true);
                parent.getProgressBar().setMaximum(CIRCLE_BUFFER_SIZE);
                parent.getProgressBar().setValue(1);
                parent.getLabel().setText("Loading camera images...");

                for (int i = 1; i < CIRCLE_BUFFER_SIZE; i++)
                {   
                    String flname = (String)cameraFileNameList.get(i);
                    ImageIcon theImage = new ImageIcon(cameraCD + "\\" + flname);
                    theImage.setImage(theImage.getImage().getScaledInstance(sizeAndLoc[6][2],sizeAndLoc[6][3],Image.SCALE_SMOOTH));
                    cameraImageList.addElement(theImage);
                    parent.getProgressBar().setValue(i+1);
                }
            } // End of else
        } // end of try.
        catch (Exception e) {
            JOptionPane.showMessageDialog(this,e.getMessage(),"Exception Catched:", JOptionPane.ERROR_MESSAGE);
        }
    }
    
    public void play() {
        if(paused) {
            setPosition(pausedPlace);
            paused = false;
        }
        else {
            if(videoPlayer != null)
                videoPlayer.start();
            if(audioPlayer != null)
                audioPlayer.start();
            if(avPlayer != null)
                avPlayer.start();
            if(slides != null)
                slides.start();
            if(whiteboard != null)
                whiteboard.start();
            if(both != null)
                both.start();
            if(camera != null)
                camera.start();
        }
    }
    
    public boolean hasSlides() {
        if(slides != null || both != null)
            return true;
        else
            return false;
    }
    
    public void pause() {
        if(paused == false) {
            pausedPlace = 0;
            if(videoPlayer != null) {
                double tempNum = videoPlayer.getMediaTime().getSeconds();
                pausedPlace = (int)(tempNum * 1000);
            }
            else if(audioPlayer != null) {
                double tempNum = audioPlayer.getMediaTime().getSeconds();
                pausedPlace = (int)(tempNum * 1000);
            }
            else if(avPlayer != null) {
                double tempNum = avPlayer.getMediaTime().getSeconds();
                pausedPlace = (int)(tempNum * 1000);
            }
            else if(slides != null) {
                int totalPlaces = slides.getPlace();
                for(int i = 0; i < totalPlaces; i++) {
                    pausedPlace += ((Integer)slideDuration.elementAt(i)).intValue();
                }
            }
            else if(whiteboard != null) {
                int totalPlaces = whiteboard.getPlace();
                for(int i = 0; i < totalPlaces; i++) {
                    pausedPlace += ((Integer)whiteboardDuration.elementAt(i)).intValue();
                }
            }
            else if(both != null) {
                int totalPlaces = both.getPlace();
                for(int i = 0; i < totalPlaces; i++) {
                    pausedPlace += ((Integer)bothDuration.elementAt(i)).intValue();
                }
            }
            else if(camera != null) {
                int totalPlaces = camera.getPlace();
                for(int i = 0; i < totalPlaces; i++) {
                    pausedPlace += ((Integer)cameraDuration.elementAt(i)).intValue();
                }
            }            
            //stop();
            if(videoPlayer != null)
                videoPlayer.stop();
            if(audioPlayer != null) 
                audioPlayer.stop();
            if(avPlayer != null) 
                avPlayer.stop();
            if(slides != null)
                slides.stop();
            if(whiteboard != null)
                whiteboard.stop();
            if(both != null)
                both.stop();
            if(camera != null)
                camera.stop();
            // Like stop, but we don't change the slides show.
        }
        else {
            setPosition(pausedPlace);
        }
        paused = !paused;
    }
    
    public void rew() {
        paused = false;
        
        if (slidePositionNum == 0) return; // Do nothing if at the first slide.
        
        if(slides != null) {
            int place = slides.getPlace() - 1;
            backCurCursor();
            waitForCurCursorReady();
            if(place >= 0) {
                int newTime = 0;
                for(int i = 0; i < place; i++)
                    newTime += ((Integer)slideDuration.elementAt(i)).intValue();
                setPosition(newTime);
            }
        }
        else if(both != null) {
            int place = both.getPlace() - 1;
            backCurCursor();
            waitForCurCursorReady();
            if(place >= 0) {
                int newTime = 0;
                for(int i = 0; i < place; i++)
                    newTime += ((Integer)bothDuration.elementAt(i)).intValue();
                setPosition(newTime);
            }
        }
    }
    
    public void fwd() {
        paused = false;
        
        if (slidePositionNum == slidesTotalSlides - 1) return; // Do nothing if at the first slide.

        if(slides != null) {
            int place = slides.getPlace();
            //clearLoaded(place+1);
            moveCurCursor();
            waitForCurCursorReady();
            if(place < slidesTotalSlides) {
                int newTime = 0;
                for(int i = 0; i <= place; i++)
                    newTime += ((Integer)slideDuration.elementAt(i)).intValue();
                setPosition(newTime);
            }
        }  
        else if(both != null) {
            int place = both.getPlace();
            //clearLoaded(place);
            moveCurCursor();
            waitForCurCursorReady();
            if(place < bothTotalSlides) {
                int newTime = 0;
                for(int i = 0; i <= place; i++)
                    newTime += ((Integer)bothDuration.elementAt(i)).intValue();
                setPosition(newTime);
            }
        }
    }
    
    public void setPosition(int mediaTime) {
        
        if(videoPlayer != null)
            videoPlayer.stop();
        if(audioPlayer != null)
            audioPlayer.stop();
        if(avPlayer != null)
            avPlayer.stop();
        double temp = (double)mediaTime / 1000;
        if(videoPlayer != null)
            videoPlayer.setMediaTime(new javax.media.Time(temp));
        if(audioPlayer != null)
            audioPlayer.setMediaTime(new javax.media.Time(temp));
        if(avPlayer != null)
            avPlayer.setMediaTime(new javax.media.Time(temp));
        if(videoPlayer != null)
            videoPlayer.start();
        if(audioPlayer != null)
            audioPlayer.start();
        if(avPlayer != null)
            avPlayer.start();
        if(slides != null)
            slides.setPosition(mediaTime);
        if(whiteboard != null)
            whiteboard.setPosition(mediaTime);
        if(both != null)
            both.setPosition(mediaTime);
        if(camera != null)
            camera.setPosition(mediaTime);
    }
    

    public void stop() {
        paused = false;
        if(videoPlayer != null) {
            videoPlayer.stop();
            videoPlayer.setMediaTime(new javax.media.Time(0));
        }
        if(audioPlayer != null) {
            audioPlayer.stop();
            audioPlayer.setMediaTime(new javax.media.Time(0));
        }
        if(avPlayer != null) {
            avPlayer.stop();
            avPlayer.setMediaTime(new javax.media.Time(0));
        }
        
        clearLoaded(0); // Be consistant with the menu selection. by Liwh.
        waitForCurCursorReady();
        
        if(slides != null)
            slides.resetSlides();
        if(whiteboard != null)
            whiteboard.resetSlides();
        if(both != null)
            both.resetSlides();
        if(camera != null)
            camera.resetSlides();
    }
    
    public void open() {
        ExampleFileFilter filter = new ExampleFileFilter();
        filter.addExtension("vcp");
        filter.setDescription("VCP ");
        chooser.setFileFilter(filter);                         
        chooser.setDialogTitle("Open vcp File");
        int returnVal = chooser.showOpenDialog(this);
        if(returnVal == JFileChooser.APPROVE_OPTION) {
			BufferedReader input = null;
            try {
                input = new BufferedReader (new InputStreamReader ((new FileInputStream(chooser.getSelectedFile()))));
                currentVCPDirectory = chooser.getCurrentDirectory().getPath();
                StringTokenizer tokens;
                for(int i = 0; i < 7; i++) {
                    names[i] = input.readLine();
                    if(!names[i].equals("none")) {
                        tokens = new StringTokenizer(input.readLine());
                        sizeAndLoc[i][0] = Integer.parseInt(tokens.nextToken()) + 5;
                        sizeAndLoc[i][1] = Integer.parseInt(tokens.nextToken()) + 5;
                        sizeAndLoc[i][2] = Integer.parseInt(tokens.nextToken());
                        sizeAndLoc[i][3] = Integer.parseInt(tokens.nextToken());
                    }
                }
            }
            catch (Exception e) {
                System.err.println("VCP File Read Problem");
            }
            if(!names[3].equals("none")) {
                readSlideFile(names[3],true);
                if (theMenu == null) {
                    theMenu = new CustomMenu(slideNames, slideDuration, slidesTotalSlides, this);
                    this.addMouseListener(theMenu.getListener());
                }
            }
            if(!names[5].equals("none")) {
                readBothFile(names[5],true);
                if (theMenu == null) {
                    theMenu = new CustomMenu(slideNames, bothDuration, bothTotalSlides, this);
                    this.addMouseListener(theMenu.getListener());
                }
            }
            if(!names[0].equals("none")) {
                try {
                    File tempFile = new File(names[0]);
                    videoPlayer = Manager.createRealizedPlayer(tempFile.toURL());
                    video = new Panel(null);
                    Runnable setupVideo = new Runnable() {
                        public void run() { 
                            Component playerComponent = videoPlayer.getVisualComponent();
                            if(theMenu != null)
                                playerComponent.addMouseListener(theMenu.getListener());
                            video.setLocation(sizeAndLoc[0][0], sizeAndLoc[0][1]);
                            video.setSize(sizeAndLoc[0][2], sizeAndLoc[0][3]);
                            playerComponent.setSize(sizeAndLoc[0][2], sizeAndLoc[0][3]);
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
            if(!names[1].equals("none")) {
                try {
                    File tempFile = new File(names[1]);
                    audioPlayer = Manager.createRealizedPlayer(tempFile.toURL());
                }
                catch (Exception e) {
                    System.err.println("audio setup problem");
                }
            }
            if(!names[2].equals("none")) {
                try {
                    File tempFile = new File(names[2]);
                    avPlayer = Manager.createRealizedPlayer(tempFile.toURL());
                    av = new Panel(null);
                    Runnable setupAV = new Runnable() {
                        public void run() { 
                            Component avComponent = avPlayer.getVisualComponent();
                            if(theMenu != null)
                                avComponent.addMouseListener(theMenu.getListener());
                            av.setLocation(sizeAndLoc[2][0], sizeAndLoc[2][1]);
                            av.setSize(sizeAndLoc[2][2], sizeAndLoc[2][3]);
                            avComponent.setSize(sizeAndLoc[2][2], sizeAndLoc[2][3]);
                            av.add(avComponent);
                            av.setVisible(true);
                        }
                    };
                    SwingUtilities.invokeLater(setupAV);
                    add(av);
                }
                catch (Exception e) {
                    System.err.println("av setup problem");
                }
            }
            if(!names[3].equals("none")) {
                slides = new SlideLabel(slideImageList, slideDuration, slidesTotalSlides, sizeAndLoc[3][2], sizeAndLoc [3][3], this, "slide");
                slides.setBounds(sizeAndLoc[3][0], sizeAndLoc[3][1], sizeAndLoc[3][2], sizeAndLoc[3][3]);
                slides.setBackground(new java.awt.Color(210, 210, 210));
                slides.setDoubleBuffered(true);
                slides.setOpaque(true);
                slides.setVisible(true);
                slides.validate();
                add(slides);
            }
            if(!names[4].equals("none")) {
                readWhiteboardFile(names[4],true);
                whiteboard = new SlideLabel(whiteboardImageList, whiteboardDuration, whiteboardTotalSlides, sizeAndLoc[4][2], sizeAndLoc[4][3], this, "whiteboard");
                whiteboard.setBounds(sizeAndLoc[4][0], sizeAndLoc[4][1], sizeAndLoc[4][2], sizeAndLoc[4][3]);
                whiteboard.setBackground(new java.awt.Color(210, 210, 210));
                whiteboard.setDoubleBuffered(true);
                whiteboard.setOpaque(true);
                whiteboard.setVisible(true);
                whiteboard.validate();
                add(whiteboard);
            }
            if(!names[5].equals("none")) {
                both = new SlideLabel(bothImageList, whiteboardDuration, bothTotalSlides, sizeAndLoc[5][2], sizeAndLoc [5][3], this, "both");
                both.setBounds(sizeAndLoc[5][0], sizeAndLoc[5][1], sizeAndLoc[5][2], sizeAndLoc[5][3]);
                both.setBackground(new java.awt.Color(210, 210, 210));
                both.setDoubleBuffered(true);
                both.setOpaque(true);
                both.setVisible(true);
                both.validate();
                add(both);
            }
            if(!names[6].equals("none")) {
                readCameraFile(names[6],true);
                camera = new SlideLabel(cameraImageList, cameraDuration, cameraTotalSlides, sizeAndLoc[6][2], sizeAndLoc[6][3], this, "camera");
                camera.setBounds(sizeAndLoc[6][0], sizeAndLoc[6][1], sizeAndLoc[6][2], sizeAndLoc[6][3]);
                camera.setBackground(new java.awt.Color(210, 210, 210));
                camera.setDoubleBuffered(true);
                camera.setOpaque(true);
                camera.setVisible(true);
                camera.validate();
                add(camera);
            }
            // From now on, we start to load the rest of the images in
            // Asynchronized way
            loadThread = new Thread(this);
            loadThread.start();
        }
        else
            return;
    }

    public void setWaitCursor()
    {
        parent.setCtlBtnEnabled(false);
        removeMouseListener(theMenu.getListener());
        parent.setCursor(new Cursor(Cursor.WAIT_CURSOR));
    }
    
    public void setDefaultCursor()
    {
        parent.setCtlBtnEnabled(true);
        addMouseListener(theMenu.getListener());
        parent.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
    }

    public void removeNotify() {
        if(video != null)
            remove(video);
        if(av != null)
            remove(av);
    }
    
    public void start() {
        loadThread.setDaemon(true);
        loadThread.start();
    }

    public void run() {
        parent.setCtlBtnVisible(true);
        setWaitCursor();
        
        if (!names[3].equals("none"))
            readSlideFile(names[3],false); 
        if (!names[4].equals("none"))
            readWhiteboardFile(names[4],false);
        if (!names[5].equals("none"))
            readBothFile(names[5],false);
        if (!names[6].equals("none"))
            readCameraFile(names[6],false);

        // Invisible the bar and label.
        parent.getProgressBar().setVisible(false);
        parent.getLabel().setVisible(false);
        setDefaultCursor();
        
        // Invoke the loading thread and initialize variable.
        slideCur = whiteboardCur = bothCur = cameraCur = 0;
        for (int i = 0 ; i < CIRCLE_BUFFER_SIZE; i++)
        {
            slideCircleStatus[i] = true;
            whiteboardCircleStatus[i] = true;
            bothCircleStatus[i] = true;
            cameraCircleStatus[i] = true;
        }
        readingThread = new VCP.ReadingThread();
        readingThread.start();
    }
    
    public boolean getCurStatus() {
        boolean result = true;
        
        if (!names[3].equals("none"))
            result = result && slideCircleStatus[slideCur];
        if (!names[4].equals("none"))
            result = result && whiteboardCircleStatus[whiteboardCur];
        if (!names[5].equals("none"))
            result = result && bothCircleStatus[bothCur];
        if (!names[6].equals("none"))
            result = result && cameraCircleStatus[cameraCur];
        
        return result;
    }

    public void backCurCursor() {
        if (!names[3].equals("none")) {
            slidePositionNum --;
            slideCircleStatus[(slideCur+1)%CIRCLE_BUFFER_SIZE] = false;
            slideCur = (slideCur-1+CIRCLE_BUFFER_SIZE) % CIRCLE_BUFFER_SIZE;
        }
        // For whiteboard
        if (!names[4].equals("none")) {
            int p1 = whiteboard.getPosition(theMenu.getTimePosition(slidePositionNum));
            int p2 = whiteboard.getPlace() - 1;
            if ( p1 == p2) {
                whiteboardCircleStatus[(whiteboardCur+1)%CIRCLE_BUFFER_SIZE] = false;
                whiteboardCur = (whiteboardCur-1+CIRCLE_BUFFER_SIZE) % CIRCLE_BUFFER_SIZE;
            } 
            else if ( p1 < p2) {
                for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++)
                    whiteboardCircleStatus[i] = false;
                whiteboardCur = 1;  // No use, but for good understand.
            }
        }
        // For both
        if (!names[5].equals("none")) {
            int p1 = both.getPosition(theMenu.getTimePosition(slidePositionNum));
            int p2 = both.getPlace() - 1;
            if (p1 == p2 ) {
                bothCircleStatus[(bothCur+1)%CIRCLE_BUFFER_SIZE] = false;
                bothCur = (bothCur-1+CIRCLE_BUFFER_SIZE) % CIRCLE_BUFFER_SIZE;
            } 
            else if (p1 < p2) {
                for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++)
                    bothCircleStatus[i] = false;
                bothCur = 1;  // No use, but for good understand.
            }
        }
        // For camera
        if (!names[6].equals("none")) {
            int p1 = camera.getPosition(theMenu.getTimePosition(slidePositionNum));
            int p2 = camera.getPlace() - 1;
            if ( p1 == p2) {
                cameraCircleStatus[(cameraCur+1)%CIRCLE_BUFFER_SIZE] = false;
                cameraCur = (cameraCur-1+CIRCLE_BUFFER_SIZE) % CIRCLE_BUFFER_SIZE;
            } 
            else if (p1 < p2) {
                for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++)
                    cameraCircleStatus[i] = false;
                cameraCur = 1;  // No use, but for good understand.
            }
        }
    }

    public void moveCurCursor(String ID) {
        if (ID.equals("slide")) {
            slidePositionNum ++;  // This is when the slide move forward auto.
            slideCircleStatus[(slideCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
            slideCur = (slideCur + 1) % CIRCLE_BUFFER_SIZE;
        }
        if (ID.equals("whiteboard")) {
            whiteboardCircleStatus[(whiteboardCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
            whiteboardCur = (whiteboardCur + 1) % CIRCLE_BUFFER_SIZE;
        }
        if (ID.equals("both")) {
            bothCircleStatus[(bothCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
            bothCur = (bothCur + 1) % CIRCLE_BUFFER_SIZE;
        }
        if (ID.equals("camera")) {
            cameraCircleStatus[(cameraCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
            cameraCur = (cameraCur + 1) % CIRCLE_BUFFER_SIZE;
        }
    }

    public void moveCurCursor() {
        if (!names[3].equals("none")) {
            slidePositionNum ++; // This is when the slide move forward manually.
            slideCircleStatus[(slideCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
            slideCur = (slideCur + 1) % CIRCLE_BUFFER_SIZE;
        }
        // For whiteboard
        // If the next slide is the same whiteboard, i.e., no new whiteboard image created, then, we do nothing
        // If the next slide is also the next whiteboard, we move the whiteboard to next one.
        // If the next slide is more than 1 whiteboard moved, we reload it. the same to others.
        if (!names[4].equals("none")) {
            int p1 = whiteboard.getPosition(theMenu.getTimePosition(slidePositionNum));
            int p2 = whiteboard.getPlace() + 1;
            if ( p1 == p2 ) {
                whiteboardCircleStatus[(whiteboardCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
                whiteboardCur = (whiteboardCur + 1) % CIRCLE_BUFFER_SIZE;
            } 
            else if (p1 > p2) {
                for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++)
                    whiteboardCircleStatus[i] = false;
                whiteboardCur = 1;  // No use, but for good understand.
            }
        }
        // For both
        if (!names[5].equals("none")) {
            int p1 = both.getPosition(theMenu.getTimePosition(slidePositionNum));
            int p2 = both.getPlace() + 1;
            if ( p1 == p2) {
                bothCircleStatus[(bothCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
                bothCur = (bothCur + 1) % CIRCLE_BUFFER_SIZE;
            } 
            else if (p1 > p2) {
                for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++)
                    bothCircleStatus[i] = false;
                bothCur = 1;  // No use, but for good understand.
            }
        }
        // For camera
        if (!names[6].equals("none")) {
            int p1 = camera.getPosition(theMenu.getTimePosition(slidePositionNum));
            int p2 = camera.getPlace() + 1;
            if (p1 == p2) {
                cameraCircleStatus[(cameraCur-1+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE] = false;
                cameraCur = (cameraCur + 1) % CIRCLE_BUFFER_SIZE;
            } 
            else if ( p1 > p2 ) {
                for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++)
                    cameraCircleStatus[i] = false;
                cameraCur = 1;  // No use, but for good understand.
            }
        }
    }
    
    public int getCurCursor(String ID) {
        if (ID.equals("slide"))
            return slideCur;
        if (ID.equals("whiteboard"))
            return whiteboardCur;
        if (ID.equals("both"))
            return bothCur;
        if (ID.equals("camera"))
            return cameraCur;
        
        return slideCur;
    }

    public void setSlidePosition(int place) {
        slidePositionNum = place;
    }

    public Vector getSlideFullNames() {
        return slideFullNames;
    }

    public VCPlayer getParentHandler() {
        return parent;
    }
    
    public void clearLoaded (int pos) {
        for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++)
        {
            slideCircleStatus[i] = false;
            whiteboardCircleStatus[i] = false;
            bothCircleStatus[i] = false;
            cameraCircleStatus[i] = false;
        }
        slideCur = whiteboardCur = bothCur = cameraCur = 0;
        
        clearNumber = pos;
    }

    public void waitForCurCursorReady()
    {
        setWaitCursor();
        while (!getCurStatus()) {
            try { 
                System.out.println("Wait for 10 msec!");
                Thread.sleep(10);
            } catch (Exception e) { 
                JOptionPane.showMessageDialog(this,"WaitForCurCursorReady" + e.getMessage(),"Exception Catched:", JOptionPane.ERROR_MESSAGE);
            }
        }
        setDefaultCursor();

    }
    
    // Define a loading class for loading the MM materials.
    public class ReadingThread extends Thread{
        private boolean stop = false;
        
        public void Stop() {
            stop = true;
        }
        
        public void setList(boolean circleStatus[], String cd,Vector fileNameList, Vector imageList, int i, int pos, int cursor) {
            ImageIcon theImage = new ImageIcon(cd + "\\" + fileNameList.elementAt(pos));
            theImage.setImage(theImage.getImage().getScaledInstance(sizeAndLoc[i][2],sizeAndLoc[i][3],Image.SCALE_SMOOTH));
            imageList.set(cursor,theImage);
            circleStatus[cursor] = true;
        }
        
        public void loadOneColumn(int pos, int direction) {
            if (!names[3].equals("none") && !slideCircleStatus[(slideCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE])   
                setList(slideCircleStatus, slideCD, slideFileNameList, slideImageList, 3, pos, (slideCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE); 
            if (!names[4].equals("none") && !whiteboardCircleStatus[(whiteboardCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE])  
                setList(whiteboardCircleStatus, whiteboardCD, whiteboardFileNameList, whiteboardImageList, 4, whiteboard.getPosition(theMenu.getTimePosition(pos)), (whiteboardCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE); 
            if (!names[5].equals("none") && !bothCircleStatus[(bothCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE])   
                setList(bothCircleStatus, bothCD, bothFileNameList, bothImageList, 5, both.getPosition(theMenu.getTimePosition(pos)), (bothCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE);  
            if (!names[6].equals("none") && !cameraCircleStatus[(cameraCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE])   
                setList(cameraCircleStatus, cameraCD, cameraFileNameList, cameraImageList, 6, camera.getPosition(theMenu.getTimePosition(pos)), (cameraCur+direction+CIRCLE_BUFFER_SIZE)%CIRCLE_BUFFER_SIZE); 

        }
        
        public boolean currentLoaded() {
            if (!names[3].equals("none") && !slideCircleStatus[slideCur])
                return false;
            if (!names[4].equals("none") && !whiteboardCircleStatus[whiteboardCur])
                return false;
            if (!names[5].equals("none") && !bothCircleStatus[bothCur])
                return false;
            if (!names[6].equals("none") && !cameraCircleStatus[cameraCur])
                return false;
                
            return true;
        }
        
        public boolean forwardLoaded() {
            if (slidePositionNum == slidesTotalSlides - 1) return true;  // Don't load if it is the first page.

            if (!names[3].equals("none") && !slideCircleStatus[(slideCur+1)%CIRCLE_BUFFER_SIZE])
                return false;
            if (!names[4].equals("none") && !whiteboardCircleStatus[(whiteboardCur+1)%CIRCLE_BUFFER_SIZE])
                return false;
            if (!names[5].equals("none") && !bothCircleStatus[(bothCur+1)%CIRCLE_BUFFER_SIZE])
                return false;
            if (!names[6].equals("none") && !cameraCircleStatus[(cameraCur+1)%CIRCLE_BUFFER_SIZE])
                return false;
                
            return true;
        }
        
        public boolean backwardLoaded() {
            if (slidePositionNum == 0) return true;  // Don't load if it is the first page.
            
            if (!names[3].equals("none") && !slideCircleStatus[(slideCur-1+CIRCLE_BUFFER_SIZE )%CIRCLE_BUFFER_SIZE])
                return false;
            if (!names[4].equals("none") && !whiteboardCircleStatus[(whiteboardCur-1+CIRCLE_BUFFER_SIZE )%CIRCLE_BUFFER_SIZE])
                return false;
            if (!names[5].equals("none") && !bothCircleStatus[(bothCur-1+CIRCLE_BUFFER_SIZE )%CIRCLE_BUFFER_SIZE])
                return false;
            if (!names[6].equals("none") && !cameraCircleStatus[(cameraCur-1+CIRCLE_BUFFER_SIZE )%CIRCLE_BUFFER_SIZE])
                return false;
                
            return true;
        }

        public void run() {
            
            while (!stop)
            {
                if (slidePositionNum < 0 || slidePositionNum >= slidesTotalSlides)
                {
                    // Something wrong!
                    try {
                        Thread.sleep(100);
                        continue;
                    } catch (Exception e) { System.out.println("Slide number is out of range!");};
                }
                    
                if (clearNumber != VOID) {
                    slidePositionNum = clearNumber;
                    clearNumber = VOID;
                    loadOneColumn(slidePositionNum, 0);
                    System.out.println("Clear Finished loading No. " + slidePositionNum);
                }
                else if (!currentLoaded()) { 
                    //We need to load new image frame current.
                    System.out.println("Current Finished loading No. " + slidePositionNum);
                    loadOneColumn(slidePositionNum, 0);
                } 
                else if (!forwardLoaded()) { 
                    //We need to load new image frame forward.
                    System.out.println("Forward Finished loading No. " + (slidePositionNum+1));
                    if (slidePositionNum + 1 < slidesTotalSlides)
                        loadOneColumn(slidePositionNum + 1, 1);
                } 
                else if (!backwardLoaded()) { 
                    // This means that the curCursor has moved on
                    // So we need to load a new image frame backward.
                    System.out.println("Backword Finished loading No. " + (slidePositionNum-1));
                    if (slidePositionNum > 0)
                        loadOneColumn(slidePositionNum - 1, -1);
                } 
                else {
                    // We don't need to do anything, so sleep.
                    try {
                        Thread.sleep(10);
                    } catch (Exception e) {};
                }
            }// end of while
            
        }
    } // End of the ReadingThread class
}

/* This is a new program structure based on the old one. 
 *
 * First, we defined circle buffer size as 3 to store the temporary images. For each
 * multimedia material, e.g. slide, whiteboard, we have a *FileNameList to store the 
 * filename of each frame instead of loading them into memory at the beginning.
 * Second, we have a global variable slidePositionNum, which indicates the current
 * slide number (minus - 1), for each material, we have a pointer, *Cur point to the 
 * current location of the corresponding Vector. Also, we have a boolean array with
 * size of the circle buffer size for each material. This array is used to indicate
 * if the current, the next , or the previous buffer is loaded for showing up.
 * Third, we add a loaded thread, readingThread to load new image if needed while 
 * the user is playbacking the materials. Several cases:
 *               1. If the user redirect the position, i.e., choose a slide from the
 * right click menu, we clear all the status for all the materials boolean array, and
 * wait the readingThread to reloading them
 *               2. If the user click fwd button, or rew button, we need to set the 
 * corresponding boolean status. and let the readingThread to load the necessary image.
 *               3. While any of the material need to change automatically, i.e., loading 
 * next image, we set the boolean status for this particular material boolean array, keep
 * it consistant.
 */ 