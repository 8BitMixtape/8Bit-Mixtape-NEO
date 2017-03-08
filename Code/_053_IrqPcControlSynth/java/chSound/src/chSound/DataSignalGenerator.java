package chSound;

/************************************************************************************

byte to audio signal converter

This class converts bytes to bits into an audio signal with differential manchester 
coding. 

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
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;


public class DataSignalGenerator 
{
	protected static final int SAMPLE_RATE = 44100;
	   
	private int 	modulationType=0;
	private int     synchronizationPreampleNumberOfBytes = 3;

	private byte    manchesterPhase     =  127;    // current phase for differential manchester coding
	private int     NumberOfSamplesPerBit = 4; // this value must be even 4 for manchester, 32 for FSK
	private int     numberOfPreampleBits  = 25; // FSK
	
	private SourceDataLine line;

	public void setModulationType(int mtype)
	{
		modulationType=mtype;
		if(modulationType==0)	NumberOfSamplesPerBit = 4; // this value must be even
		if(modulationType==1)	NumberOfSamplesPerBit = 32; // this value must be even
	}
	
	public DataSignalGenerator () throws LineUnavailableException
	{

	}

	public void open() throws LineUnavailableException
	{
		final AudioFormat af = new AudioFormat(SAMPLE_RATE, 8, 1, true, true);
		line = AudioSystem.getSourceDataLine(af);
		line.open(af, SAMPLE_RATE);
		line.start();		
	}
	
	public void close()
	{
		line.drain();
		line.close();
	}
	
	private int manchesterEdge(boolean bitValue, int pointerIntoSignal, byte signal[] )
	{
		int n;

		if(bitValue) manchesterPhase = (byte)-manchesterPhase; // toggle phase
		for(n=0;n<NumberOfSamplesPerBit;n++)
		{
			if(n==(NumberOfSamplesPerBit/2))manchesterPhase=(byte)-manchesterPhase; // toggle phase
			signal[pointerIntoSignal]=manchesterPhase;
			pointerIntoSignal++;
		}		
		
		return NumberOfSamplesPerBit;
	}
	
	private int makeBitFSK(boolean bitValue, int pointerIntoSignal, byte signal[] )
	{

		int n;

		int bitLength;
		
		if(bitValue) bitLength=NumberOfSamplesPerBit/2; // bit value 1 => high frequency
		else bitLength=NumberOfSamplesPerBit; //  bit value 0 => low frequency
		
		for(n=0;n<bitLength;n++)
		{
			byte value;
			if( n >= bitLength/2 )value=127;
			else value=-127;
			signal[pointerIntoSignal]=value;
			pointerIntoSignal++;
		}	
		return bitLength;
	
	}
	
	private int makeFSKPreample(int pointerIntoSignal, byte signal[] )
	{

		
		for(int n=0;n<numberOfPreampleBits;n++)
		{
			pointerIntoSignal+=makeBitFSK(false,pointerIntoSignal,signal); // low bit
		}
		return pointerIntoSignal;
	}

	
	public byte[] getFSKCodedSignal(int hexdata[])
	{
		int laenge=hexdata.length;
		int preampleLengthInSamples = numberOfPreampleBits * NumberOfSamplesPerBit;
		int dataLengthInSamples     = laenge*9             * NumberOfSamplesPerBit; // start bit + 8 bits = 9
		byte[] signal=new byte[ preampleLengthInSamples + dataLengthInSamples + 5000 ]; 
		// this calculation is not quite correct because zeros have only the half length 
		
		for(int n=0;n<signal.length;n++) signal[n] = 0;
		
		int counter=0;
		
		counter=makeFSKPreample(counter,signal);
		
		for(int count=0;count<hexdata.length;count++)
		{
			int dat=hexdata[count];

			/** create one byte **/		
			counter +=makeBitFSK(true,counter,signal); // start bit
			for( int n=0;n<8;n++) // first bit to send: MSB
			{
				if((dat&0x80)==0) 	counter +=makeBitFSK(false,counter,signal); // low bit
				else 				counter +=makeBitFSK(true,counter,signal);  // high bit
				
				dat      = dat << 1; // shift to next bit
			}
		}
		makeBitFSK(false, counter, signal ); // add one stop bit
		
		return signal;
	}
	
	public byte[] getDifferentialManchesterCodedSignal(int hexdata[])
	{
		int laenge=hexdata.length;
		byte[] signal=new byte[(laenge*8)*NumberOfSamplesPerBit];
		
		for(int n=0;n<signal.length;n++)signal[n]=0;
		
		int counter=0;

		/** create data signal **/
		for(int count=0;count<hexdata.length;count++)
		{
			int dat=hexdata[count];

			/** create one byte **/			
			for( int n=0;n<8;n++) // first bit to send: MSB
			{
				if((dat&0x80)==0) 	counter +=manchesterEdge(false,counter,signal); // low bit
				else 				counter +=manchesterEdge(true,counter,signal);  // high bit
				
				dat      = dat << 1; // shift to next bit
			}
		}
		return signal;	
	}
	

	void playManchesterSignal(int[] inputData) throws LineUnavailableException 
	{
		open();

		int dataLength=inputData.length;
	
		int[] data=new int[synchronizationPreampleNumberOfBytes+dataLength];

		// initialize preample with high bits ( fast pulses )
		for(int n=0;n<synchronizationPreampleNumberOfBytes;n++) data[n]=0;
		
		// set start bit
		data[synchronizationPreampleNumberOfBytes-1]=0x1;

		// add data bytes to stream
		for(int n=0;n<dataLength;n++)data[synchronizationPreampleNumberOfBytes+n]=inputData[n];
		
		// convert to manchester code
		byte[] sig=getDifferentialManchesterCodedSignal(data);
		
		System.out.println(inputData[0]+" "+inputData[1]);

		// play it
		line.write(sig, 0, sig.length);
		
		close();	
	}
	
	void playFSKSignal(int[] inputData) throws LineUnavailableException 
	{
		open();

		int dataLength=inputData.length;
	
	
		byte[] sig=getFSKCodedSignal(inputData);
		
		//System.out.println(inputData[0]+" "+inputData[1]);

		// play it
		line.write(sig, 0, sig.length);
		
		close();
		
	}
	
	void playSignal(int[] data) throws LineUnavailableException 
	{
		//NumberOfSamplesPerBit = 4;
		//playManchesterSignal(inputData);// throws LineUnavailableException
		//NumberOfSamplesPerBit = 40;
		if(modulationType==0)playManchesterSignal(data);
		if(modulationType==1)playFSKSignal(data);
	}
	
	// command length: 5 bits, data length: 10 bits
	public void writeCommand(int command, int value) throws LineUnavailableException
	{
		int[] data=new int[2];
		command&=0x1f; // 5 bits for command
		data[0]=(byte)(command<<2);
		data[0]+=(byte)((value>>8)&0x3); 
		data[1]=(byte)(value&0xFF);
		if(modulationType==0)playManchesterSignal(data);
		if(modulationType==1)playFSKSignal(data);
	}
}
