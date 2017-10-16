/*
 * Configuration.java
 *
 * Created on 2004年7月27日, 下午4:08
 */

package LDVProject;

/**
 *
 * @author  Weihong Li
 */

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.*;
import javax.media.*;
import javax.comm.*;

public class Configuration extends javax.swing.JPanel implements Runnable{
    private JTabbedPane tabPane;
    private LDVProject ldvProject;
    char lf[] = {10};
    String lfChar = new String (lf);
    
    String setControllerPowerUp[] = {"Default", "Last", "User1", "User2", "User3"};
    String setControllerRemote[] = {"Local", "LLO"};
    String setControllerReset[] = {"1"};
    
    String setInterfaceEcho[] = {"Off", "On"};
    String setInterfaceBaudRate[] = {"115200", "57600", "19200", "9600"};

    String setDisplayPage[] = {"Settings", "VelocityOutput", "VelocityDSPOut", "DisplacementOutput", "AuxiliaryOutput"};
    
    String setSensorHeadFocus[] = {"0", "1", "3300"};
    String setSensorHeadManualFocus[] = {"Unlocked", "Locked"};
    String setSensorHeadRemoteFocus[] = {"Stop", "Far", "Near"};
    String setSensorHeadAutoFocus[] = {"Search", "Stop"};
    
    String setVeloDecRange[] = {"1 mm/s/V", "50 mm/s/V", "50 mm/s/V LP", "10 mm/s/V", "10 mm/s/V LP", "2 mm/s/V", "2 mm/s/V LP", "Off"};

    String setDisplDecRange[] = {"10240 ?m/V", "5120 ?m/V", "2560 ?m/V","1280 ?m/V", "640 ?m/V", "320 ?m/V", "160 ?m/V", "80 ?m/V"};
    String setDisplDecReset[] = {"1"};
    
    String setVeloFilterLowPassRange[] = {"20 kHz", "100 kHz", "5 kHz", "Off"};
    String setVeloFilterHighPassRange[] = {"100 Hz", "Off"};
    
    String setTrackingFilterRange[] = {"Slow", "Fast", "Off"};
    
    String setDigitalOutDataRate[] = {"48 kSa/s", "96 kSa/s", "Off"};
    String setDigitalOutLeftChannelData[] = {"Velocity", "Velocity (DSP)"};
    String setDigitalOutRightChannelData[] = {"Velocity", "Displacement"};
    
    JComboBox valueList = null;
    String valueListAttrStr = "ControllerPowerUp";
    String settingStrs[];
    
	PrintStream output = null;
	boolean startRefreshRecording = false;

    
    /** Creates a new instance of Configuration */
    public Configuration(JTabbedPane parentTabPane, LDVProject ldv) {
        tabPane = parentTabPane;
        ldvProject = ldv;
        new Thread(this).start();
    }
    
    // This is the new thread.
    public void run () {
        EmptyBorder eb = new EmptyBorder(5,5,5,5);
        BevelBorder bb = new BevelBorder(BevelBorder.LOWERED);
        CompoundBorder cb = new CompoundBorder(eb,bb);        
        JPanel p = new JPanel(new BorderLayout());
        p.setBorder(new CompoundBorder(cb,new EmptyBorder(0,0,30,0)));
        Controller crtl = new Controller ();
        p.add(crtl);
        tabPane.addTab("Configuration",p);

		tabPane.setMnemonicAt(tabPane.indexOfTab("Audio Panel"),KeyEvent.VK_1);
		tabPane.setMnemonicAt(tabPane.indexOfTab("Configuration"),KeyEvent.VK_2);
        
    }
    
    public class Controller extends javax.swing.JPanel implements Runnable {
    	
		Thread saveThread = null;
		
        JPanel commandPanel = new JPanel();
        JPanel devicePanel = new JPanel();
        JPanel numberPanel = new JPanel();
        JPanel attributePanel = new JPanel();
        JPanel valuePanel = new JPanel();
        
        JPanel cmdTextPanel;
        JTextField cmdText;
        JButton sendBtn;
        JButton clearBtn;
        
        JLabel t = null;
        JLabel t1 = null, t2 = null;
        ButtonGroup radioGroup;
        JRadioButton cmdGetDevInfo, cmdGet, cmdSet;
        JRadioButton devController, devInterface, devDisplay, devSensorHead, devVeloDec,
                     devDisplDec,devVeloFilter,devTrackingFilter, devSignalLevel, devOverRange, devDigitalOut;
        JRadioButton number0, number1, number2;
        JRadioButton attr0, attr1, attr2, attr3, attr4, attr5, attr6;
        JTextArea valueText;
        JScrollPane valueScrollPane;

