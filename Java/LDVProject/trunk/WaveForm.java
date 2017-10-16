// This revision is for computing segmental SNR only.
// To get SNR:
// 1. First play a relative clean clip, pay attention to the threshods.
// 2. Change the threshod if needed, then recompile it.
// 3. Reset the LDV, check "Use Existed Speech" and load a noise clips.
// 4. Pay attention to the output value of SNR. 

/*
 * WaveForm.java
 *
 * Created on July 12, 2004, 5:22 PM
 *
 * @author  weihong
 * data[] contains the original data
 */

/* This part is for comments of the program 
 * 1. The sound like "ZZZZ...." is caused by the adaptive volume. 
 * 2. The sparke signal is caused by the SHORT type case. So, let gloabYM devided by 2.
 * 3. The larger size of Low pass window, the lower frequency energy we have.
 * 4. The median filter seems no use at this moment.  // by Liwh:10.10.04
 */

package LDVProject;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.util.regex.*;
import java.io.*;
import java.nio.*;
import java.net.*;
import javax.media.*;
import javax.swing.border.*;
import javax.sound.sampled.*;
import java.awt.geom.*;

class WaveForm extends javax.swing.JPanel implements Runnable{
    LDVProject parent;
    String selectedFileName = null;
    String currentDirectory = null;
    Thread realtimeThread = null;
    JPanel ctlPnl;
	JButton timeBtn;
    JButton stepBtn;
    JButton runBtn;
    JButton pauseBtn;
    JButton resetBtn;
    JButton dspBtn;
    JButton micBtn;
    JPanel pan0, pan1, pan2, pan3, pan4, pan5, pan6, pan7, pan8;
    JComboBox stepList;
    JComboBox highPassList, lowPassList;
    JComboBox volumeList;
    JCheckBox[] dispWinBox = new JCheckBox[3];
	JCheckBox[] dispFunBox = new JCheckBox[3];
    JRadioButton[] volumeRBs = new JRadioButton[2];
    JRadioButton[] noiseRemovalRBs = new JRadioButton[2];
    JRadioButton[] gaussianFilterRBs = new JRadioButton[2];
    final ButtonGroup volumeGroup = new ButtonGroup();
    final ButtonGroup noiseRemovalGroup = new ButtonGroup();
    final ButtonGroup gaussianFilterGroup = new ButtonGroup();
    JPanel canvasPanel;
    Canvas [] canvases;
	short colorArray[] = null;
	boolean firstFrame = true;
    boolean pause = false;
    boolean reset = true;
    boolean stopCapture = false;
    boolean notUsingMic = true;
    boolean isWaveFormat = true;
    boolean useGlobalScale = true;
    boolean usedBScale = true;
    boolean useGaussian = true;
    boolean isNoiseRemoval = true;
    boolean isAdaptiveVol = true;
    boolean needBeSaved = false;
    boolean useMedianFilter = false;
	boolean useExSpeech = false;
    boolean usePaperColor = false;              // Convenient color for paper figure
    boolean displayWindow[] = new boolean [3];
    boolean autoDrawFullWindow = false;         // Signal for drawing a full window.
    boolean isSpeechFrame = false; // used to define speech frame.
    
    
    int dataBufLen = 1024 ; 	// Data Buffer Size : 1024 /4;
    int G_WIN = 256;  			// Gaussian Window Size(256) , must be larger if the L_WIN is small.
    int L_WIN = 16;   			// Low-pass Window Size(16), the larger, the lower freq. energy: based on G= 256
    int M_WIN = 8;   			// Median-filter Window Size
    int HIGH_FREQ = 300;
    int LOW_FREQ = 3000;
    double SIGMA_FACTOR = 2.0;
    double gWeighted = 256.0; 	// The weight for the gaussian window
    double lWeighted = 9.0; 	// The weight for the low-pass window
    int medianFrame = 0; 		// Counter for loading frame for median filter.
    int medianData[]; 			// For median filter.
    double volume = 1.0; 		// Control for the sound volume.
    double average = 0.0F; 		// Adaptive volume control method.
	double volumeAverage = 0.0F;// Used for report & paper.
    int totalFrames = 0; 		// Total frame so far, used for compute the time.
    Color bkColor = Color.WHITE;
	Color fgColor = Color.BLACK;
	
    double globalYMin = -32768.0F/2, globalYMax = 32767.0F/2; 
    // By Liwh : 10.8/04 
    // Devided by 2 solve the problem of spark waveform because of the (short) convert the positive (int) to negative.
    double globalMagMax = 80.0, globalMagMin = 0.0, magScale = 0.0, magMax = 0.0, magMin = 0.0;
    double xMin = 0.0, xMax = dataBufLen , yMin = 0.0, yMax = 0.0;
    double entropyThres = (4.0/5.0)*Math.log(dataBufLen/2);

    //Calculation interval along x-axis
    double stepLen = 4.0;        	// 1 seconds time for a window waveform
    String stepChoices[] =        	// when samplerate = 48000, setp <= 0.04, we can't plot, since the drawTimes = 1;
    {"4.0", "2.0", "1.0", "8.0", "16.0", "0.5", "0.2", "0.1", "0.05", "0.04", "0.02", "0.01"};
    String volumeChoices[] = 
    {"1.0", "2.0", "4.0", "8.0", "16.0", "32.0", "64.0"};
    String highPassChoices[] = 
    {"300", "200", "100", "400", "500", "600", "700", "800", "900", "1000"};
    String lowPassChoices[] = 
    {"3000", "2500", "2000", "3500", "4000", "4500", "5000", "5500", "6000"};

	//Tic mark for frequency window
	int freqRange = 6000;
	int yTicNum = 6;
	int xTicNum = 24;
	int winNum = 8; 		// How many times for a frame, ie, window size = frame / winNum.
	double magArray[] = new double [winNum*dataBufLen];
	
	double snrNoiseArray[] = new double [dataBufLen/2];
	double snrSpeechArray[] = new double [dataBufLen/2];
	int totalSNRNoiseFrame = 0;
	int snrSpeechFrames = 0;
	double segSNR = 0.0;
	boolean isSpeechArray[] = new boolean [1000]; // magic number;
	

    //Tic mark intervals
    double xTicInt = xMax / xTicNum;
    double yTicInt = freqRange / yTicNum;

    //Tic mark lengths.  If too small on x-axis, a default value is used later.
    double xTicLen = (yMax-yMin)/50;
    double yTicLen = (xMax-xMin)/50;

    DataInputStream input = null;
    PrintStream outputProcessedWaveFile = null;    
    PrintStream output = null;
    SourceDataLine sourceDataLine = null;
    TargetDataLine targetDataLine = null;
    int initTime = 0, endTime = 0;			// For setting initial time and crop time
    int fileLen, restLen;
    double gFilter[] =  new double[G_WIN];  // For Gaussian high pass Filter.
    double lFilter[] =  new double[L_WIN];  // For Gaussian low pass Filter.
    int data[] = new int [dataBufLen];
	int data1[] = new int[dataBufLen*3];;
    int drawData[] = new int [dataBufLen]; 	// Data for plotting the window
    int drawPos = 0;  	// Up to the position drawing
    int drawTimes = 0; 	// How many times to draw for the stepLen time interval.
    int speechFramePos = 0; //up to the position which is checked.
    double magnitude[];
    double real[];
    double imag[];
    double angle[];
    byte orderDouble[] = new byte [8];
    byte orderShort[] = new byte [2];
    double sampleFreq;
    int channels, channelBytes;
    int bitsPerSample = 0;
    float waveFileSampleRate = 48000.0F;
    
