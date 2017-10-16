/*
 * VCC.java
 *
 * This serves as the main presentation creation window.  It automatically 
 * starts with the controller internal frame, which contains checkboxes that 
 * are used to add or remove components to the current presentation.  It can 
 * open a previous presentation for editing or be used to create a new 
 * presentation.  The start, stop, and pause buttons are automatically setup 
 * also.  While creating a presentation you must direct the creator to specific 
 * files in order for the program to compile the correct information.  You can 
 * also resize and move any of the components to make the best visual 
 * presentation.  Once a presentation is finished it can be saved to a user 
 * named .vcp file which can be opened with the VCP module.
 *
 */

package VCPlayer;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.*;
import javax.media.*;


public class VCC extends javax.swing.JPanel {
    private VCPlayer parent;
    private JInternalFrame theFrame, videoFrame, audioFrame, avFrame, slidesFrame,
            whiteboardFrame, bothFrame, cameraFrame;
    private JCheckBox video, audio, av, slides, whiteboard, both, camera;
    private JLayeredPane thePane;
    private String videoName, audioName, avName, slidesName, whiteboardName, bothName, cameraName;
    private JFileChooser chooser;
    private Player thePlayer;
    private ImageIcon slidesImage, whiteboardImage, bothImage, cameraImage;
    private JLabel slidesLabel, whiteboardLabel, bothLabel, cameraLabel;
    private Image slidesBigImage, whiteboardBigImage, bothBigImage, cameraBigImage;
    private String filename;
    private String currentVCCDirectory = null;
    private int totalBothPages = 0;
    
    /** Creates a new instance of VCC */
    public VCC(VCPlayer pt, JFileChooser ch) {
        super();
        this.setBackground(new java.awt.Color(224, 224, 224));
        //this.setBounds(0,0,895,710);
		setBounds(0,0,1280,1024);
        this.setLayout(null);
        parent = pt;
        chooser = ch;
        currentVCCDirectory = ch.getCurrentDirectory().getPath();
        theFrame = new JInternalFrame("Click to Add",false,false,false,false);
        theFrame.setLocation(737,0);
        theFrame.setSize(150,380);
        setupFrame();
        theFrame.setVisible(true);
        thePane = parent.getLayeredPane();
        thePane.add(theFrame,JLayeredPane.POPUP_LAYER);
    }
    
    public String getCD() {
        return currentVCCDirectory;
    }
    