		int preWidth = 230;
		int preHeight = 350;
		int disp = 50;

        String cmdStr = "GetDevInfo";
        String devStr = "Controller";
        String numStr = "0";
        String attStr = "Name";
        String setStr = "";
        String messageStr = "Null";
        String resultStr = "";
        
        SerialPortComm serialPortComm = new SerialPortComm();
        SerialPort serialPort;
        
        public Controller () {
            //setLayout(new FlowLayout());
            setLayout(new BorderLayout(0,20));

            // The following is for Value Panel;
            JPanel ctlPnl1 = new JPanel();
            ctlPnl1.setLayout(new BoxLayout(ctlPnl1, BoxLayout.Y_AXIS));

            t = new JLabel("The Value:");
            t1 = new JLabel(" ");
            t2 = new JLabel(" ");
            valueText = new JTextArea(25,20);
            valueText.setFont( new Font("TimesRoman", Font.BOLD|Font.ITALIC, 14));
            valueText.setText("OFV-5000 Vibrometer Controller");
            valueText.setLineWrap(true);
            valueText.setEditable(false);
            valueScrollPane = new JScrollPane(valueText);
            
            clearBtn = new JButton ("Clear");
            AbstractAction clearAction = new AbstractAction() {
                public void actionPerformed (java.awt.event.ActionEvent evt) {
                    resultStr = "";
                    valueText.setText("");
					clearBtn.grabFocus();
                }
            };

            settingStrs = new String[setControllerPowerUp.length];
            System.arraycopy(setControllerPowerUp, 0, settingStrs, 0, setControllerPowerUp.length);
            valueList = new JComboBox ( settingStrs );
            valueList.setMaximumRowCount(8);
            valueList.addItemListener ( new java.awt.event.ItemListener() {
                public void itemStateChanged(ItemEvent evt)
                {
                    if (valueList.getSelectedIndex() == -1 ) return;
                    setStr = ", " + settingStrs[valueList.getSelectedIndex()];
                    cmdText.setText(cmdStr + ", " + devStr + ", " + numStr + ", " + attStr + setStr);
                }
            });
            valueList.setVisible(false);
            
            ctlPnl1.add(t);
            ctlPnl1.add(valueList);
			ctlPnl1.add(t1);
			ctlPnl1.add(valueScrollPane);
			ctlPnl1.add(t2);
			ctlPnl1.add(clearBtn);
            //ctlPnl1.add(valueText);
            
            valuePanel.add(ctlPnl1);
            valuePanel.setPreferredSize(new Dimension(preWidth + disp,preHeight));

            // The following is for Attribute Panel;
            JPanel ctlPnl2 = new JPanel();
            ctlPnl2.setLayout(new BoxLayout(ctlPnl2, BoxLayout.Y_AXIS));

            t = new JLabel("Select the Attribute:");
            t1 = new JLabel(" ");
            attr0 = new JRadioButton("Name",true);
            attr1 = new JRadioButton("Version",false);
            attr2 = new JRadioButton("PowerUp",false);
            attr3 = new JRadioButton("Remote",false);
            attr4 = new JRadioButton("Attribute 5",false);
            attr4.setVisible(false);
            attr5 = new JRadioButton("Attribute 6",false);
            attr5.setVisible(false);
            attr6 = new JRadioButton("Attribute 7",false);
            attr6.setVisible(false);
            radioGroup = new ButtonGroup();
            radioGroup.add(attr0);
            radioGroup.add(attr1);
            radioGroup.add(attr2);
            radioGroup.add(attr3);
            radioGroup.add(attr4);
            radioGroup.add(attr5);
            radioGroup.add(attr6);

            ctlPnl2.add(t);
            ctlPnl2.add(t1);
            ctlPnl2.add(attr0);
            ctlPnl2.add(attr1);
            ctlPnl2.add(attr2);
            ctlPnl2.add(attr3);
            ctlPnl2.add(attr4);
            ctlPnl2.add(attr5);
            ctlPnl2.add(attr6);
            
            attributePanel.add(ctlPnl2);
            attributePanel.setPreferredSize(new Dimension(preWidth,preHeight));
            //End of create the command radio buttons.

            // The following is for Number Panel;
            JPanel ctlPnl3 = new JPanel();
            ctlPnl3.setLayout(new BoxLayout(ctlPnl3, BoxLayout.Y_AXIS));

            t = new JLabel("Select the Number:");
            t1 = new JLabel(" ");
            number0 = new JRadioButton("Number 0",true);
            number1 = new JRadioButton("Number 1",false);
            number2 = new JRadioButton("Number 2",false);
            number1.setEnabled(false);
            number2.setEnabled(false);
            radioGroup = new ButtonGroup();
            radioGroup.add(number0);
            radioGroup.add(number1);
            radioGroup.add(number2);

            ctlPnl3.add(t);
            ctlPnl3.add(t1);
            ctlPnl3.add(number0);
            ctlPnl3.add(number1);
            ctlPnl3.add(number2);

            numberPanel.add(ctlPnl3);
            numberPanel.setPreferredSize(new Dimension(preWidth - disp,preHeight));
            
            // The following is for Device Classes Panel;
            JPanel ctlPnl4 = new JPanel();
            ctlPnl4.setLayout(new BoxLayout(ctlPnl4, BoxLayout.Y_AXIS));
            //ctlPnl.setBorder(new EtchedBorder());

            t = new JLabel("Select the Device:");
            t1 = new JLabel(" ");
            devController = new JRadioButton("Controller",true);
            devInterface = new JRadioButton("Interface",false);
            devDisplay = new JRadioButton("Display on the font panel",false);
            devSensorHead = new JRadioButton("Sensor head",false);
            devVeloDec = new JRadioButton("Velocity decoder",false);
            devDisplDec = new JRadioButton("Displacement decoder",false);
            devVeloFilter = new JRadioButton("Velocity filter",false);
            devTrackingFilter = new JRadioButton("Tracking filter",false);
            devSignalLevel = new JRadioButton("Signal level",false);
            devOverRange = new JRadioButton("Overrange display",false);
            devDigitalOut = new JRadioButton("Digital Output",false);

            radioGroup = new ButtonGroup();
            radioGroup.add(devController);
            radioGroup.add(devInterface);
            radioGroup.add(devDisplay);
            radioGroup.add(devSensorHead);
            radioGroup.add(devVeloDec);
            radioGroup.add(devDisplDec);
            radioGroup.add(devVeloFilter);
            radioGroup.add(devTrackingFilter);
            radioGroup.add(devSignalLevel);
            radioGroup.add(devOverRange);
            radioGroup.add(devDigitalOut);

            ctlPnl4.add(t);
            ctlPnl4.add(t1);
            ctlPnl4.add(devController);
            ctlPnl4.add(devInterface);
            ctlPnl4.add(devDisplay);
            ctlPnl4.add(devSensorHead);
            ctlPnl4.add(devVeloDec);
            ctlPnl4.add(devDisplDec);
            ctlPnl4.add(devVeloFilter);
            ctlPnl4.add(devTrackingFilter);
            ctlPnl4.add(devSignalLevel);
            ctlPnl4.add(devOverRange);
            ctlPnl4.add(devDigitalOut);

            devicePanel.add(ctlPnl4);
            devicePanel.setPreferredSize(new Dimension(preWidth + disp,preHeight));

            // The following is for Command Panel;
            JPanel ctlPnl5 = new JPanel();
            ctlPnl5.setLayout(new BoxLayout(ctlPnl5, BoxLayout.Y_AXIS));

            //Button for command
            t = new JLabel("Select the Command:");
            t1 = new JLabel(" ");
            cmdGetDevInfo = new JRadioButton("GetDevInfo",true);
            cmdGet = new JRadioButton("Get",false);
            cmdSet = new JRadioButton("Set",false);
            radioGroup = new ButtonGroup();
            radioGroup.add(cmdGetDevInfo);
            radioGroup.add(cmdGet);
            radioGroup.add(cmdSet);

            ctlPnl5.add(t);
            ctlPnl5.add(t1);
            ctlPnl5.add(cmdGetDevInfo);
            ctlPnl5.add(cmdGet);
            ctlPnl5.add(cmdSet);

            commandPanel.add(ctlPnl5);
            commandPanel.setPreferredSize(new Dimension(preWidth - disp,preHeight));

            JSplitPane attributePane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, 
                attributePanel, valuePanel);
            attributePane.setContinuousLayout(true);
       