    WaveForm (LDVProject ldv) {
        parent = ldv;

        setLayout(new BorderLayout());
        setBorder(new EmptyBorder(5,5,5,5));
        
        //Create the control panel and give it a border for cosmetics.
        ctlPnl = new JPanel();
        //ctlPnl.setLayout(new GridLayout(0,1,0,10));
        ctlPnl.setLayout(new FlowLayout());
        ctlPnl.setBorder(new EtchedBorder());
		        
		//Button for runniing the graph
		timeBtn = new JButton("Set Time");
		AbstractAction timeAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				timeBtn.grabFocus();
				SetTimeActionPerformed (evt);
			}
		};
     
        //Button for runniing the graph
        stepBtn = new JButton("Step");
		AbstractAction stepAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				stepBtn.grabFocus();
				StepActionPerformed (evt);
			}
		};
		AbstractAction allAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				AllActionPerformed (evt);
			}
		};
     
        //Button for runniing the graph
        runBtn = new JButton("Run");
		AbstractAction runAction = new AbstractAction () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
				pauseBtn.grabFocus();
                RealtimeActionPerformed (evt);
            }
        };
     
        //Button for pausing the graph
        pauseBtn = new JButton("Pause");
		AbstractAction pauseAction = new AbstractAction () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
            	pauseBtn.grabFocus();
                PauseActionPerformed (evt);
            }
        };
     
        //Button for resetting the graph
        resetBtn = new JButton("Reset");
		AbstractAction resetAction = new AbstractAction () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
				resetBtn.grabFocus();
                ResetActionPerformed (evt);
            }
        };
     
        //Button for digital signal processing
        dspBtn = new JButton("DSP");
        dspBtn.addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                DspActionPerformed (evt);
            }
        });

        //Button for resetting the graph
        micBtn = new JButton("Microphone");
        micBtn.addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                MicrophoneActionPerformed (evt);
            }
        });

        stepList = new JComboBox ( stepChoices );
        stepList.setMaximumRowCount(2);
        stepList.addItemListener ( new java.awt.event.ItemListener() {
            public void itemStateChanged(ItemEvent evt)
            {
                RepaintActionPerformed(evt);
            }
        });

        noiseRemovalRBs[0] = new JRadioButton("Yes");
        noiseRemovalRBs[1] = new JRadioButton("No");
        noiseRemovalRBs[1].setSelected(true);
        noiseRemovalRBs[0].addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                NoiseRemovalActionPerformed (evt);
            }
        });
        noiseRemovalRBs[1].addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                NoiseRemovalActionPerformed (evt);
            }
        });
        noiseRemovalGroup.add(noiseRemovalRBs[0]);
        noiseRemovalGroup.add(noiseRemovalRBs[1]);
		
		dispWinBox[0] = new JCheckBox("Waveform", true);
		dispWinBox[1] = new JCheckBox("Spectrum", true);
		dispWinBox[2] = new JCheckBox("Spectrogram", true);
		dispWinBox[0].addActionListener(new java.awt.event.ActionListener () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				WindowDispalyActionPerformed (evt);
			}
		});
		dispWinBox[1].addActionListener(new java.awt.event.ActionListener () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				WindowDispalyActionPerformed (evt);
			}
		});
		dispWinBox[2].addActionListener(new java.awt.event.ActionListener () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				WindowDispalyActionPerformed (evt);
			}
		});
		
		dispFunBox[0] = new JCheckBox("Save Result", true);
		dispFunBox[1] = new JCheckBox("Use Existed Speech", true);
		dispFunBox[2] = new JCheckBox("Use dB Scale", true);
		dispFunBox[0].setSelected(false);
		dispFunBox[1].setSelected(false);
		dispFunBox[2].setSelected(true);
		dispFunBox[0].addActionListener(new java.awt.event.ActionListener () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				NeedSavedActionPerformed (evt);
			}
		});
		dispFunBox[1].addActionListener(new java.awt.event.ActionListener () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				UseMedianFilterDispalyActionPerformed (evt);
			}
		});
		dispFunBox[2].addActionListener(new java.awt.event.ActionListener () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				UsedBScaleActionPerformed (evt);
			}
		});

        gaussianFilterRBs[0] = new JRadioButton("Yes");
        gaussianFilterRBs[1] = new JRadioButton("No");
        gaussianFilterRBs[1].setSelected(true);
        gaussianFilterRBs[0].addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                GaussianFilterActionPerformed (evt);
            }
        });
        gaussianFilterRBs[1].addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                GaussianFilterActionPerformed (evt);
            }
        });
        gaussianFilterGroup.add(gaussianFilterRBs[0]);
        gaussianFilterGroup.add(gaussianFilterRBs[1]);

        volumeRBs[0] = new JRadioButton("Selective");
        volumeRBs[1] = new JRadioButton("Adaptive");
        volumeRBs[0].setSelected(true);
        volumeRBs[0].addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                VolumeActionPerformed (evt);
            }
        });
        volumeRBs[1].addActionListener(new java.awt.event.ActionListener () {
            public void actionPerformed (java.awt.event.ActionEvent evt) {
                VolumeActionPerformed (evt);
            }
        });
        volumeGroup.add(volumeRBs[0]);
        volumeGroup.add(volumeRBs[1]);
        
        volumeList = new JComboBox ( volumeChoices );
        volumeList.setMaximumRowCount(2);
        volumeList.setEnabled(true);
        volumeList.addItemListener ( new java.awt.event.ItemListener() {
            public void itemStateChanged(ItemEvent evt)
            {
                volume = Double.parseDouble(volumeChoices[volumeList.getSelectedIndex()]);
                
                // Comment the following out, can't see any usefulness. it leads to a bug of BS = 8; 12.20.05 by Liwh
                //if (bitsPerSample != 0) volume /= Math.pow(2,(bitsPerSample - 16)); // 12.13.04 by Liwh.
            }
        });
        
        highPassList = new JComboBox ( highPassChoices );
        highPassList.setMaximumRowCount(2);
        highPassList.addItemListener ( new java.awt.event.ItemListener() {
            public void itemStateChanged(ItemEvent evt)
            {
                HPFilterActionPerformed(evt);
            }
        });

        lowPassList = new JComboBox ( lowPassChoices );
        lowPassList.setMaximumRowCount(2);
        lowPassList.addItemListener ( new java.awt.event.ItemListener() {
            public void itemStateChanged(ItemEvent evt)
            {
                LPFilterActionPerformed(evt);
            }
        });
        
        pan0 = new JPanel();
        pan0.setLayout(new GridLayout(3,1));
        pan0.add(new JLabel("Time:"));
        pan0.add(stepList);
        pan1 = new JPanel();
        pan1.setLayout(new GridLayout(3,1));
        pan1.add(new JLabel("Volume Style:"));
        pan1.add(volumeRBs[0]);
        pan1.add(volumeRBs[1]);
        pan2 = new JPanel();
        pan2.setLayout(new GridLayout(3,1));
        pan2.add(new JLabel("Volume"));
        pan2.add(volumeList);
        pan3 = new JPanel();
        pan3.setLayout(new GridLayout(3,1));
        pan3.add(new JLabel("LP Filter?"));
        pan3.add(noiseRemovalRBs[0]);
        pan3.add(noiseRemovalRBs[1]);
        pan4 = new JPanel();
        pan4.setLayout(new GridLayout(3,1));
        pan4.add(new JLabel("HP Filter?"));
        pan4.add(gaussianFilterRBs[0]);
        pan4.add(gaussianFilterRBs[1]);
        pan5 = new JPanel();
        pan5.setLayout(new GridLayout(3,1));
        pan5.add(new JLabel("Freq.:"));
        pan5.add(highPassList);
        pan6 = new JPanel();
        pan6.setLayout(new GridLayout(3,1));
        pan6.add(new JLabel("Freq.:"));
        pan6.add(lowPassList);
		pan7 = new JPanel();
		pan7.setLayout(new GridLayout(3,1));
		pan7.add(dispWinBox[0]);
		pan7.add(dispWinBox[1]);
		pan7.add(dispWinBox[2]);
		pan8 = new JPanel();
		pan8.setLayout(new GridLayout(3,1));
		pan8.add(dispFunBox[0]);
		pan8.add(dispFunBox[1]);
		pan8.add(dispFunBox[2]);

		// Initilize the settings
		InitSettings();
		
        //Add the populated panels and the button to the control panel with a grid layout.
        ctlPnl.add(pan7);
		ctlPnl.add(pan0);
		ctlPnl.add(pan4);
        ctlPnl.add(pan5);
        ctlPnl.add(pan3);
        ctlPnl.add(pan6);
        ctlPnl.add(pan1);
        ctlPnl.add(pan2);
		ctlPnl.add(timeBtn);
        ctlPnl.add(stepBtn);
        ctlPnl.add(runBtn);
        ctlPnl.add(pauseBtn);
        ctlPnl.add(resetBtn);
		ctlPnl.add(pan8);
        //ctlPnl.add(dspBtn);
        //ctlPnl.add(micBtn);
        
		AbstractAction helpAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				ShowHelpMessagePerformed(evt);
			}
		};

		AbstractAction colorAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				usePaperColor = !usePaperColor;
			}
		};

		AbstractAction cropAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				CropAudioClip(evt);
			}
		};

		AbstractAction getMagAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				GetMagnitudePerformed(evt);
			}
		};

		AbstractAction getAveSSAction = new AbstractAction () {
			public void actionPerformed (java.awt.event.ActionEvent evt) {
				GetAverageSignalStrengthPerformed(evt);
			}
		};
     
        // Add shortcut for the keyboard
		InputMap inputMap = getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW);
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),  "RESET");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_A, 0),  "ALL");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_S, 0),  "STEP");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_R, 0),  "RUN");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_P, 0),  "PAUSE");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_T, 0),  "SETTIME");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0),  "HELP");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F3, 0),  "CHNAGECOLOR");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F4, 0),  "CROP");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F7, 0),  "GETASS");
		inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F8, 0),  "GETMAG");
		
		ActionMap actionMap = getActionMap();
		actionMap.put("RESET", resetAction);
		actionMap.put("ALL", allAction);
		actionMap.put("STEP", stepAction);
		actionMap.put("RUN", runAction);
		actionMap.put("PAUSE", pauseAction);
		actionMap.put("SETTIME", timeAction);
		actionMap.put("HELP", helpAction);
		actionMap.put("CHNAGECOLOR", colorAction);
		actionMap.put("CROP", cropAction);
		actionMap.put("GETMAG", getMagAction);
		actionMap.put("GETASS", getAveSSAction);
		
		stepBtn.setAction(stepAction);
		resetBtn.setAction(resetAction);
		runBtn.setAction(runAction);
		pauseBtn.setAction(pauseAction);
		timeBtn.setAction(timeAction);
		timeBtn.setText("Set Time");
		stepBtn.setText("Step");
		resetBtn.setText("Reset");
		runBtn.setText("Run");
		pauseBtn.setText("Pause");
        
        // Disable the initilized buttons.
		runBtn.setEnabled(false);
		timeBtn.setEnabled(false);
		stepBtn.setEnabled(false);
		pauseBtn.setEnabled(false);
		resetBtn.setEnabled(false);
		dspBtn.setEnabled(false);
		micBtn.setEnabled(true);
        //Create a panel to contain the Canvas objects.  They will be displayed in a one-column grid.
        canvasPanel = new JPanel();
        canvasPanel.setLayout(new GridLayout(0,1,5,4));
        
        canvases = new Canvas [3];
        canvases[0] = new MyCanvas(0);
        canvases[1] = new MyCanvas(1);
		canvases[2] = new MyCanvas(2);
		canvases[0].setBackground(bkColor);
		canvases[1].setBackground(bkColor);
		canvases[2].setBackground(bkColor);
        
		canvasPanel.add(canvases[0]);
		canvasPanel.add(canvases[1]);
		canvasPanel.add(canvases[2]);
        
        /*
        JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, 
            ctlPnl, canvasPanel);
        splitPane.setContinuousLayout(true);
        add(splitPane);
         */
        add("North",ctlPnl);
        add(canvasPanel);

        // Initialize the gaussian filter.
        windowSizeHigh(HIGH_FREQ);
        windowSizeLow(LOW_FREQ);
        // Initialize the median filter
        if (useMedianFilter)
                medianData = new int[dataBufLen*3]; 
        

		for (int i=0; i<3; i++) displayWindow[i] = true;
		RepaintCanvs();
        
        startAudio();
        startMicrophone();
    } // end of construction //

	public void InitSettings()
	{
		// Display settings
		dispWinBox[0].setSelected(true);
		dispWinBox[1].setSelected(false);
		dispWinBox[2].setSelected(false);
		
		// Step value
		stepLen = 4.0;
		
		// Filter settings.
		useGaussian = false;
		highPassList.setEnabled(false);
		isNoiseRemoval = false;
		lowPassList.setEnabled(false);
		
		// Volume type settings;
		isAdaptiveVol = false;
		volumeList.setEnabled(true);
		volume = 1.0;
		
		
	}
	
	public void RepaintCanvs() {
		canvases[0].repaint();
		canvases[1].repaint();
		canvases[2].repaint();
	}
	
    public void startAudio () {
        try {
            if (sourceDataLine != null) sourceDataLine.close();
            AudioFormat audioFormat = getAudioFormat();
            DataLine.Info dataLineInfo =  new DataLine.Info(
                      SourceDataLine.class, audioFormat);
            sourceDataLine = (SourceDataLine) AudioSystem.getLine(dataLineInfo);
            sourceDataLine.open(audioFormat);
            sourceDataLine.start();
        }
        catch (Exception e) {
            System.out.println("Start Audio Failure: " + e.getMessage());
        }
    }
    
    public void startMicrophone () {
        try {
            AudioFormat audioFormat = getMicrophoneFormat();
            DataLine.Info dataLineInfo =  new DataLine.Info(
                      TargetDataLine.class, audioFormat);
            targetDataLine = (TargetDataLine) AudioSystem.getLine(dataLineInfo);
            targetDataLine.open(audioFormat);
            targetDataLine.start();
        }
        catch (Exception e) {
            System.out.println("Start Microphone Failure: " + e.getMessage());
        }
    }

    public String setSelectedFileName(String sf) {
        return selectedFileName = currentDirectory + "\\" + sf;
    }
    
    public String setCurrentDirectory(String cd) {
        return currentDirectory = cd;
    }

    public void windowSizeHigh(int freq) {
        double alpha = freq / SIGMA_FACTOR;
        double sigma = 1 / ( 2 * Math.PI * alpha) ;
        sigma = waveFileSampleRate * sigma;
        G_WIN = (int) (SIGMA_FACTOR * 2 * sigma);
        gFilter =  new double[G_WIN+1];
        Gaussian(gFilter, sigma);
        System.out.println("High Pass Size:" + G_WIN);
    }

    public void windowSizeLow(int freq) {
        double alpha = freq / SIGMA_FACTOR;
        double sigma = 1 / ( 2 * Math.PI * alpha) ;
        sigma = waveFileSampleRate * sigma;
        L_WIN = (int) (SIGMA_FACTOR * 2 * sigma);
        lFilter =  new double[L_WIN+1];
        Gaussian(lFilter, sigma);
        System.out.println("Low Pass Size:" + L_WIN);
    }
    
    public void HPFilterActionPerformed (java.awt.event.ItemEvent evt) {
        int tempFreq = Integer.parseInt(highPassChoices[highPassList.getSelectedIndex()]);
        windowSizeHigh(tempFreq);
		HIGH_FREQ = tempFreq;
    }
  
	public void WindowDispalyActionPerformed (java.awt.event.ActionEvent evt) {
		for (int i = 0; i < 3; i ++)
			displayWindow[i] = dispWinBox[i].isSelected();
	}
	
	public void NeedSavedActionPerformed (java.awt.event.ActionEvent evt) {
		needBeSaved = dispFunBox[0].isSelected();
		if (needBeSaved && selectedFileName != null)
			JOptionPane.showMessageDialog(this,"Please select the file again!","Reminding...",JOptionPane.INFORMATION_MESSAGE);
		
	}
	
	public void UseMedianFilterDispalyActionPerformed (java.awt.event.ActionEvent evt) {
		//useMedianFilter = dispFunBox[1].isSelected();
		useExSpeech = dispFunBox[1].isSelected();
		
	}

	public void UsedBScaleActionPerformed (java.awt.event.ActionEvent evt) {
		usedBScale = dispFunBox[2].isSelected();
		
	}

    public void LPFilterActionPerformed (java.awt.event.ItemEvent evt) {
        int tempFreq = Integer.parseInt(lowPassChoices[lowPassList.getSelectedIndex()]);
        windowSizeLow(tempFreq);
        LOW_FREQ = tempFreq;
    }
    
	public void ShowHelpMessagePerformed (java.awt.event.ActionEvent evt) {

		String showMessage = 
			"Alt+1: Choose the wave form tab. \n" +
			"Alt+2: Choose the configuration tab. \n\n" +
			"\tIn this wave form tab: \n" +
			"\t\tESC: Reset button. \n" +
			"\t\tF1: Popup this help dialog. \n" +			"\t\tF3: Flip colors for display and paper versios. \n" +			"\t\tF4: Crop a sound clip. \n" +
			"\t\tF7: Compute the average return signal strength. \n" +
			"\t\tF8: Compute the average magnitude. \n" +			"\t\t O: Open files (or Ctrl + O).  \n" +
			"\t\t S: Step button. \n" +			"\t\t R: Run button. \n" +			"\t\t P: Pause button. \n" +			"\t\t T: Set time button. \n" +			"\t\t A: Automatically display A full Window. \n\n" +
			"\tIn configuration tab: \n" +
			"\t\tESC: Clear button. \n" +
			"\t\tF1: Popup help dialog. \n" +
			"\t\tF2: AutoFocus. \n" +
			"\t\tF5: Get signal strength. \n" +
			"\t\tF9: Save signal strength values to file. \n" +
			"\t\tF10: Stop saving signal strength. \n" +
			"\t\tF12: Send commands for default parameters. \n";

		JOptionPane.showMessageDialog(this,showMessage,"Short-cut Keyboard Mapping",JOptionPane.INFORMATION_MESSAGE);
	}

	public void GetAverageSignalStrengthPerformed (java.awt.event.ActionEvent evt) {
		ExampleFileFilter filter = new ExampleFileFilter();
		filter.addExtension("txt");
		filter.setDescription("TXT ");
		JFileChooser fChooser = new JFileChooser();
		fChooser.setDialogTitle("Choose the record file:");
		fChooser.setFileFilter(filter);   
		File currDir = new File (parent.getCD());
		fChooser.setCurrentDirectory(currDir);
		int returnVal = fChooser.showOpenDialog(this);
		String ssFileName = null;
		if (returnVal == JFileChooser.APPROVE_OPTION)
			ssFileName = fChooser.getSelectedFile().getPath();
		else return;

		
		try {
			File in = new File(ssFileName);
			if (!in.exists())
			{
				JOptionPane.showMessageDialog(this,"File Not Exists: " + ssFileName);
				return;
			}
			BufferedReader input = new BufferedReader(new InputStreamReader(new FileInputStream(in)));
			
			String temp = null;
			long sum = 0;
			int count  = 0;
			input.readLine(); //Ignore the first line.
			while (true) 
			{
				temp = input.readLine();
				if (temp == null) break;
				sum += Integer.parseInt(temp);
				count ++;
			}
			
			input.close();
			
			JOptionPane.showMessageDialog(this,"The Average " + count + " Signal Strengths is: " + (sum/count));
			
		} catch (NumberFormatException e) {
			JOptionPane.showMessageDialog(this,"File Format Error: -- " + ssFileName);
		} catch (Exception e) {
			JOptionPane.showMessageDialog(this,"Average singal strength computation error!");
		}
		

	}

	public void GetMagnitudePerformed (java.awt.event.ActionEvent evt) {
		if (selectedFileName == null)
		{
			JOptionPane.showMessageDialog(this,"Please select a file first");
			return;
		}

		byte shortBytes [] = new byte [2];
		byte bs[] = new byte[4];
		double tempData = 0.0;
	
		volumeAverage = 0.0;
		try {
			switch (bitsPerSample) 
			{
				case 16: 
					for (int i = 0; i < restLen; i++)
					{
						input.readFully(shortBytes);  // Left channel
						tempData = (double) (ByteBuffer.wrap(shortBytes).order(ByteOrder.LITTLE_ENDIAN).getShort());
						tempData = (tempData > 0) ? tempData : -tempData;
						volumeAverage += tempData / restLen;                
						if (channels == 2) input.readFully(shortBytes); 
					}
					break;
	                            
				case 24:
	                            
					for (int i = 0; i < restLen; i++)
					{
						input.readFully(bs,1,3);
						bs[0] = 0;
						tempData = (double) (ByteBuffer.wrap(bs).order(ByteOrder.LITTLE_ENDIAN).getInt() / 256);
						tempData = (tempData > 0) ? tempData : -tempData;
						volumeAverage += tempData / restLen;                
						if (channels == 2) input.readFully(bs,1,3);
					}
					break;
			}
		} catch (Exception e) {System.out.println("File stream read error!");}
		
		resetBtn.grabFocus();
		ResetActionPerformed (evt);
		
		JOptionPane.showMessageDialog(this,"The Average Magnitude is: " + (Double.toString(volumeAverage)).substring(0,(Double.toString(volumeAverage)).indexOf('.') + 3));
		
	}

    public void RepaintActionPerformed (java.awt.event.ItemEvent evt) {
        double tempLen = stepLen;
        stepLen = Double.parseDouble(stepChoices[stepList.getSelectedIndex()]);
        drawTimes = (int)(( stepLen * waveFileSampleRate ) / dataBufLen);
        if (drawTimes == 0) {
            stepLen = tempLen;
            drawTimes = (int)(( stepLen * waveFileSampleRate ) / dataBufLen);
            stepList.setSelectedItem(String.valueOf(stepLen));
            JOptionPane.showMessageDialog(this,"This choice is too small for the sample rate of " + waveFileSampleRate, "Error", JOptionPane.ERROR_MESSAGE);
        }

		int timeLen = (int)stepLen;
		if (colorArray != null && timeLen > 1)
			colorArray = null;

		RepaintCanvs();

    }

	// The constructor:
	// timeType: 0: start time 1: end time.
	public class JSetTimeDlg extends JDialog {
		JTextField jtf1;
		int timeType = 0;
		
		public JSetTimeDlg(String title, String name, int tt){
			super(parent,true);
			timeType = tt;
			initComponents(title, name);
			setLocation(600,165);
		}
		
		public void initComponents(String title, String name) {
			setTitle(title);
			JPanel p1 = new JPanel();
			JPanel p2 = new JPanel();
			JLabel j1 = new JLabel (name);
			JLabel j2 = new JLabel ("milliseconds.");
			jtf1 = new JTextField(5);
			JButton okBtn = new JButton("Ok");
			JButton cancelBtn = new JButton("Cancel");
			getContentPane().setLayout(new GridLayout(2,2)); 
			
			okBtn.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent evt) {
					getTheInitialTime(evt);
				}
			});
			
			AbstractAction cancelAction = new AbstractAction() {
				public void actionPerformed(ActionEvent e) {
					closeDialog(e);      
				}    
			};
			
			p1.add(j1);
			p1.add(jtf1);
			p1.add(j2);
			p2.add(okBtn);
			p2.add(cancelBtn);
			getContentPane().add(p1);
			getContentPane().add(p2);
			getRootPane().setDefaultButton(okBtn);

			InputMap inputMap = getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW);
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),  "CANCEL");
			ActionMap actionMap = getRootPane().getActionMap();
			actionMap.put("CANCEL", cancelAction);
			cancelBtn.setAction(cancelAction);
			cancelBtn.setText("Cancel");		//Don't know why??????  11/12/04
								      
			pack();
		
		}

		public void getTheInitialTime(java.awt.event.ActionEvent evt){
			String initTimeString = jtf1.getText();
			try {
				if (timeType == 0) {
					initTime = Integer.parseInt(initTimeString);
					int frames = (int)( (double)(initTime) * waveFileSampleRate / 1000000);
					if (frames * dataBufLen > restLen || frames < 0)
						throw new RuntimeException("The time must larger than or equal to 0 and less than " + 1000*(int)(restLen/waveFileSampleRate) + " milliseconds." );
					// Skipping the beginning of the data
					input.skipBytes(frames*dataBufLen*channels*(bitsPerSample/8)); // seek to the data area;
					restLen -= frames*dataBufLen;
					totalFrames = frames;
					timeBtn.setEnabled(false);
					closeDialog(evt);
				} else if (timeType == 1) { // This is for cropping.
					endTime = Integer.parseInt(initTimeString);
					if (endTime <= initTime)
						throw new RuntimeException("The end time should be larger than the start time!" );
					int frames = (int)( (double)(endTime) * waveFileSampleRate / 1000000);
					if (frames * dataBufLen > fileLen || frames <= 0)
						throw new RuntimeException("The time must larger than 0 and less than " + 1000*(int)(fileLen/waveFileSampleRate) + " milliseconds." );
					timeBtn.setEnabled(false);
					closeDialog(evt);
				}
			}catch (NumberFormatException e) {
				JOptionPane.showMessageDialog(this,"Please input a positive integer number!","Exception Catched:", JOptionPane.ERROR_MESSAGE);
				jtf1.setText(null);
				jtf1.grabFocus();
			}catch (RuntimeException e) {
				JOptionPane.showMessageDialog(this,e.getMessage(),"Exception Catched:", JOptionPane.ERROR_MESSAGE);
				jtf1.setText(null);
				jtf1.grabFocus();
			}catch (IOException e) {
				JOptionPane.showMessageDialog(this,"Fatal Error while set the location of the stream data","Exception Catched:", JOptionPane.ERROR_MESSAGE);
			}
		}
			
		public void closeDialog(java.awt.event.ActionEvent evt) {
			setVisible(false);
			dispose();
		}
	}

	public void SetTimeActionPerformed (java.awt.event.ActionEvent evt) {
		if (selectedFileName == null) {
			JOptionPane.showMessageDialog(this, "Please choose file first.");
			return;
		}

		new JSetTimeDlg("Set the Start Time:","Start Time:  ", 0).show();
	}
		
	public void IntegerToByte(int orig, byte [] dest) {
		dest[3] = (byte) (orig >>> 24);
		dest[2] = (byte) (orig >>> 16);
		dest[1] = (byte) (orig >>> 8);
		dest[0] = (byte) (orig);
	}

	public void CropAudioClip (java.awt.event.ActionEvent evt) {
		if (selectedFileName == null) {
			JOptionPane.showMessageDialog(this, "Please choose file first.");
			return;
		}
		
		initTime = endTime = -1;
		new JSetTimeDlg("Set the Start Time:","Start Time:  ", 0).show();
		if (initTime == -1) return;
		
		new JSetTimeDlg("Set the End Time:","End Time:  ", 1).show();
		if (endTime == -1) return;
		
		System.out.println("The init time and end time is: " + initTime + " " + endTime);
		int frames = (int)( (double)(endTime - initTime) * waveFileSampleRate / 1000000);
		int newFileLen = frames*dataBufLen*channels*(bitsPerSample/8);
		byte orderInt[] = new byte [4];
		byte tempBuff[] = new byte [64];
		int diffLen = 0;

		try {

			if (input != null ) input.close();
			input = new DataInputStream(new FileInputStream(selectedFileName));
            
			// Initialize the output processed wave file.
			String processedFileName = selectedFileName.substring(0,selectedFileName.length() - 4) + "_crop.wav";
			System.out.println("Output processed file:" + processedFileName);
			outputProcessedWaveFile = new PrintStream (new FileOutputStream(processedFileName));
			copyFileStream(input,outputProcessedWaveFile,4);
			
			// By Liwh: 1/4/05 fixed the wave head format bug.
			input.readFully(orderDouble,0,4);
            
			input.readFully(tempBuff,diffLen,2);
			String fmtStr = new String(tempBuff,diffLen,2);
			diffLen += 2;
			while (!fmtStr.equals("ta"))
			{
				input.readFully(tempBuff,diffLen,2);
				fmtStr = new String(tempBuff,diffLen,2);
				diffLen += 2;
			}

			IntegerToByte(newFileLen+diffLen+4, orderInt);
			outputProcessedWaveFile.write(orderInt, 0 ,4);


			outputProcessedWaveFile.write(tempBuff,0,diffLen);
			input.readFully(orderDouble,0,4);
			IntegerToByte(newFileLen, orderInt);
			outputProcessedWaveFile.write(orderInt,0,4);


			frames = (int)( (double)(initTime) * waveFileSampleRate / 1000000);
			input.skipBytes(frames*dataBufLen*channels*(bitsPerSample/8)); // seek to the data area;
			for (int i = 0; i < newFileLen; i=i+2)
				copyFileStream(input,outputProcessedWaveFile,2);

			outputProcessedWaveFile.close();
			
			// Reset the input file handler.            
			if (input != null ) input.close();
			input = new DataInputStream(new FileInputStream(selectedFileName));
			
			JOptionPane.showMessageDialog(this, processedFileName, "Cropped File Save As:", JOptionPane.INFORMATION_MESSAGE);
		} catch (Exception e) {
			System.out.println("Crop file saved error!!!");
		}

	}
	
    public void PauseActionPerformed (java.awt.event.ActionEvent evt) {
        if (!pause) {
            pause = true;
            stepBtn.setEnabled(true);
            resetBtn.setEnabled(true);
        }
        else {
            pause = false;
            stepBtn.setEnabled(false);
        }
        
        if (!stopCapture) stopCapture = true;
        else stopCapture = false;
    }
    
    public void DspActionPerformed (java.awt.event.ActionEvent evt) {
        String newAudioFile = selectedFileName.substring(0,selectedFileName.length() - 4) + "_new.wav";
        
        try {
            input = new DataInputStream(new FileInputStream(selectedFileName));
            output = new PrintStream(new FileOutputStream(newAudioFile));

            int tempLen = (input.available() - 44) / 4;
            short cursorShrot = 0;
            int minV = 0, maxV = 0, tempV = 0;
            int newFileLen = input.available();
            byte bs[] = new byte [2];
            
            //input.mark(tempLen * 4 + 440); mark and reset are not supported!
            input.skipBytes(44); // seek to the data area;
            for (int i = 0; i < tempLen; i++)
            {
                input.readFully(bs); 
                //tempV = ByteBuffer.wrap(bs).order(ByteOrder.LITTLE_ENDIAN).getShort();
                //minV = (tempV < minV) ? tempV : minV;
                //maxV = (tempV > maxV) ? tempV : maxV;

                input.readFully(bs); 
                tempV = ByteBuffer.wrap(bs).order(ByteOrder.LITTLE_ENDIAN).getShort();
                minV = (tempV < minV) ? tempV : minV;
                maxV = (tempV > maxV) ? tempV : maxV;
                
            }
            System.out.println("Max and Min is : " + maxV + " : " + minV);
            input.close();

            input = new DataInputStream(new FileInputStream(selectedFileName));
            input.skipBytes(44); // seek to the data area;
            double scaleV = 0.05 * (maxV - minV); // get the threshold value;
            long total = 0;
            int frameNum = tempLen / dataBufLen + 1;
            int remainNum = tempLen % dataBufLen;
            boolean needWrite [] = new boolean [frameNum];

            for (int j = 0; j < frameNum - 1 ; j++)
            {
                needWrite[j] = true;
                for (int i = 0; i < dataBufLen; i++)
                {
                    input.readFully(bs); 

                    input.readFully(bs); 
                    tempV = ByteBuffer.wrap(bs).order(ByteOrder.LITTLE_ENDIAN).getShort();
                    total += (tempV > 0 ) ? tempV : 0 - tempV;
                }

                if ((total / dataBufLen) > scaleV ) {
                    newFileLen -= 4 * dataBufLen;
                    needWrite[j] = false;
                }

                System.out.println("Average v.s. Scale : " + (total / dataBufLen) + " : " + scaleV + " " + needWrite[j]);
                
                total = 0;
                
            }
            input.close();
            
            
            input = new DataInputStream(new FileInputStream(selectedFileName));
            byte copyData [] = new byte [4*dataBufLen];
            input.readFully(copyData,0,4);
            output.write(copyData,0,4);
            newFileLen += 38; // Hex: 26
            byte b = (byte)newFileLen;
            output.write(b);
            b = (byte) (newFileLen >>> 8);
            output.write(b);
            b = (byte) (newFileLen >>> 16);
            output.write(b);
            b = (byte) (newFileLen >>> 24);
            output.write(b);
            
            input.skipBytes(4);
            input.readFully(copyData,0,34);
            output.write(copyData,0,34);
            newFileLen -= 38; // Hex: 26
            b = (byte)newFileLen;
            output.write(b);
            b = (byte) (newFileLen >>> 8);
            output.write(b);
            b = (byte) (newFileLen >>> 16);
            output.write(b);
            b = (byte) (newFileLen >>> 24);
            output.write(b);    
            input.skipBytes(4);
            
            for (int j = 0; j < frameNum - 1 ; j++)
            {
                if ( !needWrite[j]) {
                    input.skipBytes(4*dataBufLen);
                    continue;
                }
                    
                input.readFully(copyData); 
                output.write(copyData,0,4*dataBufLen);
                
            }

            if (remainNum > 0) {
                input.readFully(copyData,0, remainNum * 4);
                output.write(copyData,0,remainNum * 4);
            }
            
            input.close();
            output.close();
            
            
        }
        catch (Exception e) { System.out.println("File error!" + e.getMessage()); }

    }

    public void copyFileStream(DataInputStream in, PrintStream out, int length) {
        byte swap[] = new byte[64];
        
        try {
            if (length < 64)
            {
                in.readFully(swap,0,length);
                out.write(swap,0,length);
            }
            else {
                System.out.println("It's larger than 64 bit, so quit!");
                System.exit(0);
            }
            
        } catch (Exception e) 
        {   System.out.println("Error Copy File Stream:" + e.getMessage()); }
       
    }
    
    public void ResetActionPerformed (java.awt.event.ActionEvent evt) {
        restLen = fileLen;
        Pattern p = Pattern.compile("\\.wav*");
        
        // A bug found on 2/16/06: to playback TIMIT sph format, and found that the frequncy is not correct.
        // Not sure what is the purpose of this isWaveFormat variable for further use.
        //Matcher m = p.matcher(selectedFileName);
        //isWaveFormat = m.find();
        String fmtStr = new String(orderDouble,0,2);
        
		// Initialize.
        firstFrame = true;  
        reset = true;     
        totalFrames = 0;
		speechFramePos = 0;
		snrSpeechFrames = 0;
		totalSNRNoiseFrame = 0;
		segSNR = 0.0;
		for (int i=0; i<dataBufLen/2; i++)
			snrNoiseArray[i] = 0.0; // need to be initilized in Reset.
		for (int i=0; i<dataBufLen/2; i++)
			snrSpeechArray[i] = 0.0;  // Assumed clean speech.


        try {

            //if (realtimeThread != null) realtimeThread.join(100);
            if (input != null ) input.close();
            input = new DataInputStream(new FileInputStream(selectedFileName));
            
            // Initialize the output processed wave file.
            if( needBeSaved )
            {
                String processedFileName = selectedFileName.substring(0,selectedFileName.length() - 4) + "_new.wav";
                System.out.println("Output processed file:" + processedFileName);
                outputProcessedWaveFile = new PrintStream (new FileOutputStream(processedFileName));
                copyFileStream(input,outputProcessedWaveFile,22);
                
                input.readFully(orderDouble,0,2);
                fmtStr = new String(orderDouble,0,2);
                while (!fmtStr.equals("ta"))
                {
                    outputProcessedWaveFile.write(orderDouble,0,2);
                    input.readFully(orderDouble,0,2);
                    fmtStr = new String(orderDouble,0,2);
                }
                outputProcessedWaveFile.write(orderDouble,0,2);
                input.readFully(orderDouble,0,4);
                outputProcessedWaveFile.write(orderDouble,0,4);
                
                if (input != null ) input.close();
                input = new DataInputStream(new FileInputStream(selectedFileName));
            }
            
			input.readFully(orderDouble,0,4);
			fmtStr = new String(orderDouble,0,4);

            if (fmtStr.equals("NIST"))
            {
            	int headLen = 1024;
            	byte strByte[] = new byte[headLen];
				input.readFully(strByte,0,headLen-4);
				fmtStr = new String(strByte,0,headLen-4);
				String temp = fmtStr.substring(fmtStr.indexOf("sample_rate")+15, fmtStr.indexOf("sample_rate")+20);
				waveFileSampleRate = Float.valueOf(temp).floatValue(); 
				temp = fmtStr.substring(fmtStr.indexOf("channel_count")+17, fmtStr.indexOf("channel_count")+18);
				channels = Integer.valueOf(temp).intValue(); 
				temp = fmtStr.substring(fmtStr.indexOf("sample_sig_bits")+19, fmtStr.indexOf("sample_sig_bits")+21);
				bitsPerSample = Integer.valueOf(temp).intValue(); 
				temp = fmtStr.substring(fmtStr.indexOf("sample_n_bytes")+18, fmtStr.indexOf("sample_n_bytes")+19);
				channelBytes = Integer.valueOf(temp).intValue(); 
				temp = fmtStr.substring(fmtStr.indexOf("sample_count")+16, fmtStr.indexOf("sample_count")+21);
				int tat = Integer.valueOf(temp).intValue(); 
				restLen = fileLen = tat;
				
				drawTimes = (int)(( stepLen * waveFileSampleRate ) / dataBufLen);
				drawPos = 0;
				//input.skipBytes(1024-256);
				 
				
			}
            else 
            {
            	// input.skipBytes(22);
            	// should minus 4.
				input.skipBytes(18);
	            input.readFully(orderDouble,0,2);
	            channels = ByteBuffer.wrap(orderDouble).order(ByteOrder.LITTLE_ENDIAN).getShort();
	            input.readFully(orderDouble,0,4);
	            waveFileSampleRate = ByteBuffer.wrap(orderDouble).order(ByteOrder.LITTLE_ENDIAN).getInt();
	            drawTimes = (int)(( stepLen * waveFileSampleRate ) / dataBufLen);
	            drawPos = 0;
	
	            input.skipBytes(4);
	            input.readFully(orderDouble,0,2);
	            channelBytes = ByteBuffer.wrap(orderDouble).order(ByteOrder.LITTLE_ENDIAN).getShort();
	            input.readFully(orderDouble,0,2);
	            bitsPerSample = ByteBuffer.wrap(orderDouble).order(ByteOrder.LITTLE_ENDIAN).getShort();
	
	
	            input.readFully(orderDouble,0,2);
	            fmtStr = new String(orderDouble,0,2);
	            while (!fmtStr.equals("ta"))
	            {
	                input.readFully(orderDouble,0,2);
	                fmtStr = new String(orderDouble,0,2);
	            }
	
	            input.readFully(orderDouble,0,4);
	            restLen = fileLen = ByteBuffer.wrap(orderDouble).order(ByteOrder.LITTLE_ENDIAN).getInt() / channelBytes;
	
	            //input.skipBytes(2); // For bytes alignment. IMPORTANT!!!!
	            //input.skipBytes(3); // For bytes alignment. IMPORTANT!!!!
			}
			       
            System.out.println("File name is:\\\\" + selectedFileName.substring(currentDirectory.length()+1));
            System.out.println("Sample Rate is:" + waveFileSampleRate);
            System.out.println("Channel number is:" + channels);
            System.out.println("Bits for sample:" + bitsPerSample);
            System.out.println("Channel bytes is:" + channelBytes);
            System.out.println("Data Lens:" + restLen);


			// Initialize the gaussian filter again.
			windowSizeHigh(HIGH_FREQ);
			windowSizeLow(LOW_FREQ);
			// end by liwh 12/15/04
            
            yMax = yMin = 0.0;
            for (int i = 0; i < dataBufLen; i++)
                data[i] = 0;
            //Compute magnitude spectra of the raw data and save it in output arrays.  
            //Note that the real, imag, and angle arrays are not used later, 
            //So they are discarded each time a new spectral analysis is performed.
            magnitude = new double[dataBufLen];
            for (int i=0; i<dataBufLen; i++) magnitude[i] = 0;
            FastFourierTransform.fftMag(data,magnitude);

            magMax = magMin = 0.0;
            for ( int i = 1; i < dataBufLen; i++)
            {
                if (magnitude[i] > magMax) magMax = magnitude[i];
                else if (magnitude[i] < magMin) magMin = magnitude[i];
            }

        }
        catch (Exception e) {
            System.err.println("Reset Error:" + e.getMessage());
        }
        
        resetBtn.setEnabled(true);
        dspBtn.setEnabled(true);
        stepBtn.setEnabled(true);
        runBtn.setEnabled(true);
        pauseBtn.setEnabled(true);
		timeBtn.setEnabled(true);
        pause = false;
        startAudio(); // Restart the Audio device based on the wave file sample rate.
        
		RepaintCanvs();

    }

    public int [] SortArray(int [] source, int pos, int window) {
        int medianWinSize = 2 * window + 1;
        int result[] = new int[medianWinSize]; // By Liwh, to let the sample data centered.
        int temp;

        for (int i=0; i < medianWinSize; i++)
            result[i] = source[pos+i-window];
        
        for (int i=0; i < medianWinSize; i++)
            for (int j=i+1; j < medianWinSize; j++) 
                if (result[j]<result[i]) {
                    temp = result[i];
                    result[i] = result[j];
                    result[j] = temp;
                }

        return result;
    }

    public int GetMedianFromNewArray(int out, int in, int newArray[]) {
        int size = newArray.length;
        int i,j;
 
        if (in == out) return newArray[size/2];

        for (i = 0; i < size; i++)
            if (newArray[i] == out) break;
        for (j = 0; j < size; j++)
            if (in < newArray[j]) break;
        
        if (i == size) System.out.println("Something wrong in GetMedianFromNewArray: i = " + i);

        if (j == 0) {
            System.arraycopy(newArray, 0, newArray, 1, i);
            newArray[0] = in;
        }
        else if (i < j-1) {
            System.arraycopy(newArray, i+1, newArray, i, j-1-i);
            newArray[j-1] = in;
        }
        else if (i > j-1) {
            System.arraycopy(newArray, j, newArray, j+1, i-j);
            newArray[j] = in;
        }else newArray[i] = in;
        
        return newArray[size/2];
    }
    
    public void MedianFilter (int flag) {
        if (flag != -1) {
            for (int i=0; i < dataBufLen; i++)
                medianData[i + flag*dataBufLen] = data[i];
        }
        else {
            System.arraycopy(medianData,dataBufLen,medianData,0, 2*dataBufLen);
            for (int i=0; i < dataBufLen; i++)
                medianData[i + 2*dataBufLen] = data[i];
            
            //Save the processed data to the array data[];
            int tempArray[] = SortArray(medianData, dataBufLen, M_WIN);
            data[0] = tempArray[M_WIN];
            for (int i = dataBufLen+1 ; i < 2 * dataBufLen; i ++) 
                data[i-dataBufLen] = GetMedianFromNewArray(medianData[i-M_WIN-1],medianData[i+M_WIN],tempArray);
                // We move the window one pixel a time.
        }
    }

	public void ReadDataFromFileBuffer() {
		byte shortBytes [] = new byte [2];
		byte bs[] = new byte[4];

		try {
			// Reset Volume for the Noise Removal entropy method.
			if (volume == 0) 
				volume = Double.parseDouble(volumeChoices[volumeList.getSelectedIndex()]);
	
	
			// Step 1: Read data from wave file.
			if (firstFrame)  // Suppose the input data is stereo and 16bits or 24bits.
			{
				for (int i = 0; i < 3*dataBufLen; i++) 
					data1[i] = 0;
				// Padding the 3rd group using 0.
	                	
				switch (bitsPerSample) 
				{
					case 8: 
						for (int i = 0; i < 2*dataBufLen; i++)
						{
							shortBytes[0] = 0;
							shortBytes[1] = 0;
							input.readFully(shortBytes, 0, 1);  // 8 bits per sample
							data1[i] = ByteBuffer.wrap(shortBytes).order(ByteOrder.LITTLE_ENDIAN).getShort();
							if (i<dataBufLen) data[i] = data1[i];
							if (channels == 2) input.readFully(shortBytes, 1, 1); 
						}
						break;

					case 16: 
						for (int i = 0; i < 2*dataBufLen; i++)
						{
							input.readFully(shortBytes);  // Left channel
							data1[i] = ByteBuffer.wrap(shortBytes).order(ByteOrder.LITTLE_ENDIAN).getShort();
							if (i<dataBufLen) data[i] = data1[i];
							if (channels == 2) input.readFully(shortBytes); 
						}
						break;
	                            
					case 24:
	                            
						for (int i = 0; i < 2*dataBufLen; i++)
						{
							input.readFully(bs,1,3);
							bs[0] = 0;
							// Divided by 256
							data1[i] = ByteBuffer.wrap(bs).order(ByteOrder.LITTLE_ENDIAN).getInt() / 256;
							if (i<dataBufLen) data[i] = data1[i];
							if (channels == 2) input.readFully(bs,1,3);
						}
						break;
				}
	
				restLen -= dataBufLen;  // For the bounding case.
				totalFrames ++;
	                    
			}
			else {
				switch (bitsPerSample) 
				{
					case 8: 
						for (int i = dataBufLen; i < 2*dataBufLen; i++)
						{
							shortBytes[0] = 0;
							shortBytes[1] = 0;
							input.readFully(shortBytes, 0, 1); 
							data[i-dataBufLen] = data1[i-dataBufLen];
							data1[i] = ByteBuffer.wrap(shortBytes).order(ByteOrder.LITTLE_ENDIAN).getShort();
							if (channels == 2) input.readFully(shortBytes, 1, 1); 
						}
	
						break;
	                            
					case 16: 
						for (int i = dataBufLen; i < 2*dataBufLen; i++)
						{
							input.readFully(shortBytes); 
							data[i-dataBufLen] = data1[i-dataBufLen];
							data1[i] = ByteBuffer.wrap(shortBytes).order(ByteOrder.LITTLE_ENDIAN).getShort();
							if (channels == 2) input.readFully(shortBytes); 
						}
	
						break;
	                            
					case 24:
						for (int i = dataBufLen; i < 2*dataBufLen; i++)
						{
							input.readFully(bs,1,3);
							bs[0] = 0;
							// Divided by 256
							data[i-dataBufLen] = data1[i-dataBufLen];
							data1[i] = ByteBuffer.wrap(bs).order(ByteOrder.LITTLE_ENDIAN).getInt() / 256;
							if (channels == 2) input.readFully(bs,1,3);
						}
	
						break;
				}
	
			}
		} catch (Exception e) {System.out.println("File stream read error!");}
		
	}
	
	public void CalMagArray() {
		
		double magnitude1[] = new double[dataBufLen];
		int dataTemp [] = new int [dataBufLen];
		int offset = 0;
		
		for (int num = 0; num < winNum; num++)
		{		
			//For the mid-point of the buffer, from 0 to winNum-1;
			offset = ((dataBufLen/winNum)*num - dataBufLen/2 + 3*dataBufLen) % (3*dataBufLen); 
			// Compute the Low-Pass Filter Value.
			if (isNoiseRemoval) { // low pass
				for (int i = 0; i < dataBufLen; i++)
				{
					double sum = 0.0;
					// Gaussian Filter.
					for (int j = - L_WIN/2 ; j < L_WIN/2; j++)
						sum += data1[((offset+i)+j+ 3*dataBufLen) % (3*dataBufLen)]*lFilter[j+L_WIN/2];
					dataTemp[i] = (int)sum; // low pass                        
				}
	
				for (int i = 0; i < dataBufLen; i++)
					data1[(offset+i+ 3*dataBufLen) % (3*dataBufLen)]= dataTemp[i]; // copy back 
			}
			// Compute the Gaussian Filter Value, i.e. High-pass filter.
			if (useGaussian) 
			{
	
				for (int i = 0; i < dataBufLen; i++)
				{
					double sum = 0.0;
					// Gaussian Filter.
					for (int j = - G_WIN/2 ; j < G_WIN/2; j++)
						sum += data1[((offset+i)+j+ 3*dataBufLen) % (3*dataBufLen)]*gFilter[j+G_WIN/2];
					dataTemp[i] = data1[(offset+i+ 3*dataBufLen) % (3*dataBufLen)] - (int)sum; // high pass
	                        
				}
	                    
			}
			if (!useGaussian && !isNoiseRemoval)
			{ 
				for (int i = 0; i < dataBufLen; i++)
					dataTemp[i] = data1[((offset+i) + 3*dataBufLen) % (3*dataBufLen)];
			}
			
			FastFourierTransform.fftMag(dataTemp,magnitude1);
			System.arraycopy(magnitude1,0,magArray,num*dataBufLen,dataBufLen);

		}// end of winNum iterations.
		System.arraycopy(magArray,0,magnitude,0,dataBufLen);
		
	}

    public void StepActionPerformed (java.awt.event.ActionEvent evt) {
        
        //restLen -= dataBufLen;
        // totalFrames ++; add by the end of code.
		timeBtn.setEnabled(false);
        
        long startTime = 0, endTime = 0;

        if (restLen > dataBufLen)
        {
            try {
            	
				ReadDataFromFileBuffer();
				
				// Step 3: Do Gaussian Filter if needed.
				if (firstFrame) firstFrame = false;
				// Compute the Low-Pass Filter Value.
				
				if (isNoiseRemoval) { // low pass
					for (int i = L_WIN/2; i < dataBufLen + L_WIN/2; i++)
					{
						double sum = 0.0;
						// Gaussian Filter.
						for (int j = - L_WIN/2 ; j < L_WIN/2; j++)
							sum += data1[i+j]*lFilter[j+L_WIN/2];
						data[i-L_WIN/2] = (int)sum; // low pass                        
					}

					for (int i = L_WIN/2; i < dataBufLen + L_WIN/2; i++)
						data1[i]= data[i-L_WIN/2]; // copy back 
				}
				// Compute the Gaussian Filter Value, i.e. High-pass filter.
				if (useGaussian) 
				{

					for (int i = G_WIN/2; i < dataBufLen + G_WIN/2; i++)
					{
						double sum = 0.0;
						// Gaussian Filter.
						for (int j = - G_WIN/2 ; j < G_WIN/2; j++)
							sum += data1[i+j]*gFilter[j+G_WIN/2];
						data[i-G_WIN/2] = data1[i] - (int)sum; // high pass
                        
					}
                    
				}
				

				if (useMedianFilter) {
					medianFrame ++;
					if (medianFrame < 3) {
						MedianFilter(medianFrame);
						return;
					}
					else MedianFilter(-1);
				}
                
                // Step 2: FFT:  - by Zhu
                //magnitude = new double[dataBufLen];
                //FastFourierTransform.fftMag(data,magnitude);
				
				CalMagArray();
				

                magMax = magMin = 0.0;
                for ( int i = 1; i < dataBufLen; i++)
                {
                    if (magnitude[i] > magMax) magMax = magnitude[i];
                    else if (magnitude[i] < magMin) magMin = magnitude[i];
                }
                
				// No matter what, the following should be done.
				for (int i = 0; i < dataBufLen; i++)
				{
					data1[2*dataBufLen + i] = data1[i]; // Cycling buffer. 
					data1[i] =  data1[i + dataBufLen];
				}
                
                // Step 4: Adaptive Volume if needed.
                if (isAdaptiveVol)  average = AdaptiveVolume(average);
                              
                // Step 5: Playback and display the waveform
                int value = 0;
                if ((drawPos + (dataBufLen / drawTimes)) >= dataBufLen) drawPos = 0;
                //by Liwh: 10/8/04 Fix the drawPos roundup to 0 problem and can't plot.
                yMax = yMin = 0;
                for (int i = 0; i < dataBufLen; i++)
                {
                    value = (short) (volume * data[i]) ;
                    // Sub-Sampling the sample data;
                    if (drawPos == dataBufLen) drawPos = 0;
                    if (i % drawTimes == 0) 
                        drawData[drawPos++] = value ;
                    yMax = (yMax > value) ? yMax : value;
                    yMin = (yMin < value) ? yMin : value;
                }
                
                restLen -= dataBufLen;
				totalFrames ++;
				displayWindow[2] = true;
				dispWinBox[2].setSelected(true);
				RepaintCanvs();

            }
            catch (Exception e) {
                System.err.println("Run failure: " + e.getMessage() + " restLen: " + restLen);
            }
            
        }
    }

	
	public void AllActionPerformed (java.awt.event.ActionEvent evt) {
		if (selectedFileName == null) {
			JOptionPane.showMessageDialog(this, "Please choose file first.");
			return;
		}
		Runnable allWindowDisp = new Runnable () {
			public void run () {
				while (true) {
					autoDrawFullWindow = true;
					StepActionPerformed(null);
					if ((drawPos + (dataBufLen / drawTimes)) >= dataBufLen) 
						return;

					try {
						while (autoDrawFullWindow)
							Thread.sleep(10);  // Nice design.
					} catch (Exception e) { System.out.println("AllAction Thread Error!"); } 
				}
			}
		};
		
		Thread dispThread = new Thread(allWindowDisp);
		dispThread.start();
	}

    public void RealtimeActionPerformed (java.awt.event.ActionEvent evt) {
        realtimeThread = new Thread(this);
        realtimeThread.start();
    }
 
    public void start() {
        realtimeThread.setDaemon(true);
        realtimeThread.start();
    }
    
    public void run() {

        resetBtn.setEnabled(false);
		timeBtn.setEnabled(false);
		stepBtn.setEnabled(false);
        runBtn.setEnabled(false);
        
        reset = false;

        //restLen -= dataBufLen; //Don't need this  --- according to NeedBeSaved function.
		totalFrames ++;
        
        byte shortBytes [] = new byte [2];
        byte bs[] = new byte[4];
        byte bs24[] = new byte [6];
        long startTime = 0, endTime = 0;

        while (restLen > dataBufLen)
        {
            try {
            	if (reset) return;
            	
                if (pause) {
                    Thread.sleep(10);
                    continue;
                }
            

                startTime = System.currentTimeMillis();
				ReadDataFromFileBuffer();

                
                // Step 2: Do Gaussian Filter if needed.
				if (firstFrame) firstFrame = false;

				// Compute the Low-Pass Filter Value.
				if (isNoiseRemoval) { // low pass
					for (int i = 0; i < dataBufLen; i++)
					{
						double sum = 0.0;
						// Gaussian Filter.
						for (int j = - L_WIN/2 ; j < L_WIN/2; j++)
							sum += data1[(i+j+ 3*dataBufLen) % (3*dataBufLen)]*lFilter[j+L_WIN/2];
						data[i] = (int)sum; // low pass                        
					}
	
					for (int i = 0; i < dataBufLen; i++)
						data1[i]= data[i]; // copy back 
				}
				// Compute the Gaussian Filter Value, i.e. High-pass filter.
				if (useGaussian) 
				{
	
					for (int i = 0; i < dataBufLen; i++)
					{
						double sum = 0.0;
						// Gaussian Filter.
						for (int j = - G_WIN/2 ; j < G_WIN/2; j++)
							sum += data1[(i+j+ 3*dataBufLen) % (3*dataBufLen)]*gFilter[j+G_WIN/2];
						data[i] = data1[i] - (int)sum; // high pass
	                        
					}
	                    
				}
                

                if (useMedianFilter) {
                    medianFrame ++;
                    if (medianFrame < 3) {
                        MedianFilter(medianFrame);
                        continue;
                    }
                    else MedianFilter(-1);
                }
                
                // No matter what, the following should be done.
                for (int i = 0; i < dataBufLen; i++)
                {
                	data1[2*dataBufLen + i] = data1[i]; // Cycling buffer. 
                    data1[i] =  data1[i + dataBufLen];
                }
                
                // Step 3: FFT:  - by Zhu
                // If not displaying spectrum, FFT is not needed.
                if (dispWinBox[1].isSelected())
                {
	                magnitude = new double[dataBufLen];
	                for (int i=0; i<dataBufLen; i++) magnitude[i] = 0;
	                FastFourierTransform.fftMag(data,magnitude);
	
	                magMax = magMin = 0.0;
	                for ( int i = 1; i < dataBufLen; i++)
	                {
	                    if (magnitude[i] > magMax) magMax = magnitude[i];
	                    else if (magnitude[i] < magMin) magMin = magnitude[i];
	                }
				}
               
                // Step 4: Adaptive Volume if needed.
                if (isAdaptiveVol)  average = AdaptiveVolume(average);
                
                // Step 5: Playback and display the waveform
                int value = 0;
                if ((drawPos + (dataBufLen / drawTimes)) >= dataBufLen) drawPos = 0;
                //by Liwh: 10/8/04 Fix the drawPos roundup to 0 problem and can't plot.
                yMax = yMin = 0;
                for (int i = 0; i < dataBufLen; i++)
                {
                    switch (bitsPerSample) 
                    { 
						case 8:
							// converted to short for playback.
							value = (short) (volume * data[i] );
							value = (value << 7);
							//value = (short) ( (short)data[i] );
							bs[0] = (byte)value;
							bs[1] = (byte)(value >>> 8);
							sourceDataLine.write(bs, 0, 2);
							
							value -= globalYMax;
							break;

                        case 16:
                            // converted to short for playback.
                            value = (short) (volume * data[i] );
                            //value = (short) ( (short)data[i] );
                            bs[0] = (byte)value;
                            bs[1] = (byte)(value >>> 8);
                            sourceDataLine.write(bs, 0, 2);
                            if (needBeSaved) {    
                                outputProcessedWaveFile.write(bs,0,2);
                                if (channels == 2) 
									outputProcessedWaveFile.write(bs,0,2);
							}
                            break;
                        case 24:
                            // converted to short for playback.
                            value = (short) (volume * data[i]) ;
                            //System.out.println("Value is:" + value);
                            //value = (short) ( (short)data[i] );
                            bs[0] = (byte)value;
                            bs[1] = (byte)(value >>> 8);
                            bs[2] = (byte)(value >>> 16);
                            bs[3] = (byte)(value >>> 24);
                            sourceDataLine.write(bs, 0, 2);
                            break;
                    }
                    
                    // Sub-Sampling the sample data;
                    if (drawPos == dataBufLen) drawPos = 0;
                    if (i % drawTimes == 0) 
                        drawData[drawPos++] = value ;
                    yMax = (yMax > value) ? yMax : value;
                    yMin = (yMin < value) ? yMin : value;
                }
                
                restLen -= dataBufLen;
				totalFrames ++;
                displayWindow[2] = false;
				dispWinBox[2].setSelected(false);
				RepaintCanvs();

                endTime = System.currentTimeMillis();
                Thread.sleep((int)(1000 * dataBufLen /waveFileSampleRate) - (endTime - startTime)); // Based on the sample rate.

            }
            catch (Exception e) {
                System.err.println("Run failure: " + e.getMessage() + " restLen: " + restLen);
            }
            
        }
        
        //Block and wait for internal buffer of the data line to empty.
        //sourceDataLine.drain();
        //sourceDataLine.close();
        
        pauseBtn.setEnabled(false);
        resetBtn.setEnabled(true);
        if (needBeSaved && outputProcessedWaveFile != null)
        {
        	for (int i = 0; i < dataBufLen; i++) {
				short value = (short) (volume * data1[i] );
				bs[0] = (byte)value;
				bs[1] = (byte)(value >>> 8);
				outputProcessedWaveFile.write(bs,0,2);
					if (channels == 2) 
						outputProcessedWaveFile.write(bs,0,2);
        	}
        	
        	while (restLen > 0)
        	{
        		restLen --;
				copyFileStream(input,outputProcessedWaveFile,channelBytes);
        	}
        		
			outputProcessedWaveFile.close();
			JOptionPane.showMessageDialog(this,selectedFileName.substring(0,selectedFileName.length() - 4) + "_new.wav","Processed File Saved At:",JOptionPane.INFORMATION_MESSAGE);
        }

    }

    public double AdaptiveVolume(double average) {
        
        if (bitsPerSample < 16) return 1.0; // 12.10.05 by wli: temporally to avoid the bug of adaptive volumn for 8 bits sample.
        
        //Adaptive method used to adaptively control the volume.
        yMax = yMin = 0;
        for (int i = 0; i < dataBufLen; i++)
        {
            yMax = (yMax > data[i]) ? yMax : data[i];
            yMin = (yMin < data[i]) ? yMin : data[i];
        }
        
        if (yMax > 0 - yMin)
            volume = globalYMax / yMax;
        else volume = globalYMin / yMin;
        
        return volume;  // No use, just for keep the prototype of the function.

    }
