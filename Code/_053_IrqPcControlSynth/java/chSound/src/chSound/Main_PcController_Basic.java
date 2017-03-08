package chSound;

/************************************************************************************

receiver program to receive to bytes from the audio line

With this program you can remote control your microcontroller over the audio line.
The first byte controls the LEDs.
The second byte controls the beeper.

*************************************************************************************

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

********************************** list of outhors **********************************

v0.1  2.3.2017 C. -H-A-B-E-R-E-R-  initial version

It is mandatory to keep the list of authors in this code.
Please add your name if you improve/extend something
2017 ChrisMicro
*************************************************************************************
*/
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JTextArea;


public class Main_PcController_Basic {
    //
   protected static final int SAMPLE_RATE = 44100;
   
	public 		JFrame frame;
	public 		JTextArea messageText;
	public 		JButton   button_writeWav;
	public 		JPanel panel;
	public 		JSlider slider1;
	public 		JSlider slider2;
	private 	boolean runLoop=true;
	
	int counter=0;

	DataSignalGenerator wg;
	
	public static void delay_ms(int t_ms)
	{
	    try 
	    {
	        Thread.sleep(t_ms);           
	    } catch(InterruptedException ex) 
	    {
	        Thread.currentThread().interrupt();
	    }
	}
	
	public Main_PcController_Basic() throws LineUnavailableException
	{
		frame= new JFrame(); // create main window
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setTitle("AttinyAudioLinePcControl");

		frame.addWindowListener
		(
			new WindowAdapter() 
			{
				public void windowClosing(WindowEvent e) 
				{
					runLoop=false;
					wg.close();
				    try 
				    {
				    Thread.sleep(500);           
				    } catch(InterruptedException ex) 
				    { 
				        Thread.currentThread().interrupt();
				    }
					
					System.exit(0);        
				}        
			}
		);
		//frame.getContentPane().setLayout(null); // kein Layout Manager für freie Größenwahl
		
		panel        = new JPanel();
		//panel.setLayout(new BoxLayout(panel,BoxLayout.Y_AXIS));
		
		button_writeWav     = new JButton("play");
		button_writeWav.setBackground(Color.GREEN);
		button_writeWav.setVisible(true);

		writeWav_ButtonListener writeWavButtonListener = new writeWav_ButtonListener();
		button_writeWav.addActionListener(writeWavButtonListener);
		panel.add(button_writeWav);
		
		slider1=new JSlider(JSlider.VERTICAL,
                0, 255, 128);
	    slider1.setMajorTickSpacing(20);
	    slider1.setMinorTickSpacing(5);
	    slider1.setPaintTicks(true);
	    slider1.setPaintLabels(true);
		panel.add(slider1);

		slider2=new JSlider(JSlider.VERTICAL,
                0, 255, 128);
	    slider2.setMajorTickSpacing(20);
	    slider2.setMinorTickSpacing(5);
	    slider2.setPaintTicks(true);
	    slider2.setPaintLabels(true);
		panel.add(slider2);
		
		//frame.add(panel,BorderLayout.LINE_END);
		frame.add(panel);

		frame.setSize(400,300);
		frame.setVisible(true);
		
		 wg=new DataSignalGenerator ();
		//long oldMillis = System.currentTimeMillis() ;

		int oldSliderValue1=0;
		int oldSliderValue2=0;
		
		while (runLoop)
		{
			delay_ms(200);
			/*
			// delay with slider
			long millis;
			do
			{
				millis=System.currentTimeMillis();
			}while(millis < (oldMillis+100 ));
			
			oldMillis=millis ;
			*/
			//System.out.println(slider1.getValue());
			int[] data=new int[2];
			
			data[0]=slider1.getValue();
			data[1]=slider2.getValue();
			if(data[0]!=oldSliderValue1 || data[1]!=oldSliderValue2)
			{
				wg.playSignal(data);
			}
			oldSliderValue1=data[0];
			oldSliderValue2=data[1];			
		}

	}
	
	class writeWav_ButtonListener implements ActionListener
	{		
		
		public void actionPerformed(ActionEvent e) 
		{
			

					int[] data=new int[2];
	
					data[0]=1;
					data[1]=slider2.getValue();
					//data[2]=0x55;
					try {
						wg.playSignal(data);
					} catch (LineUnavailableException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					delay_ms(200);
			 
		}
	}
	
	public static void main(String[] args) throws LineUnavailableException 
	{
	   Main_PcController_Basic mpc=new Main_PcController_Basic();
    }

}