            JSplitPane numberPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, 
                numberPanel, attributePane);
            numberPane.setContinuousLayout(true);
                 
            JSplitPane devicePane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, 
                devicePanel, numberPane);
            devicePane.setContinuousLayout(true);

            JSplitPane commandPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, 
                commandPanel, devicePane);
            commandPane.setContinuousLayout(true);
            add(commandPane);
            
            cmdTextPanel = new JPanel(new FlowLayout(FlowLayout.CENTER,30,0));
            cmdText = new JTextField();
            cmdText.setFont( new Font("TimesRoman", Font.BOLD, 14));
            cmdText.setText("GetDevInfo, Controller, 0, Name");
            cmdText.setPreferredSize(new Dimension(500,27));
            sendBtn = new JButton ("Send");
            AbstractAction sendAction = new AbstractAction() {
                public void actionPerformed (java.awt.event.ActionEvent evt) {
                    messageStr = cmdText.getText();
                    messageStr = messageStr + lfChar;
                    sendBtn.grabFocus();
                    serialPortComm.write(messageStr);
                }
            };
			AbstractAction autoFocusAction = new AbstractAction() {
				public void actionPerformed (java.awt.event.ActionEvent evt) {
					messageStr = "Set, SensorHead, 0, AutoFocus, Search" + lfChar;
					serialPortComm.write(messageStr);
				}
			};
			AbstractAction signalAction = new AbstractAction() {
				public void actionPerformed (java.awt.event.ActionEvent evt) {
					//resultStr += "55555555\n";  Just for test purpose
					//valueText.setText(resultStr);
					messageStr = "Get, SignalLevel, 0, Value" + lfChar;
					serialPortComm.write(messageStr);
				}
			};
			AbstractAction saveSignalAction = new AbstractAction() {
				public void actionPerformed (java.awt.event.ActionEvent evt) {
					SaveSignalPerformed(evt);
				}
			};
			AbstractAction stopSignalAction = new AbstractAction() {
				public void actionPerformed (java.awt.event.ActionEvent evt) {
					StopSaveSignalPerformed(evt);
				}
			};
			AbstractAction helpAction = new AbstractAction() {
				public void actionPerformed (java.awt.event.ActionEvent evt) {
					ShowHelpMessagePerformed(evt);
				}
			};
			AbstractAction initAction = new AbstractAction() {
				public void actionPerformed (java.awt.event.ActionEvent evt) {
					try {
						messageStr = "Set, VeloDec, 0, Range, 1 mm/s/V" + lfChar;
						serialPortComm.write(messageStr);
						Thread.sleep(100);
						messageStr = "Set, VeloFilter, 0, LowPassRange, 20 kHz" + lfChar;
						serialPortComm.write(messageStr);
						Thread.sleep(100);
						messageStr = "Set, VeloFilter, 0, HighPassRange, 100 Hz" + lfChar;
						serialPortComm.write(messageStr);
						Thread.sleep(100);
						messageStr = "Set, TrackingFilter, 0, Range, Slow" + lfChar;
						serialPortComm.write(messageStr);
						Thread.sleep(100);
						messageStr = "Set, DigitalOut, 0, DataRate, 48 kSa/s" + lfChar;
						serialPortComm.write(messageStr);
						System.err.println("System initialized successfully!!!");
					}catch (Exception e) {
						System.err.println("System initialized failed!!!");
					}
				}
			};
			
            cmdTextPanel.add(cmdText);
            cmdTextPanel.add(sendBtn);
            add("South",cmdTextPanel);
            
			InputMap inputMap = getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW);
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0),  "SEND");
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),  "CLEAR");
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0),  "HELP");
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F2, 0),  "AUTOFOCUS");
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F5, 0),  "SIGNAL");
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F9, 0),  "SAVESIGNAL");
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F10, 0),  "STOPSAVESIGNAL");
			inputMap.put( KeyStroke.getKeyStroke(KeyEvent.VK_F12, 0),  "INIT");
			ActionMap actionMap = getActionMap();
			actionMap.put("SEND", sendAction);
			actionMap.put("CLEAR", clearAction);
			actionMap.put("HELP", helpAction);
			actionMap.put("AUTOFOCUS", autoFocusAction);
			actionMap.put("SIGNAL", signalAction);
			actionMap.put("STOPSAVESIGNAL", stopSignalAction);
			actionMap.put("SAVESIGNAL", saveSignalAction);
			actionMap.put("INIT", initAction);
			sendBtn.setAction(sendAction);
			clearBtn.setAction(clearAction);
			clearBtn.setText("Clear");
			sendBtn.setText("Send");			// Don't know why?    11/12/04
			// Add the Listener to the buttons
            RadioButtonHandler handler = new RadioButtonHandler ();
            cmdGetDevInfo.addItemListener(handler);
            cmdGet.addItemListener(handler);
            cmdSet.addItemListener(handler);
            devController.addItemListener(handler);
            devInterface.addItemListener(handler);
            devDisplay.addItemListener(handler);
            devSensorHead.addItemListener(handler);
            devVeloDec.addItemListener(handler);
            devDisplDec.addItemListener(handler);
            devVeloFilter.addItemListener(handler);
            devTrackingFilter.addItemListener(handler);
            devSignalLevel.addItemListener(handler);
            devOverRange.addItemListener(handler);
            devDigitalOut.addItemListener(handler);
            number0.addItemListener(handler);
            number1.addItemListener(handler);
            number2.addItemListener(handler);
            attr0.addItemListener(handler);
            attr1.addItemListener(handler);
            attr2.addItemListener(handler);
            attr3.addItemListener(handler);
            attr4.addItemListener(handler);
            attr5.addItemListener(handler);
            attr6.addItemListener(handler);
            
        }

		public void StopSaveSignalPerformed (java.awt.event.ActionEvent evt) {

			if (!startRefreshRecording) {
				JOptionPane.showMessageDialog(this,"Not start recording yet!!!");
				return;
			}
			
			startRefreshRecording = false;
		}
		
		public void SaveSignalPerformed (java.awt.event.ActionEvent evt) {
			saveThread = new Thread(this);
			saveThread.start();
		}
		
		public void run() {
			String time = (new Date(System.currentTimeMillis())).toString();
			time = time.replace(':','-');
			
			if (startRefreshRecording) {
				JOptionPane.showMessageDialog(this,"Already started recording!!!");
				return;
			}
			
			startRefreshRecording = true;
			String processedFileName = ldvProject.getCD() + "\\" + time + ".txt";
			try {
				output = new PrintStream (new FileOutputStream(processedFileName));
				output.println(time);
				resultStr = "";

				while (startRefreshRecording) {

					messageStr = "Get, SignalLevel, 0, Value" + lfChar;
					serialPortComm.write(messageStr);
				
					Thread.sleep(500);
				}
				
				output.print(resultStr);
				output.close();
				JOptionPane.showMessageDialog(this, processedFileName, "File Saved To:", JOptionPane.INFORMATION_MESSAGE );
				
			} catch (Exception e) {System.out.println("File write error to " + processedFileName); }

		}
		
		public void ShowHelpMessagePerformed (java.awt.event.ActionEvent evt) {

			String showMessage = 
				"Alt+1: Choose the wave form tab. \n" +
				"Alt+2: Choose the configuration tab. \n\n" +
				"\tIn this configuration tab: \n" +
				"\t\tESC: Clear button. \n" +
				"\t\tF1: Popup this help dialog. \n" +
				"\t\tF2: AutoFocus. \n" +
				"\t\tF5: Get signal strength. \n" +
				"\t\tF9: Save signal strength values to file. \n" +
				"\t\tF10: Stop saving signal strength. \n" +
				"\t\tF12: Send commands for default parameters. \n\n" +
				"\tIn wave form tab: \n" +
				"\t\tESC: Reset button. \n" +
				"\t\tF1: Popup help dialog. \n" +
				"\t\tF3: Flip colors for display and paper versios. \n" +
				"\t\tF4: Crop a sound clip. \n" +
				"\t\tF7: Compute the average return signal strength. \n" +
				"\t\tF8: Compute the average magnitude. \n" +
				"\t\t O: Open files (or Ctrl + O).  \n" +
				"\t\t S: Step button. \n" +
				"\t\t R: Run button. \n" +
				"\t\t P: Pause button. \n" +
				"\t\t T: Set time button. \n" +
				"\t\t A: Automatically display A full Window. \n";
				
	
			JOptionPane.showMessageDialog(this,showMessage,"Short-cut Keyboard Mapping",JOptionPane.INFORMATION_MESSAGE);
		}

        public void enableNumber12 () {
            number1.setEnabled(true);
            number2.setEnabled(true);
        }
        
        public void disableNumber12 () {
            numStr = "0";
            number0.setSelected(true);
            number1.setEnabled(false);
            number2.setEnabled(false);
        }

        public void setValuesforJComboBox(String [] src) {
            settingStrs = new String [ src.length ];
            System.arraycopy(src,  0, settingStrs,  0, src.length);
            for (int i=0; i < src.length; i++)
                valueList.insertItemAt(src[i], i);
           
            valueList.setSelectedIndex(0);
        }
        
        
        public void showSetting () {
            
            if (cmdStr != "Set" || valueListAttrStr == devStr + attStr)  return;
            
            valueListAttrStr = devStr + attStr;

            if (devStr == "Controller" && attStr == "PowerUp" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setControllerPowerUp);
            } else if (devStr == "Controller" && attStr == "Remote" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setControllerRemote);
                
            } else if (devStr == "Controller" && attStr == "Reset" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setControllerReset);
            } else if (devStr == "Interface" && attStr == "Echo" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setInterfaceEcho);
            } else if (devStr == "Interface" && attStr == "BaudRate" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setInterfaceBaudRate);
            } else if (devStr == "Display" && attStr == "Page" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setDisplayPage);
            } else if (devStr == "SensorHead" && attStr == "Focus" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setSensorHeadFocus);
            } else if (devStr == "SensorHead" && attStr == "ManualFocus" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setSensorHeadManualFocus);
            } else if (devStr == "SensorHead" && attStr == "RemoteFocus" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setSensorHeadRemoteFocus);
            } else if (devStr == "SensorHead" && attStr == "AutoFocus" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setSensorHeadAutoFocus);
            } else if (devStr == "VeloDec" && attStr == "Range" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setVeloDecRange);
            } else if (devStr == "DisplDec" && attStr == "Range" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setDisplDecRange);
            } else if (devStr == "DisplDec" && attStr == "Reset" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setDisplDecReset);
            } else if (devStr == "VeloFilter" && attStr == "LowPassRange" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setVeloFilterLowPassRange);
            } else if (devStr == "VeloFilter" && attStr == "HighPassRange" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setVeloFilterHighPassRange);
            } else if (devStr == "TrackingFilter" && attStr == "Range" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setTrackingFilterRange);
            } else if (devStr == "DigitalOut" && attStr == "DataRate" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setDigitalOutDataRate);
            } else if (devStr == "DigitalOut" && attStr == "LeftChannelData" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setDigitalOutLeftChannelData);
            } else if (devStr == "DigitalOut" && attStr == "RightChannelData" ) {
                valueList.removeAllItems();
                setValuesforJComboBox(setDigitalOutRightChannelData);
            }
            
                
            
        }

        public void setAttribute () {
            attr0.setVisible(false);
            attr1.setVisible(false);
            attr2.setVisible(false);
            attr3.setVisible(false);
            attr4.setVisible(false);
            attr5.setVisible(false);
            attr6.setVisible(false);
            
            if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "Controller"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr3.setVisible(true);
                attr0.setText("Name");
                attr1.setText("Version");
                attr2.setText("PowerUp");
                attr3.setText("Remote");
                attr0.setSelected(true);
                attStr = "Name";
            }
            else if (cmdStr == "Set" && devStr == "Controller"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr0.setText("PowerUp");
                attr1.setText("Remote");
                attr2.setText("Reset");
                attr0.setSelected(true);
                attStr = "PowerUp";
            }
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "Interface"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr0.setText("Name");
                attr1.setText("Echo");
                attr2.setText("BaudRate");
                attr0.setSelected(true);
                attStr = "Name";
            }
            else if (cmdStr == "Set" && devStr == "Interface"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr0.setText("Echo");
                attr1.setText("BaudRate");
                attr0.setSelected(true);
                attStr = "Echo";
            }
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "Display"){
                attr0.setVisible(true);
                attr0.setText("Page");
                attr0.setSelected(true);
                attStr = "Page";
            }
            else if (cmdStr == "Set" && devStr == "Display"){
                attr0.setVisible(true);
                attr0.setText("Page");
                attr0.setSelected(true);
                attStr = "Page";
            }
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "SensorHead"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr3.setVisible(true);
                attr4.setVisible(true);
                attr5.setVisible(true);
                attr6.setVisible(true);
                attr0.setText("Name");
                attr1.setText("Version");
                attr2.setText("Focus");
                attr3.setText("ManualFocus");
                attr4.setText("RemoteFocus");
                attr5.setText("AutoFocus");
                attr6.setText("AutoFocusResult");
                attr0.setSelected(true);
                attStr = "Name";
            }
            else if (cmdStr == "Set" && devStr == "SensorHead"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr3.setVisible(true);
                attr0.setText("Focus");
                attr1.setText("ManualFocus");
                attr2.setText("RemoteFocus");
                attr3.setText("AutoFocus");
                attr3.setSelected(true);
                attStr = "AutoFocus";
                setStr = ", Search";
            }
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "VeloDec"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr3.setVisible(true);
                attr0.setText("Name");
                attr1.setText("Range");
                attr2.setText("MaxFreq");
                attr3.setText("Active");
                attr0.setSelected(true);
                attStr = "Name";
            }
            else if (cmdStr == "Set" && devStr == "VeloDec"){
                attr0.setVisible(true);
                attr0.setText("Range");
                attr0.setSelected(true);
                attStr = "Range";
                setStr = ", 1 mm/s/V";
            }//VeloDec
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "DisplDec"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr3.setVisible(true);
                attr4.setVisible(true);
                attr0.setText("Name");
                attr1.setText("Range");
                attr2.setText("Resolution");
                attr3.setText("MaxFreq");
                attr4.setText("Active");
                attr0.setSelected(true);
                attStr = "Name";
            }
            else if (cmdStr == "Set" && devStr == "DisplDec"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr0.setText("Range");
                attr1.setText("Reset");
                attr0.setSelected(true);
                attStr = "Range";
            }//DisplDec
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "VeloFilter"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr0.setText("Name");
                attr1.setText("LowPassRange");
                attr2.setText("HighPassRange");
                attr0.setSelected(true);
                attStr = "Name";
            }
            else if (cmdStr == "Set" && devStr == "VeloFilter"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr0.setText("LowPassRange");
                attr1.setText("HighPassRange");
                attr0.setSelected(true);
                attStr = "LowPassRange";
                setStr = ", 20 KHz";
            }//VeloFilter
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "TrackingFilter"){
                attr0.setVisible(true);
                attr0.setText("Range");
                attr0.setSelected(true);
                attStr = "Range";
            }
            else if (cmdStr == "Set" && devStr == "TrackingFilter"){
                attr0.setVisible(true);
                attr0.setText("Range");
                attr0.setSelected(true);
                attStr = "Range";
                setStr = ", Slow";
            }//TrackingFilter
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "SignalLevel"){
                attr0.setVisible(true);
                attr0.setText("Value");
                attr0.setSelected(true);
                attStr = "Value";
            }//SignalLevel
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "OverRange"){
                attr0.setVisible(true);
                attr0.setText("Value");
                attr0.setSelected(true);
                attStr = "Value";
            }//OverRange
            else if ((cmdStr == "GetDevInfo" || cmdStr == "Get") && devStr == "DigitalOut"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr3.setVisible(true);
                attr0.setText("Name");
                attr1.setText("DataRate");
                attr2.setText("LeftChannelData");
                attr3.setText("RightChannelData");
                attr0.setSelected(true);
                attStr = "Name";
            }
            else if (cmdStr == "Set" && devStr == "DigitalOut"){
                attr0.setVisible(true);
                attr1.setVisible(true);
                attr2.setVisible(true);
                attr0.setText("DataRate");
                attr1.setText("LeftChannelData");
                attr2.setText("RightChannelData");
                attr0.setSelected(true);
                attStr = "DataRate";
                setStr = ", 48 KSa/S";
            }//DigitalOut

            // Set the avalaible setting value;
            if (cmdStr == "Set")
            {
                clearBtn.setVisible(false);
                valueText.setVisible(false);
                valueList.setVisible(true);
                showSetting();
            }
            else 
            {
                clearBtn.setVisible(true);
                valueText.setVisible(true);
                valueList.setVisible(false);
            }
                
        }

        public class RadioButtonHandler implements ItemListener {
            public void itemStateChanged( ItemEvent e)
            {
                if (e.getSource() == cmdGetDevInfo) {
                    valueText.setEditable(false);
                    cmdStr = "GetDevInfo";
                    setAttribute();
                }
                else if (e.getSource() == cmdGet) {
                    valueText.setEditable(false);
                    cmdStr = "Get";
                    setAttribute();
                }
                else if (e.getSource() == cmdSet) {
                    valueText.setEditable(true);
                    cmdStr = "Set";
                    setAttribute();
                }
                else if (e.getSource() == devController) {
                    devStr = "Controller";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == devInterface) {
                    devStr = "Interface";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == devDisplay) {
                    devStr = "Display";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == devSensorHead) {
                    devStr = "SensorHead";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == devVeloDec) {
                    devStr = "VeloDec";
                    enableNumber12();
                    number0.setSelected(true);
                    numStr = "0";
                    setAttribute();
                }
                else if (e.getSource() == devDisplDec) {
                    devStr = "DisplDec";
                    enableNumber12();
                    number1.setSelected(true);
                    numStr = "1";
                    setAttribute();
                }
                else if (e.getSource() == devVeloFilter) {
                    devStr = "VeloFilter";
                    enableNumber12();
                    number0.setSelected(true);
                    numStr = "0";
                    setAttribute();
                }
                else if (e.getSource() == devTrackingFilter) {
                    devStr = "TrackingFilter";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == devSignalLevel) {
                    devStr = "SignalLevel";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == devOverRange) {
                    devStr = "OverRange";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == devDigitalOut) {
                    devStr = "DigitalOut";
                    disableNumber12();
                    setAttribute();
                }
                else if (e.getSource() == number0) {
                    numStr = "0";
                }
                else if (e.getSource() == number1) {
                    numStr = "1";
                }
                else if (e.getSource() == number2) {
                    numStr = "2";
                }
                else if (e.getSource() == attr0) {
                    attStr = ((AbstractButton)(e.getSource())).getText();
                    showSetting();
                }
                else if (e.getSource() == attr1) {
                    attStr = ((AbstractButton)(e.getSource())).getText();
                    showSetting();
                }
                else if (e.getSource() == attr2) {
                    attStr = ((AbstractButton)(e.getSource())).getText();
                    showSetting();
                }
                else if (e.getSource() == attr3) {
                    attStr = ((AbstractButton)(e.getSource())).getText();
                    showSetting();
                }
                else if (e.getSource() == attr4) {
                    attStr = ((AbstractButton)(e.getSource())).getText();
                    showSetting();
                }
                else if (e.getSource() == attr5) {
                    attStr = ((AbstractButton)(e.getSource())).getText();
                    showSetting();
                }
                else if (e.getSource() == attr6) {
                    attStr = ((AbstractButton)(e.getSource())).getText();
                    showSetting();
                }
                
                messageStr = cmdStr + ", " + devStr + ", " + numStr + ", " + attStr + setStr + lfChar;
                cmdText.setText(cmdStr + ", " + devStr + ", " + numStr + ", " + attStr + setStr);
                setStr = "";

            }
        }
        
        public class SerialPortComm implements Runnable, SerialPortEventListener{
            CommPortIdentifier portId;
            Enumeration portList;

            InputStream inputStream;
            OutputStream outputStream;
            Thread readThread;
            int numBytes = 0;
            
            public SerialPortComm() {
                portList = CommPortIdentifier.getPortIdentifiers(); 

                System.out.println("Start to initiliz serial port!");
                while (portList.hasMoreElements()) {
                    portId = (CommPortIdentifier) portList.nextElement();
                    String temp = portId.getName();
                    System.out.println("Find the ports!" + temp);
                    if (portId.getPortType() == CommPortIdentifier.PORT_SERIAL) {
                        if (portId.getName().equals("COM1")) {
                            try {
                                serialPort = (SerialPort)
                                    portId.open("LDVProjectApp", 2000);
                            } catch (PortInUseException e) {}

                            if (serialPort == null) {
                                JOptionPane.showMessageDialog(tabPane,"Serial port initilized failed", "Error", JOptionPane.ERROR_MESSAGE);
                                return;
                            }
                            
                            try {
                                serialPort.setSerialPortParams(115200,
                                    SerialPort.DATABITS_8,
                                    SerialPort.STOPBITS_1,
                                    SerialPort.PARITY_NONE);
                            } catch (UnsupportedCommOperationException e) {}

                            try {
                                outputStream = serialPort.getOutputStream();
                                inputStream = serialPort.getInputStream();
                            } catch (IOException e) {}

                            try {
                                serialPort.addEventListener(this);
                            } catch (TooManyListenersException e) {}
                            serialPort.notifyOnDataAvailable(true);

                            readThread = new Thread(this);
                            readThread.start();

                            System.out.println("COM1 initilized successfully!");
                            break;

                        }

                    }
                }
            }

            public void run() {
                try {
                    Thread.sleep(20000);
                        
                } catch (InterruptedException e) {}
            }

            public void serialEvent(SerialPortEvent event) {
                switch(event.getEventType()) {
                case SerialPortEvent.BI:
                case SerialPortEvent.OE:
                case SerialPortEvent.FE:
                case SerialPortEvent.PE:
                case SerialPortEvent.CD:
                case SerialPortEvent.CTS:
                case SerialPortEvent.DSR:
                case SerialPortEvent.RI:
                case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
                    break;
                case SerialPortEvent.DATA_AVAILABLE:
                    byte[] readBuffer = new byte[200];

                    try {
                        while (inputStream.available() > 0) {
                            numBytes = inputStream.read(readBuffer);
                            resultStr = resultStr + new String(readBuffer,0,numBytes);
                        }
                        System.out.println(resultStr);
                        valueText.setText(resultStr);

                    } catch (IOException e) {}
                    break;
                }
            }
            
            public void write(String messageString) {

                try {
                    outputStream.write(messageString.getBytes());
                } catch (IOException e) {}
                            
            }
            
        }
    }
    
}
