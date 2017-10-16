/*
 * VoiceExtraction.java
 *
 * Created on July 12, 2004, 5:39 PM
 */

package LDVProject;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.nio.*;
import java.net.*;
import javax.media.*;

/**
 *
 * @author  weihong
 */

public class VoiceExtraction {
    private LDVProject parent;
    private String currentDirectory;
    private DataInputStream input = null;
    private PrintStream output = null;
    
    /** Creates a new instance of VoiceExtraction */
    public VoiceExtraction(JFileChooser fChooser, LDVProject ldv) {
        try{
            input = new DataInputStream(new FileInputStream(fChooser.getSelectedFile()));
            currentDirectory = fChooser.getCurrentDirectory().getPath();
        } 
        catch (Exception e) {
            System.err.println("File operation problem:" + e.getMessage());    
        }
    }
    
    public void IntergerToByte(int orig, byte [] dest) {
        dest[3] = (byte) (orig >>> 24);
        dest[2] = (byte) (orig >>> 16);
        dest[1] = (byte) (orig >>> 8);
        dest[0] = (byte) (orig);
    }
    
    public void ShortToByte(short orig, byte [] dest) {
        dest[1] = (byte) (orig >>> 8);
        dest[0] = (byte) (orig);
    }

    public void SaveVoiceFile() {
        boolean oneByte = false;
	char ChunkID[] = {'R','I','F','F'};
	char Format[] = {'W','A','V','E','f','m','t',' '};
	char SubChunk2ID[] = {'d','a','t','a'};
        byte orderShort[] = new byte [2];
        byte orderInt[] = new byte [4];
        byte orderDouble[] = new byte [8];
	int None_INT = 0;
	short None_SHORT = 0;
	double sampleFreq;
        byte byteValue;
        short new_value;
	byte newValue[] = new byte [2];
        
        try{
            output = new PrintStream(new FileOutputStream(currentDirectory + "\\1.wav"));
            //PrintStream outputSample = new PrintStream(new FileOutputStream(currentDirectory + "\\1.txt"));
            int fileLen = input.available() / 8;
            
            None_INT = oneByte ? fileLen : fileLen * 2;
            
            output.print(ChunkID);
            IntergerToByte(None_INT, orderInt);
            output.write(orderInt, 0 ,4);
            output.print(Format);
            None_INT = 16; //Subchunk1Size    16 for PCM.
            IntergerToByte(None_INT, orderInt);
            output.write(orderInt, 0 ,4);
            None_SHORT = 1;
            ShortToByte(None_SHORT, orderShort);
            output.write(orderShort, 0 ,2);//AudioFormat  PCM = 1 (i.e. Linear quantization)
            output.write(orderShort, 0 ,2);//NumChannels   Mono = 1, Stereo = 2, etc
            None_INT = 25000;
            IntergerToByte(None_INT, orderInt);
            output.write(orderInt, 0 ,4); //4 bytes   SampleRate   8000, 44100, etc.
            None_INT = 25000 * 1 * 2;
            IntergerToByte(None_INT, orderInt);
            output.write(orderInt, 0 ,4); //4 bytes ByteRate == SampleRate * NumChannels * BitsPerSample/8
            None_SHORT = 2;
            ShortToByte(None_SHORT, orderShort);
            output.write(orderShort, 0 ,2); //BlockAlign == NumChannels * BitsPerSample/8
            None_SHORT = 16;
            ShortToByte(None_SHORT, orderShort);
            output.write(orderShort, 0 ,2); //BitsPerSample    8 bits = 8, 16 bits = 16, etc.
            output.print(SubChunk2ID);
            None_INT = oneByte ? (fileLen - 36) : (fileLen - 36) * 2;
            IntergerToByte(None_INT, orderInt);
            output.write(orderInt, 0 ,4);

            //int ii =  ByteBuffer.wrap(orderInt).order(ByteOrder.LITTLE_ENDIAN).getInt();
            input.skipBytes(256); // seek to the data area;
            for (int i = 0; i < fileLen - 32; i++)
            {
                input.readFully(orderDouble); 
                sampleFreq = ByteBuffer.wrap(orderDouble).order(ByteOrder.LITTLE_ENDIAN).getDouble();
                sampleFreq = 10000 * sampleFreq;
                //outputSample.println(sampleFreq);
                if (oneByte) {
                    byteValue = (byte) sampleFreq;
                    output.write(byteValue);
                }
                else {
                    new_value = (short)sampleFreq;
                    ShortToByte(new_value, newValue);
                    output.write(newValue, 0, 2);
                }
            }
            
            input.close();
            output.close();
            //outputSample.close();

        }
        catch (Exception e) {
            System.err.println("File transfer problem:" + e.getMessage());
        }
    }
    
}