/*    
    public double AdaptiveVolume(double average) {
        
        //Adaptive method used to adaptively control the volume.
        double total = 0.0;
        for (int i = 0; i < dataBufLen; i++)
            total += (data[i] > 0 ) ? data[i] : 0 - data[i];

        if (average == 0 ) 
            average =  total / dataBufLen;
        else average =  (average + (total / dataBufLen)) / 2;


        //volume = 200000000 /(average + 1);
        volume = globalYMax / 8 /(average + 1);
        
        return average;

    }
*/

    public void Gaussian(double g1[], double sigma) {

        int size = g1.length;
        
        double SigmaSquare = sigma * sigma;
        
        double sum = 0.0;
        
		for (int i=0; i<size;i++) {
	
			double x = (float)(i-size/2);
	
			g1[i] = Math.exp(0.0- x*x/(2.0*SigmaSquare));
	                sum += g1[i];
	
		}
        
	for (int i=0; i<size;i++) g1[i] /= sum;
        
    }
    
/*
    public void Gaussian(double g1[], double weighted) {

        int size = g1.length;
//        double weighted = 16.0; // 512.0;
        
        double SigmaSquare = (float)(weighted)*(float)(size-1)/4.0 ;

        double sum = 0.0;
        
	for (int i=0; i<size;i++) {

		double x = (float)(i-size/2);

		g1[i] = Math.exp(0.0- x*x/(2.0*SigmaSquare));
                sum += g1[i];

	}
        
	for (int i=0; i<size;i++) g1[i] /= sum;
        
    }
 */
    
    public boolean NoiseRemoval() {
    
        // Entropy method used for removing the noise.
        double sum = 0.0;
        double sumLog = 0.0;
        double entropy = 0.0;
        
        boolean removed = false;
        
        for (int i = 0; i < dataBufLen / 2; i ++)
            sum += magnitude[i];
        
        sumLog = Math.log(sum);
        
        for (int i = 0; i < dataBufLen / 2; i ++)
            entropy += magnitude[i] * (sumLog - Math.log(magnitude[i]) );
        
        entropy /= sum;  // This is the value of entropy
        
        if (entropy >= entropyThres) 
        {
            volume = 0;  // Set volume to 0 for display and playback
            removed = true; // return value
        }
        
        return removed;

    }
    
    public void MicrophoneActionPerformed (java.awt.event.ActionEvent evt) {
        pauseBtn.setEnabled(true);
        pauseBtn.setText("Stop");
        
        Thread captureThread = new Thread (new CaptureThread ());
        captureThread.start();
        
    }

    public void NoiseRemovalActionPerformed (java.awt.event.ActionEvent evt) {
        if (noiseRemovalRBs[0].isSelected()) {
            isNoiseRemoval = true;
            lowPassList.setEnabled(true);
        }
        else {
            isNoiseRemoval = false;
            lowPassList.setEnabled(false);
        }
        
    }
    
    public void GaussianFilterActionPerformed (java.awt.event.ActionEvent evt) {
        if (gaussianFilterRBs[0].isSelected()) {
            useGaussian = true;
            highPassList.setEnabled(true);
        }
        else {
            useGaussian = false;
            highPassList.setEnabled(false);
        }
        
    }

    public void VolumeActionPerformed (java.awt.event.ActionEvent evt) {
        if (volumeRBs[0].isSelected()) {
            isAdaptiveVol = false;
            volumeList.setEnabled(true);
            volume = Double.parseDouble(volumeChoices[volumeList.getSelectedIndex()]);
            
			// Comment the following out, can't see any usefulness. it leads to a bug of BS = 8; 12.20.05 by Liwh
			//if (bitsPerSample != 0) volume /= Math.pow(2,(bitsPerSample - 16)); // 12.13.04 by Liwh.
			//if (bitsPerSample > 16 ) volume /= Math.pow(2,(bitsPerSample - 16)); // 12.10.05 by Liwh.
        }
        else {
            isAdaptiveVol = true;
            volumeList.setEnabled(false);
        }
        
    }

    private AudioFormat getAudioFormat(){
        float sampleRate = 22050.0F;
        //8000,11025,16000,22050,44100
        int sampleSizeInBits = 16;
        //8,16
        int channelNum = 1;
        //1,2
        boolean signed = true;
        //true,false
        boolean bigEndian = false;
        //true,false
        
        if (isWaveFormat) {
            sampleRate = waveFileSampleRate;
			channelNum = 1; // Can only be 1; 3/7/2006
			sampleSizeInBits = bitsPerSample;
            //sampleSizeInBits = 24;
        }
        
        return new AudioFormat(
                          sampleRate,
                          sampleSizeInBits,
						  channelNum,
                          signed,
                          bigEndian);
    }//end getAudioFormat

    private AudioFormat getMicrophoneFormat(){
        float sampleRate = 16000.0F;
        //8000,11025,16000,22050,44100
        // int sampleSizeInBits = 16; MONOBYTE
        int sampleSizeInBits = 16;
        //8,16
        int channels = 1;
        //1,2
        boolean signed = true;
        //true,false
        boolean bigEndian = false;
        //true,false
        return new AudioFormat(
                          sampleRate,
                          sampleSizeInBits,
                          channels,
                          signed,
                          bigEndian);
    }//end getAudioFormat

    // This is an inner class, which is used to capture the data from microphone.
    class CaptureThread extends Thread{
        //An arbitrary-size temporary holding buffer
        //byte tempBuffer[] = new byte[dataBufLen*2]; MONOBYTE
        byte tempBuffer[] = new byte[dataBufLen*2];

        public void run(){
            stopCapture = false;
            notUsingMic = false;
            try{//Loop until stopCapture is set by Stop button.
                while(!stopCapture){
                    //Read data from the internal buffer of the data line.
                    Thread.sleep(10);
                    int cnt = targetDataLine.read(tempBuffer, 0, tempBuffer.length);
                    //if(cnt > dataBufLen-2){ MONOBYTE 
                    if(cnt > dataBufLen*2 - 2){                          
                        int i;
                        //for (i=0; i < cnt; i++) MONOBYTE
                        //yMax = yMin = 0;
                        for (i=0; i < cnt; i=i+2)
                        {
                            data[i/2] = ((tempBuffer[i+1]<<8) + tempBuffer[i]);
                            //data[i] = tempBuffer[i]*100; MONOBYTE
                            if (data[i/2] > yMax) yMax = data[i/2] ;
                            else if (data [i/2] < yMin) yMin = data[i/2];
                        }

						RepaintCanvs();

                        
                    }
                }//end while
                
                notUsingMic = true;

            }catch (Exception e) {
                System.out.println(e);
                System.exit(0);
            }//end catch
        }//end run
    }//end inner class CaptureThread
    
    
    //This is an inner class, which is used to override the paint method on the plotting surface.
    // By Liwh: 9/21/04. Expand the frequencey from 200 - 3200Hz, 
    // The normal VF (Telephone Frequencey) is: 300 - 3400Hz
    // The normal voice range is 500 - 2000Hz
    class MyCanvas extends Canvas{

        int cnt;//object number
        int width;
        int height;
        int arrayPosRoundOff = 0;
        //Factors to convert from double values to integer pixel locations.
        double xScale;
        double yScale;

        MyCanvas(int cnt){//save obj number
            this.cnt = cnt;
        }//end constructor

		public void UseSpeechArray()
		{

			speechFramePos ++;
			isSpeechFrame = isSpeechArray[speechFramePos];
			
			if (!dispWinBox[1].isSelected())
			{
				magnitude = new double[dataBufLen];
				for (int i=0; i<dataBufLen; i++) magnitude[i] = 0;
				FastFourierTransform.fftMag(data,magnitude);
	
			}
	
			if (!isSpeechFrame)
			{
				for (int i=0; i<dataBufLen/2; i++)
				{
					snrNoiseArray[i] += magnitude[i]; // need to be initilized in Reset.
					totalSNRNoiseFrame ++;
				}
			}
			else 
			{
				double snrSumNoise = 0, snrSumSpeech = 0;
				for (int i=0; i<dataBufLen/2; i++)
				{
					double noiseD = snrNoiseArray[i]/totalSNRNoiseFrame;
					double tempD = magnitude[i] - noiseD;
					//snrSpeechArray[i] = tempD > 0? tempD : 0.0;  // Assumed clean speech.
					snrSpeechArray[i] = tempD;  // Assumed clean speech.
					snrSumSpeech += tempD;
					snrSumNoise += noiseD;
				}
				
				snrSpeechFrames ++;
				
				if (snrSumSpeech < 0) snrSumSpeech = 0.00001;
				segSNR += Math.log(snrSumSpeech/snrSumNoise);
				System.out.println("******* total Speech frame #is:" + snrSpeechFrames + "current log:" + Math.log(snrSumSpeech/snrSumNoise));
				System.out.println("******* Segmental SNR is:" + (10.0/snrSpeechFrames)*segSNR);
			}
			
		}
		
		public void CheckSpeechFrame()
		{
			if (speechFramePos > totalFrames - 1)
				return;

			if (useExSpeech)
			{
				UseSpeechArray();
				return;
			}

			speechFramePos ++;			
			// Check the energy strength
			int lowCutoff = 200;
			int highCutoff = 3600; 
			int freqLowNum = (int)((double)lowCutoff/(waveFileSampleRate/2)*(dataBufLen/2)); // The number of mag values for the freqRange.
			int freqHighNum = (int)((double)highCutoff/(waveFileSampleRate/2)*(dataBufLen/2)); // The number of mag values for the freqRange.
			
			System.out.println("speechFramePos:" + speechFramePos + "  totalFrames:" + totalFrames);
			System.out.println("Low freq:" + freqLowNum + "  High freq:" + freqHighNum);
		
			double thresSpeech = 2.0; // a magic number to be determined.
			
			if (!dispWinBox[1].isSelected())
			{
				magnitude = new double[dataBufLen];
				for (int i=0; i<dataBufLen; i++) magnitude[i] = 0;
				FastFourierTransform.fftMag(data,magnitude);
	
			}
			
			// compute the mean
			double ave = 0;
			for (int i = freqLowNum; i < freqHighNum; i++)
				ave += magnitude[i];
			
			ave /= freqHighNum - freqLowNum;
			
			System.out.println("Average energy is:" + ave);
			
			isSpeechFrame = (ave > thresSpeech)? true : false; 
			
			if (!isSpeechFrame)
			{
				isSpeechArray[speechFramePos] =  false;
				for (int i=0; i<dataBufLen/2; i++)
				{
					snrNoiseArray[i] += magnitude[i];
					totalSNRNoiseFrame ++;
				}
			}
			else 
			{
				isSpeechArray[speechFramePos] =  true;
				double snrSumNoise = 0, snrSumSpeech = 0;
				for (int i=0; i<dataBufLen/2; i++)
				{
					double noiseD = snrNoiseArray[i]/totalSNRNoiseFrame;
					double tempD = magnitude[i] - noiseD;
					//snrSpeechArray[i] = tempD > 0? tempD : 0.0;  // Assumed clean speech.
					snrSpeechArray[i] = tempD;  // Assumed clean speech.
					snrSumSpeech += tempD;
					snrSumNoise += noiseD;
				}
				
				snrSpeechFrames ++;
				
				if (snrSumSpeech < 0) snrSumSpeech = 0.00001;
				segSNR += Math.log(snrSumSpeech/snrSumNoise);
				System.out.println("******* total Speech frame #is:" + snrSpeechFrames + "current log:" + Math.log(snrSumSpeech/snrSumNoise));
				System.out.println("******* Segmental SNR is:" + (10.0/snrSpeechFrames)*segSNR);
			}
			
			
		}
		
        //Override the paint method
        public void paint(Graphics g){
            
            // Check this frame is speech or noise;
            // CheckSpeechFrame();
            
            // Display nothing if no file selected.
            if (selectedFileName == null && notUsingMic) return;
            if (!displayWindow[cnt]) return;
            
			//Get and save the size of the plotting surface
			width = canvases[cnt].getWidth();
			height = canvases[cnt].getHeight();

            //Calculate the scale factors
            xScale = width/(xMax-xMin);
            if (useGlobalScale) yScale = height / (globalYMax - globalYMin);
            else yScale = height/(yMax-yMin);

            if (usedBScale) magScale = height / (globalMagMax - globalMagMin);
			else magScale = height / (magMax - magMin);

            //Set the origin based on the minimum values in x and y
            //g.translate((int)((0-xMin)*xScale),(int)((0-yMin)*yScale));
            drawAxes(g);
            g.setColor(fgColor);
            if (usePaperColor) g.setColor(Color.WHITE);


			if (cnt == 2) {
				if (colorArray == null)
				{
					int timeLen = (int)stepLen;
					if (timeLen == 0) timeLen = 1;
					colorArray = new short [height*drawTimes*winNum*timeLen];
				}
				drawFreq(g);
				if (autoDrawFullWindow) autoDrawFullWindow = false;
				return;
			}
			
            //Just a test for the spectrum
            //for(int n = 0;n < dataBufLen;n++)
                //data[n] = 0.5*(yMax - yMin)*Math.sin(2*Math.PI*n*0.02)*Math.sin(2*Math.PI*n*0.005);            
            
            // End of test

            //Get initial data values
            double xVal = xMin;
            int oldX = getTheX(xVal);
            int oldY = 0;
            //Use the Canvas obj number to determine which method to invoke to get the value for y.
            switch(cnt){
                  case 0 :
                    oldY = 0;//getTheY(data.f1(xVal));
                    //g.translate((int)((0-xMin)*xScale),(int)((0-yMin)*yScale));
                    g.translate(0,(int)(height/2));
                    g.setColor(fgColor);
					if (usePaperColor) g.setColor(Color.WHITE);
                    break;
                  case 1 :
                    oldY = 0;//getTheY(data.f2(xVal));
                    g.translate(0,height);
                    break;
            }//end switch

            //Now loop and plot the points
            int count = 0;
            while(xVal < xMax){
                int yVal = 0;
                //Get next data value.  Use the Canvas obj number to determine which method to  invoke to get the value for y.
                int x = getTheX(xVal);
                switch(cnt){
                    case 0 :
                        if (xVal >= drawPos) { xVal = xMax; break; }
                        if (xVal > drawPos - (dataBufLen / drawTimes ) && !usePaperColor)
                        {
							g.setColor(Color.RED);
                        }
                        yVal = getTheY(drawData[(int)xVal]);//getTheY(data.f1(xVal));
                        if (x > 0) g.drawLine(oldX,oldY,x,yVal); 
                        //Increment along the x-axis
                        xVal += 1.0; //go to next data;
                        break;
                    case 1 :
                        // If use the dB as the scale, then, we can't outstand the voice energe easily . by liwh: 9/21/04
                        if (count== 0) xVal = x = oldX = 8;
                        if (usedBScale)
                        	yVal = getThe2ndY(magnitude[count++] > 1 ? 10*Math.log(magnitude[count]) : 0);
                        else yVal = getThe2ndY(magnitude[count++]);
                        g.drawLine(oldX,0,oldX,yVal);
                        xVal += 8.0;
                        break;
                  }//end switch1

                  //Save end point to use as start point for next line segment.
                  oldX = x;
                  oldY = yVal;
            }//end while loop

        }//end overridden paint method
        //---------------------------------//
        
		//LUP for drawing.  More visible.
		void xPosLUT(int p[]) {
			int length = p.length;
			int total = getTheX(dataBufLen/drawTimes);
			p[0] =  total / length;
			int oldV = p[0];
			int newV;
			for (int i=1; i<length; i++)
			{
				total = total - p[i-1];
				newV = total / (length - i);
				if (newV > oldV) oldV = newV;
				p[i] = p[i-1] + oldV; 
			} 
			
		}
		
        //Method to draw the freqency strength
        // Think about using linear interpolation. now NN
        void drawFreq(Graphics g) {
        	int count = 0;
			int y = (int)((double)height/yTicNum); // The number of pixels between 1KHz
			int freqNum = (int)((double)freqRange/(waveFileSampleRate/2)*(dataBufLen/2)); // The number of mag values for the freqRange.
			double yVal = 0.0;
			int cIndex = 0;
			int arrayPos = drawPos / (dataBufLen/drawTimes);
			if (arrayPos > 1) arrayPos --; // for the boundary case.
			else arrayPos = 0;
			if (arrayPos - arrayPosRoundOff == 2){ // Round off error.
				arrayPos --; 
			}
			arrayPosRoundOff = arrayPos;

			int xPos[] = new int[winNum];
			xPosLUT(xPos);			

        	Color tmpC = null;	
        
        	for (int i=0; i < height; i++)
        	{
				count = (int)((double)(freqNum * i)/height); // The corresponding index of the height.
				for (int num=0; num < winNum; num++)
				{
					yVal = magArray[num*dataBufLen+count] > 1 ? 10*Math.log(magArray[num*dataBufLen+count]) : 0;
					cIndex = (int)(yVal/globalMagMax * 255);
					if (cIndex > 255) cIndex = 255;  // By liwh 12/12/04.
					if (cIndex < 0) cIndex = 0;
					colorArray[i+(num+ winNum*arrayPos)*height] = (short)cIndex;
				}
        	}

			//int startPos = getTheX((dataBufLen/drawTimes)*i);
			int startPos = 0;
			int offset = 0;
			for (int i=0; i<arrayPos; i++)
        	{ 
        		for (int j=0; j<height; j++)
        		{
        			offset = 0;
        			for (int num=0; num<winNum; num++)
        			{
        				int k = colorArray[(i*winNum+num)*height+j];
        				tmpC = new Color(255-k,255-k,255-k);
        				g.setColor(tmpC);
						g.drawLine(getTheX(startPos+offset),-j,getTheX(startPos + xPos[num]),-j);
						offset = xPos[num];
        			}
        		}
        		startPos += offset;
        	}
        	
			g.setColor(Color.WHITE);
			if (usePaperColor) g.setColor(Color.WHITE);
			yTics(g);
        }

        //Method to draw axes with tic marks and labels in the color RED
        void drawAxes(Graphics g){
            g.setColor(Color.BLACK);
			if (usePaperColor) g.setColor(Color.WHITE);

            switch (cnt) 
            {
                case 0:
                    g.drawString("" + (long)yMin, 1152, 15);
                    g.drawString("" + (long)yMax, 1152, 150);
					g.drawString("" + (long)(totalFrames*(1000000/waveFileSampleRate)) + " msec.", 1152, 175);

                    //Draw the axes
                    g.drawLine(0,height/2,getTheX(xMax),height/2);

                break;
                case 1:
                    //Draw the tic marks on axes
                    xTics(g);
                    //yTics(g);
                break;
				case 2:
					//Draw the tic marks on axes
					//xTics(g);
					g.translate(0,height);
					yTics(g);
					//yTics(g);
				break;
            }
        }//end drawAxes

        //---------------------------------//

        //Method to draw tic marks on x-axis
        void xTics(Graphics g){
            double xDoub = 0;
            int x = 0;

            //Get the ends of the tic marks.
            int topEnd = getTheY(xTicLen/2);
            int bottomEnd = getTheY(-xTicLen/2);

            //If the vertical size of the plotting area is small, the calculated tic size may be too small.  
            //In that case, set it to 10 pixels.
            if(topEnd < 5){
              topEnd = 5;
              bottomEnd = -5;
            }//end if

            //Loop and draw a series of short lines to serve as tic marks. Begin with the positive x-axis moving to the right from zero.
            int count = 0;
            while(xDoub < xMax){
	          x = getTheX(xDoub);
	          g.drawLine(x,topEnd,x,bottomEnd);
	          if (count == 0) 
	              g.drawString(" 0(Hz)", x, 15); // By Liwh : 9/21/04
	          else g.drawString("" + 250*count, x, 15); // By Liwh : 9/21/04
	          
	          xDoub += xTicInt;
	          count++;
            }//end while

        }//end xTics
        //---------------------------------//

        //Method to draw tic marks on y-axis
        void yTics(Graphics g){
            int y = 0;
            int disp = 5;
            int rightEnd = 5;
            int leftEnd = 0;

			int count = 0;
			while(count <= yTicNum){
				g.drawLine(rightEnd,y-1,leftEnd,y-1);
				if (!usePaperColor) {
				  	if (count == 0) 
					  	g.drawString("0(KHz)",rightEnd+3, y-disp); 
				  	else g.drawString("" + count, rightEnd+3, y+disp+3); 
				}
              
				count++;
				y -= (int)((double)height/yTicNum);
			}//end while

        }//end yTics

        //---------------------------------//

        //This method translates and scales a double y value to plot properly in the integer coordinate system.
        // In addition to scaling, it causes the positive direction of the y-axis to be from bottom to top.
        int getTheY(double y){
            //double yDoub = (yMax+yMin)-y;
            //int yInt = (int)(yDoub*yScale);
            int yInt = (int)(y*yScale);
            return yInt;
        }//end getTheY
        //---------------------------------//
        int getThe2ndY(double y){
            double yDoub = magMin - y;
            int yInt = (int)(yDoub*magScale);
            return yInt;
        }//end getTheY
        //---------------------------------//

        //This method scales a double x value to plot properly in the integer coordinate system.
        int getTheX(double x){
            return (int)(x*xScale);
        }//end getTheX
        //---------------------------------//

    }//end inner class MyCanvas
    //===================================//
    
    
}


