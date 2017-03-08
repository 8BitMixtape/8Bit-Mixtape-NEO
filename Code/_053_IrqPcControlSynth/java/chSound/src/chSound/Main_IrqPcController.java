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
import javax.swing.JTextField;


public class Main_IrqPcController {
    //
   protected static final int SAMPLE_RATE = 44100;
   
	public 		JFrame frame;
	public 		JTextArea messageText;
	public 		JButton   button1;
	public 		JButton   button2;

	public 		JPanel panel;
	public 		JSlider slider1;
	public 		JSlider slider2;
	public 		JSlider slider3;
	public 		JSlider slider4;
	public 		JSlider slider5;
	public 		JSlider slider6;
	
	public JTextField textField;
	private 	boolean runLoop=true;
	public boolean autoRunFlag=false;
	
	int ledNumber=0;	
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
	
	public Main_IrqPcController() throws LineUnavailableException
	{
		frame= new JFrame(); // create main window
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setTitle("NeoPixel RGB LED controller");

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
		
		button1     = new JButton("next setup");
		button1.setBackground(Color.GREEN);
		button1.setVisible(true);

		Button1_listener b1l = new Button1_listener();
		button1.addActionListener(b1l);
		panel.add(button1);

		
		button2     = new JButton("auto run");
		button2.setBackground(Color.YELLOW);
		button2.setVisible(true);

		Button2_listener b2l = new Button2_listener();
		button2.addActionListener(b2l);
		panel.add(button2);
		
		int maxValue=1023;

		
		slider1=new JSlider(JSlider.VERTICAL,
                0, maxValue, 0);
	    slider1.setMajorTickSpacing(100);
	    slider1.setMinorTickSpacing(20);
	    slider1.setPaintTicks(true);
	    slider1.setPaintLabels(true);
		panel.add(slider1);

		slider2=new JSlider(JSlider.VERTICAL,
                0, maxValue, 0);
	    slider2.setMajorTickSpacing(100);
	    slider2.setMinorTickSpacing(20);
	    slider2.setPaintTicks(true);
	    slider2.setPaintLabels(true);
		panel.add(slider2);

		slider3=new JSlider(JSlider.VERTICAL,
                0, maxValue, 10);
	    slider3.setMajorTickSpacing(100);
	    slider3.setMinorTickSpacing(20);
	    slider3.setPaintTicks(true);
	    slider3.setPaintLabels(true);
		panel.add(slider3);
		
		slider4=new JSlider(JSlider.VERTICAL,
                0, maxValue, 10);
	    slider4.setMajorTickSpacing(100);
	    slider4.setMinorTickSpacing(20);
	    slider4.setPaintTicks(true);
	    slider4.setPaintLabels(true);
		panel.add(slider4);

		slider5=new JSlider(JSlider.VERTICAL,
                0, maxValue, 10);
	    slider5.setMajorTickSpacing(100);
	    slider5.setMinorTickSpacing(20);
	    slider5.setPaintTicks(true);
	    slider5.setPaintLabels(true);
		panel.add(slider5);
		
		textField=new JTextField("red: --- green: --- blue: ---   ");
		panel.add(textField);
		//frame.add(panel,BorderLayout.LINE_END);
		frame.add(panel);

		frame.setSize(600,300);
		frame.setVisible(true);
		
		 wg=new DataSignalGenerator ();
		 wg.setModulationType(1);//FSK
		//long oldMillis = System.currentTimeMillis() ;

		int oldSliderValue1;
		int oldSliderValue2;
		int oldSliderValue3;
		int oldSliderValue4;
		int oldSliderValue5;
		//int oldSliderValue6;
		
		oldSliderValue1=slider1.getValue();
		oldSliderValue2=slider2.getValue();
		oldSliderValue3=slider3.getValue();
		oldSliderValue4=slider4.getValue();
		oldSliderValue5=slider5.getValue();
		//oldSliderValue6=slider6.getValue();

		while (runLoop)
		{
			delay_ms(50);
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
			int value;
			
			value=slider1.getValue();
			if(value!=oldSliderValue1)
			{
				wg.writeCommand(1,value);
				oldSliderValue1=value;

			}
			
			value=slider2.getValue();			
			if(value!=oldSliderValue2)
			{
				wg.writeCommand(2,value);

				oldSliderValue2=value;
			}
			
			value=slider3.getValue();			
			if(value!=oldSliderValue3)
			{
				wg.writeCommand(3,value);

				oldSliderValue3=value;
			}
			
			value=slider4.getValue();
			if(value!=oldSliderValue4)
			{
				wg.writeCommand(4,value);
				oldSliderValue4=value;

			}
			
			value=slider5.getValue();
			if(value!=oldSliderValue5)
			{
				wg.writeCommand(5,value);
				oldSliderValue5=value;

			}
			
			//textField.setText("red:"+slider1.getValue()+" green:"+slider2.getValue()+" blue:"+slider3.getValue());
			if(autoRunFlag)
			{
				/*
				ledNumber++;
				if(ledNumber>3)ledNumber=0;
				wg.writeCommand(4,ledNumber);*/
				int v=slider1.getValue();
				v++;
				if(v>1000)v=0;
				slider1.setValue(v);
			}
		}

	}
	void toggleAutoRunFlag()
	{
		autoRunFlag=!autoRunFlag;
	}

	
	class Button1_listener implements ActionListener
	{		

		public void actionPerformed(ActionEvent e) 
		{
			

					int[] data=new int[3];
	
					data[0]=1;
					data[1]=slider2.getValue();
					data[2]=0x55;
					try {
						wg.writeCommand(16,ledNumber);
						ledNumber++;
						if(ledNumber>3)ledNumber=0;
						textField.setText("setup no."+ledNumber);
					} catch (LineUnavailableException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
					//delay_ms(200);
			 
		}
	}

	class Button2_listener implements ActionListener
	{		

		public void actionPerformed(ActionEvent e) 
		{
			toggleAutoRunFlag();
			
			
		}
	}
	public static void main(String[] args) throws LineUnavailableException 
	{
	   Main_IrqPcController mpc=new Main_IrqPcController();
    }

}