    private void setupFrame() {
        video = new JCheckBox("Video Panel");
        video.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                videoActionPerformed(evt);
            }
        });
        audio = new JCheckBox("Audio File");
        audio.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                audioActionPerformed(evt);
            }
        });
        av = new JCheckBox("Video with Audio");
        av.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                avActionPerformed(evt);
            }
        });
        slides = new JCheckBox("Slides Panel");
        slides.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                slidesActionPerformed(evt);
            }
        });
        whiteboard = new JCheckBox("Whiteboard Panel");
        whiteboard.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                whiteboardActionPerformed(evt);
            }
        });
        both = new JCheckBox("Slides with Board");
        both.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                bothActionPerformed(evt);
            }
        });
        camera = new JCheckBox("Camera Frames");
        camera.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cameraActionPerformed(evt);
            }
        });
        theFrame.getContentPane().setLayout(new FlowLayout(FlowLayout.LEFT,5,20));
        theFrame.getContentPane().add(video);
        theFrame.getContentPane().add(audio);
        theFrame.getContentPane().add(av);
        theFrame.getContentPane().add(slides);
        theFrame.getContentPane().add(whiteboard);
        theFrame.getContentPane().add(both);
        theFrame.getContentPane().add(camera);
    }
    
    public void removeNotify() {
        super.removeNotify();
        thePane.remove(theFrame);
        if(videoFrame != null)
            thePane.remove(videoFrame);
        if(audioFrame != null)
            thePane.remove(audioFrame);
        if(avFrame != null)
            thePane.remove(avFrame);
        if(slidesFrame != null)
            thePane.remove(slidesFrame);
        if(whiteboardFrame != null)
            thePane.remove(whiteboardFrame);
        if(bothFrame != null)
            thePane.remove(bothFrame);
        if(cameraFrame != null)
            thePane.remove(cameraFrame);
        thePane = null;
    }
    
    private void videoActionPerformed(java.awt.event.ActionEvent evt) {
        if(video.isSelected()) {
            chooser.setDialogTitle("Click on the Video File");
            int returnVal = chooser.showOpenDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                videoName = chooser.getSelectedFile().getPath();
                setupVideoFrame(20,20,300,250);
            }
            else
                video.getModel().setSelected(false);
        }
        else {
            thePane.remove(videoFrame);
            thePane.repaint();
            videoFrame = null;
        }
    }
    
    public void setupVideoFrame(int locX, int locY, int sizeX, int sizeY) {
        videoFrame = new JInternalFrame("Video",true,false,false,false);
        videoFrame.setLocation(locX,locY);
        videoFrame.setSize(sizeX,sizeY);
        try {
            File tempp = new File(videoName);
            URL temp2 = tempp.toURL();
            thePlayer = Manager.createRealizedPlayer(temp2);
            Runnable setupVideo = new Runnable() {
                public void run() { 
                    videoFrame.getContentPane().add(thePlayer.getVisualComponent());
                    videoFrame.getContentPane().validate();  
                }
            };
            SwingUtilities.invokeLater(setupVideo);
        }
        catch (Exception e) {
            System.out.println("Video Problem");
        }
        videoFrame.setVisible(true);
        thePane.add(videoFrame,JLayeredPane.DEFAULT_LAYER);
    }
    
    
    private void audioActionPerformed(java.awt.event.ActionEvent evt) {
        if(audio.isSelected()) {
            chooser.setDialogTitle("Click on the Audio File");
            ExampleFileFilter filter = new ExampleFileFilter();
            filter.addExtension("wav");
            filter.addExtension("mp3");
            filter.setDescription("Audio");
            chooser.setFileFilter(filter);            
            int returnVal = chooser.showOpenDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                audioName = chooser.getSelectedFile().getPath();
                setupAudioFrame(20,200,200,50);
            }
            else
                audio.getModel().setSelected(false);
        }
        else {
            thePane.remove(audioFrame);
            thePane.repaint();
            audioFrame = null;
        }
    }
    
    public void setupAudioFrame(int locX, int locY, int sizeX, int sizeY) {
        audioFrame = new JInternalFrame("Audio",true,false,false,false);
        audioFrame.setLocation(locX,locY);
        audioFrame.setSize(sizeX,sizeY);
        audioFrame.setVisible(true);
        thePane.add(audioFrame,JLayeredPane.DEFAULT_LAYER);
    }
    
    private void avActionPerformed(java.awt.event.ActionEvent evt) {
        if(av.isSelected()) {
            chooser.setDialogTitle("Click on the Audio/Video File");
            int returnVal = chooser.showOpenDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                avName = chooser.getSelectedFile().getPath();
                setupAVFrame(20,20,300,250);
            }
            else
                av.getModel().setSelected(false);
        }
        else {
            thePane.remove(avFrame);
            thePane.repaint();
            avFrame = null;
        }
    }
    
    public void setupAVFrame(int locX, int locY, int sizeX, int sizeY) {
        avFrame = new JInternalFrame("Audio and Video",true,false,false,false);
        avFrame.setLocation(locX,locY);
        avFrame.setSize(sizeX,sizeY);
        try {
            File tempp = new File(avName);
            URL temp2 = tempp.toURL();
            thePlayer = Manager.createRealizedPlayer(temp2);
            Runnable setupVideo = new Runnable() {
                public void run() { 
                    avFrame.getContentPane().add(thePlayer.getVisualComponent());
                    avFrame.getContentPane().validate();  
                }
            };
            SwingUtilities.invokeLater(setupVideo);
        }
        catch (Exception e) {
            System.out.println("Video Problem");
        }
        avFrame.setVisible(true);
        thePane.add(avFrame,JLayeredPane.DEFAULT_LAYER);
    }
    
    private void slidesActionPerformed(java.awt.event.ActionEvent evt) {
        if(slides.isSelected()) {
            chooser.setDialogTitle("Click on the Timing File");
            ExampleFileFilter filter = new ExampleFileFilter();
            filter.addExtension("txt");
            filter.setDescription("TXT ");
            chooser.setFileFilter(filter);             
            int returnVal = chooser.showOpenDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                slidesName = chooser.getSelectedFile().getPath();
                setupSlidesFrame(20,20,300,250);
                /* We don't need this                 
                if(whiteboard.isSelected()) {
                    SyncDialog temp = new SyncDialog(parent,"Select Type of Synchronization",true);
                    temp.show();
                }
                 */
            }
            else
                slides.getModel().setSelected(false);
        }
        else {
            thePane.remove(slidesFrame);
            thePane.repaint();
            slidesFrame = null;
        }
    }
    
    public void setupSlidesFrame(int locX, int locY, int sizeX, int sizeY) {
        BufferedReader input = null;
        try {
            File theFile = new File(slidesName);
            input = new BufferedReader(new InputStreamReader(new FileInputStream(theFile)));
            input.readLine();
            input.readLine();
            input.readLine();
            input.readLine();
            input.readLine();
            StringTokenizer tokens = new StringTokenizer(input.readLine());
            tokens.nextToken();
            tokens.nextToken();
            tokens.nextToken();
            String flname = tokens.nextToken();
            slidesImage = new ImageIcon(theFile.getParent() + "\\" + flname);
        }
        catch (Exception e) {
            System.err.println("IO Problem");
        }
        slidesFrame = new JInternalFrame("Slides",true,false,false,false);
        slidesFrame.addMouseListener(new SlidesListener());
        slidesFrame.setLocation(locX,locY);
        slidesFrame.setSize(sizeX,sizeY);
        slidesBigImage = slidesImage.getImage();
        slidesImage.setImage(slidesImage.getImage().getScaledInstance(sizeX,sizeY,Image.SCALE_SMOOTH));
        slidesLabel = new JLabel(slidesImage);
        slidesFrame.getContentPane().add(slidesLabel);
        slidesFrame.getContentPane().validate();
        slidesFrame.setVisible(true);
        thePane.add(slidesFrame,JLayeredPane.DEFAULT_LAYER);
    }
    
    private void whiteboardActionPerformed(java.awt.event.ActionEvent evt) {
        if(whiteboard.isSelected()) {
            chooser.setDialogTitle("Click on the Index HTML file");
            ExampleFileFilter filter = new ExampleFileFilter();
            filter.addExtension("html");
            filter.setDescription("HTML ");
            chooser.setFileFilter(filter);             
            int returnVal = chooser.showOpenDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                whiteboardName = chooser.getCurrentDirectory().getPath();
                setupWhiteboardFrame(20,20,300,250);
                /* We don't need this                 
                if(slides.isSelected()) {
                    SyncDialog temp = new SyncDialog(parent,"Select Type of Synchronization",true);
                    temp.show();
                }
                 */
            }
            else
                whiteboard.getModel().setSelected(false);
        }
        else {
            thePane.remove(whiteboardFrame);
            thePane.repaint();
            whiteboardFrame = null;
        }
    }
    
    public void setupWhiteboardFrame(int locX, int locY, int sizeX, int sizeY) {
        whiteboardFrame = new JInternalFrame("Whiteboard",true,false,false,false);
        whiteboardFrame.addMouseListener(new WhiteboardListener());
        whiteboardFrame.setLocation(locX,locY);
        whiteboardFrame.setSize(sizeX,sizeY);
        whiteboardImage = new ImageIcon(whiteboardName + "\\" + "img000.jpg");
        whiteboardBigImage = whiteboardImage.getImage();
        whiteboardImage.setImage(whiteboardImage.getImage().getScaledInstance(sizeX,sizeY,Image.SCALE_SMOOTH));
        whiteboardLabel = new JLabel(whiteboardImage);
        whiteboardFrame.getContentPane().add(whiteboardLabel);
        whiteboardFrame.getContentPane().validate();
        getWhiteboardTimes();
        whiteboardFrame.setVisible(true);
        thePane.add(whiteboardFrame,JLayeredPane.DEFAULT_LAYER);
    }
    
    private void bothActionPerformed(java.awt.event.ActionEvent evt) {
        if(both.isSelected()) {
            chooser.setDialogTitle("Click on the Index HTML file, the same as whiteboard");
            ExampleFileFilter filter = new ExampleFileFilter();
            filter.addExtension("html");
            filter.setDescription("HTML");
            chooser.setFileFilter(filter);             
            int returnVal = chooser.showOpenDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                bothName = chooser.getSelectedFile().getPath();
                String currentDirectory = chooser.getCurrentDirectory().getPath();
                int i = 0;
                File temp = new File(currentDirectory + "\\" + i + ".jpg");
                while(temp.exists()) {
                    i++;
                    temp = new File(currentDirectory + "\\" + i + ".jpg");
                }
                
                if (i == 0)
                    JOptionPane.showMessageDialog(this,"Please make sure that the aligned images \n are located in the same folder as the whiteboard images \n and start with 0.jpg","File can't be found!", JOptionPane.ERROR_MESSAGE);
                else {
                    totalBothPages = ++i; 
                    saveBothNumber();  // By Liwh: 9/27/04
                    setupBothFrame(20,20,300,250);
                }
            }
            else
                both.getModel().setSelected(false);
        }
        else {
            thePane.remove(bothFrame);
            thePane.repaint();
            bothFrame = null;
        }
    }
    
    public void setupBothFrame(int locX, int locY, int sizeX, int sizeY) {
        bothFrame = new JInternalFrame("Slides with Whiteboard",true,false,false,false);
        bothFrame.addMouseListener(new BothListener());
        bothFrame.setLocation(locX,locY);
        bothFrame.setSize(sizeX,sizeY);
        File temp = new File(bothName);
        bothImage = new ImageIcon(temp.getParent() + "//0.jpg");
        bothBigImage = bothImage.getImage();
        bothImage.setImage(bothImage.getImage().getScaledInstance(sizeX,sizeY,Image.SCALE_SMOOTH));
        bothLabel = new JLabel(bothImage);
        bothFrame.getContentPane().add(bothLabel);
        bothFrame.getContentPane().validate();
        bothFrame.setVisible(true);
        thePane.add(bothFrame,JLayeredPane.DEFAULT_LAYER);
    }
    
    private void cameraActionPerformed(java.awt.event.ActionEvent evt) {
        if(camera.isSelected()) {
            chooser.setDialogTitle("Click on the Camera Timing File");
            ExampleFileFilter filter = new ExampleFileFilter();
            filter.addExtension("txt");
            filter.setDescription("TXT ");
            chooser.setFileFilter(filter);             
            int returnVal = chooser.showOpenDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                cameraName = chooser.getSelectedFile().getPath();
                setupCameraFrame(20,20,300,250);
                /* We don't need this 
                if(whiteboard.isSelected()) {
                    SyncDialog temp = new SyncDialog(parent,"Select Type of Synchronization",true);
                    temp.show();
                }
                 */
            }
            else
                camera.getModel().setSelected(false);
        }
        else {
            thePane.remove(cameraFrame);
            thePane.repaint();
            cameraFrame = null;
        }
    }
    
    public void setupCameraFrame(int locX, int locY, int sizeX, int sizeY) {
        cameraFrame = new JInternalFrame("Camera Frames",true,false,false,false);
        cameraFrame.addMouseListener(new CameraListener());
        cameraFrame.setLocation(locX,locY);
        cameraFrame.setSize(sizeX,sizeY);
        File temp = new File(cameraName);
        cameraImage = new ImageIcon(temp.getParent() + "/1.jpg");
        cameraBigImage = cameraImage.getImage();
        cameraImage.setImage(cameraImage.getImage().getScaledInstance(sizeX,sizeY,Image.SCALE_SMOOTH));
        cameraLabel = new JLabel(cameraImage);
        cameraFrame.getContentPane().add(cameraLabel);
        cameraFrame.getContentPane().validate();
        cameraFrame.setVisible(true);
        thePane.add(cameraFrame,JLayeredPane.DEFAULT_LAYER);
    }
    
    public void save() {
        PrintWriter outputStream;
        if(filename == null) {
            ExampleFileFilter filter = new ExampleFileFilter();
            filter.addExtension("vcp");
            filter.setDescription("VCP ");
            chooser.setFileFilter(filter);                         
            chooser.setDialogTitle("Save File");
            File selectedFile = new File("Untitled");
            chooser.setSelectedFile(selectedFile);
            chooser.rescanCurrentDirectory();
            int returnVal = chooser.showSaveDialog(this);
            if(returnVal == JFileChooser.APPROVE_OPTION) {
                currentVCCDirectory = chooser.getCurrentDirectory().getPath();
                filename = chooser.getSelectedFile().getPath();
                if(filename.indexOf(".vcp") == -1)
                    filename += ".vcp";
            }
            else
                return;
        }
        try {
            outputStream = new PrintWriter(new FileOutputStream(filename));
            if(videoFrame != null) {
                outputStream.println(videoName);
                outputStream.println(videoFrame.getLocation().x + " " + videoFrame.getLocation().y + " " + videoFrame.getContentPane().getWidth() + " " + videoFrame.getContentPane().getHeight());
            }
            else
                outputStream.println("none");
            if(audioFrame != null) {
                outputStream.println(audioName);
                outputStream.println(audioFrame.getLocation().x + " " + audioFrame.getLocation().y + " " + audioFrame.getContentPane().getWidth() + " " + audioFrame.getContentPane().getHeight());
            }
            else
                outputStream.println("none");
            if(avFrame != null) {
                outputStream.println(avName);
                outputStream.println(avFrame.getLocation().x + " " + avFrame.getLocation().y + " " + avFrame.getContentPane().getWidth() + " " + avFrame.getContentPane().getHeight());
            }
            else
                outputStream.println("none");
            if(slidesFrame != null) {
                outputStream.println(slidesName);
                outputStream.println(slidesFrame.getLocation().x + " " + slidesFrame.getLocation().y + " " + slidesFrame.getContentPane().getWidth() + " " + slidesFrame.getContentPane().getHeight());
            }
            else
                outputStream.println("none");
            if(whiteboardFrame != null) {
                outputStream.println(whiteboardName);
                outputStream.println(whiteboardFrame.getLocation().x + " " + whiteboardFrame.getLocation().y + " " + whiteboardFrame.getContentPane().getWidth() + " " + whiteboardFrame.getContentPane().getHeight());
            }
            else
                outputStream.println("none");
            if(bothFrame != null) {
                outputStream.println(bothName);
                outputStream.println(bothFrame.getLocation().x + " " + bothFrame.getLocation().y + " " + bothFrame.getContentPane().getWidth() + " " + bothFrame.getContentPane().getHeight());
            }
            else
                outputStream.println("none");
            if(cameraFrame != null) {
                outputStream.println(cameraName);
                outputStream.println(cameraFrame.getLocation().x + " " + cameraFrame.getLocation().y + " " + cameraFrame.getContentPane().getWidth() + " " + cameraFrame.getContentPane().getHeight());
            }
            else
                outputStream.println("none");
            outputStream.close();
        }
        catch(FileNotFoundException e) {System.out.println("file error");}
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
            currentVCCDirectory = chooser.getCurrentDirectory().getPath();
            String temp;
            try {
                input = new BufferedReader ( new InputStreamReader ((new FileInputStream(chooser.getSelectedFile()))));
                StringTokenizer tokens;
                temp = input.readLine();
                if(!temp.equals("none")) {
                    videoName = temp;
                    tokens = new StringTokenizer(input.readLine());
                    setupVideoFrame(Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()) + 10, Integer.parseInt(tokens.nextToken()) + 33);
                    video.getModel().setSelected(true);
                }
                temp = input.readLine();
                if(!temp.equals("none")) {
                    audioName = temp;
                    tokens = new StringTokenizer(input.readLine());
                    setupAudioFrame(Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()) + 10, Integer.parseInt(tokens.nextToken()) + 33);
                    audio.getModel().setSelected(true);
                }
                temp = input.readLine();
                if(!temp.equals("none")) {
                    avName = temp;
                    tokens = new StringTokenizer(input.readLine());
                    setupAVFrame(Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()) + 10, Integer.parseInt(tokens.nextToken()) + 33);
                    av.getModel().setSelected(true);
                }
                temp = input.readLine();
                if(!temp.equals("none")) {
                    slidesName = temp;
                    tokens = new StringTokenizer(input.readLine());
                    setupSlidesFrame(Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()) + 10, Integer.parseInt(tokens.nextToken()) + 33);
                    slides.getModel().setSelected(true);
                }
                temp = input.readLine();
                if(!temp.equals("none")) {
                    whiteboardName = temp;
                    tokens = new StringTokenizer(input.readLine());
                    setupWhiteboardFrame(Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()) + 10, Integer.parseInt(tokens.nextToken()) + 33);
                    whiteboard.getModel().setSelected(true);
                }
                temp = input.readLine();
                if(!temp.equals("none")) {
                    bothName = temp;
                    tokens = new StringTokenizer(input.readLine());
                    setupBothFrame(Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()) + 10, Integer.parseInt(tokens.nextToken()) + 33);
                    both.getModel().setSelected(true);
                }
                temp = input.readLine();
                if(!temp.equals("none")) {
                    cameraName = temp;
                    tokens = new StringTokenizer(input.readLine());
                    setupCameraFrame(Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()), Integer.parseInt(tokens.nextToken()) + 10, Integer.parseInt(tokens.nextToken()) + 33);
                    camera.getModel().setSelected(true);
                }
                input.close();
            }
            catch (Exception e) {
                System.err.println("IO Problem");
            }
        }
        else
            return;
    }
    
    
    
    public class SlidesListener extends MouseAdapter {
        public void mouseReleased(MouseEvent e) {
            if(e.getButton() == MouseEvent.BUTTON1) {
                slidesImage.setImage(slidesBigImage.getScaledInstance(slidesFrame.getContentPane().getWidth(), slidesFrame.getContentPane().getHeight(), Image.SCALE_SMOOTH));
                slidesLabel.setIcon(slidesImage);
                slidesFrame.validate();
            }
        }
    }
    
    public class WhiteboardListener extends MouseAdapter {
        public void mouseReleased(MouseEvent e) {
            if(e.getButton() == MouseEvent.BUTTON1) {
                whiteboardImage.setImage(whiteboardBigImage.getScaledInstance(whiteboardFrame.getContentPane().getWidth(), whiteboardFrame.getContentPane().getHeight(), Image.SCALE_SMOOTH));
                whiteboardLabel.setIcon(whiteboardImage);
                whiteboardFrame.validate();
            }
        }
    }
    
    public class BothListener extends MouseAdapter {
        public void mouseReleased(MouseEvent e) {
            if(e.getButton() == MouseEvent.BUTTON1) {
                bothImage.setImage(bothBigImage.getScaledInstance(bothFrame.getContentPane().getWidth(), bothFrame.getContentPane().getHeight(), Image.SCALE_SMOOTH));
                bothLabel.setIcon(bothImage);
                bothFrame.validate();
            }
        }
    }

    public class CameraListener extends MouseAdapter {
        public void mouseReleased(MouseEvent e) {
            if(e.getButton() == MouseEvent.BUTTON1) {
                cameraImage.setImage(cameraBigImage.getScaledInstance(cameraFrame.getContentPane().getWidth(), cameraFrame.getContentPane().getHeight(), Image.SCALE_SMOOTH));
                cameraLabel.setIcon(cameraImage);
                cameraFrame.validate();
            }
        }
    }
    
    private void saveBothNumber() {
        PrintWriter output = null;
        try {
            File tempFile = new File(bothName);
            output = new PrintWriter(new FileOutputStream(tempFile.getParent() + "\\" + "both.txt"));
            output.println(totalBothPages);
            output.close();
        }
        catch(Exception e){System.out.println(e);}
    }
    
    private void getWhiteboardTimes() {
        int ones = 0, tens = 0, hundreds = 0;
        BufferedReader input = null;
        PrintWriter output = null;
        String temp;
        try {
            input = new BufferedReader ( new InputStreamReader ((new FileInputStream(whiteboardName + "\\" + "page000.html"))));
            output = new PrintWriter(new FileOutputStream(whiteboardName + "\\" + "whiteboard.txt"));
            temp = input.readLine();
            while(temp.indexOf("1 of") == -1) {
                temp = input.readLine();
            }
            StringTokenizer tokens = new StringTokenizer(temp);
            tokens.nextToken("f");
            int total = Integer.parseInt(tokens.nextToken("f <"));
            input.close();
            for(int i = 0; i < total; i++) {
                input = new BufferedReader ( new InputStreamReader ((new FileInputStream(whiteboardName + "\\" + "page" + hundreds + tens + ones + ".html"))));
                temp = input.readLine();
                while(temp.indexOf("STRONG") == -1) {
                    temp = input.readLine();
                }
                tokens = new StringTokenizer(temp);
                tokens.nextToken(" ");
                tokens.nextToken(" ");
                int hrs = 3600000*Integer.parseInt(tokens.nextToken(" :"));
                int min = 60000*Integer.parseInt(tokens.nextToken(" :"));
                int sec = 1000*Integer.parseInt(tokens.nextToken(" :"));
                if(tokens.nextToken(" <").equals("PM"))
                    hrs += 12*3600000;
                int totalTime = hrs + min + sec;
                output.println(totalTime);
                input.close();
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
            }
            output.close();
        }
        catch (Exception e) {
            System.out.println("getWhiteboardTimes problem");
        }
    }
}
