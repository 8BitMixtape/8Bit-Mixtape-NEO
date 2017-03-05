
function decodeHexFile(str) {
    var res= [];
    var eof= false;
    var segment= 0;
    var extended= 0;

    function flatten(ary, ret) {
        ret = ret === undefined ? [] : ret;
        for (var i = 0; i < ary.length; i++) {
            if (Array.isArray(ary[i])) {
                flatten(ary[i], ret);
            } else {
                ret.push(ary[i]);
            }
        }
        return ret;
    }

    function readLine(line, lineIndex) {
        if(line[0]!==':') { //check start code
            //console.log('read error: no colon at beginning of line');
        } else {
            var byteCount= parseInt(line.substr(1, 2), 16);
            var address= parseInt(line.substr(3, 4), 16);
            var recordType= parseInt(line.substr(7, 2), 16);
            var data= [];
            switch(recordType) {
                case 0: //data
                    for(var i= 0; i<byteCount; i++) {
                        data.push(parseInt(line.substr(i*2+9, 2), 16));
                    }
                    break;
                case 1: //end of file
                    eof= true;
                    break;
                case 2: //extended segment address
                    segment= parseInt(line.substr(9, 2), 16);
                    break;
                case 3: //start segment address
                    //console.log('warning: Start Segment Address Record - not implemented');
                    break;
                case 4: //extended linear address
                    extended= parseInt(line.substr(9, 2), 16);
                    break;
                case 5: //start linear address
                    //console.log('warning: Start Linear Address Record - not implemented');
                    break;
                default:
                    //console.log('read error: no matching record type');
            }
            var checksum= parseInt(line.substr(byteCount*2+9, 2), 16);
            var sum= 0;
            for(var i= 0; i<data.length; i++) {
                sum= sum+data[i];
            }
            if(256-((byteCount+(address%255)+recordType+sum)&255)!=checksum) {
                //console.log('checksum error in line '+lineIndex);
                //document.querySelector('#error').innerHTML= 'checksum error in line'+lineIndex;
            }
            if(recordType!=1) {
                var addy= segment*16+address;
                addy= extended*65536+addy;
                res.push(data)
            }
        }
    }

    var lines= str.split('\n');
    for(var i= 0; i<lines.length; i++) {
        var line= lines[i];
        if(line.length>0) {
            readLine(line, i);
        }
    }
    if(!eof) {
        console.log('read error: no end of file');
    }
    return flatten(res);
}


var HexToSignal = function (fullSpeedFlag) {

	this.startSequencePulses = 40;
	this.numStartBits        =  1;
	this.numStopBits         =  1;
	this.manchesterPhase     =  1;    // double / current phase for differential manchester coding

	this.invertSignal        =  true; // correction of an inverted audio signal line
	
	this.lowNumberOfPulses   =  2; // not for manchester coding, only for flankensignal
	this.highNumberOfPulses  =  3; // not for manchester coding, only for flankensignal
	
	this.manchesterNumberOfSamplesPerBit = 4; // this value must be even
	this.useDifferentialManchsterCode = true;

	this.setSignalSpeed(fullSpeedFlag);
	
}

HexToSignal.prototype.setSignalSpeed = function (fullSpeedFlag) {
		if( fullSpeedFlag ) this.manchesterNumberOfSamplesPerBit = 4; // full speed
		else                this.manchesterNumberOfSamplesPerBit = 8; // half speed
};

HexToSignal.prototype.manchesterEdge = function (flag, pointerIntoSignal, signal) {


		var sigpart = [];
		var n = 0;
		var value = 0;

		if( !this.useDifferentialManchsterCode ) // non differential manchester code
		{
			if(flag) value=1;
			else value=-1;
			if(this.invertSignal)value=value*-1;  // correction of an inverted audio signal line
			for(n=0;n<this.manchesterNumberOfSamplesPerBit;n++)
			{
				if(n<this.manchesterNumberOfSamplesPerBit/2)signal[pointerIntoSignal]=-value;
				else signal[pointerIntoSignal]=value;
				pointerIntoSignal++;
			}
		}
		else // differential manchester code ( inverted )
		{
			if(flag) this.manchesterPhase=-this.manchesterPhase; // toggle phase
			for(n=0;n<this.manchesterNumberOfSamplesPerBit;n++)
			{
				if(n==(this.manchesterNumberOfSamplesPerBit/2))this.manchesterPhase=-this.manchesterPhase; // toggle phase
				signal[pointerIntoSignal]=this.manchesterPhase;
				pointerIntoSignal++;
			}		
		}
		return sigpart;

};

HexToSignal.prototype.manchesterCoding = function (hexdata) {
	var laenge = hexdata.length;
	var signal= new Array((1+this.startSequencePulses+laenge*8)*this.manchesterNumberOfSamplesPerBit);
	var counter = 0;

	for (var n = 0; n < this.startSequencePulses; n++) {
			this.manchesterEdge(false,counter,signal); // 0 bits: generate falling edges 
			counter+=this.manchesterNumberOfSamplesPerBit;
	}

	this.manchesterEdge(true,counter,signal); //  1 bit:  rising edge 
	counter+=this.manchesterNumberOfSamplesPerBit;
		
	/** create data signal **/
	var count=0;
	for(count=0;count<hexdata.length;count++)
	{
		var dat=hexdata[count];
		//System.out.println(dat);
		/** create one byte **/			
		for( var n=0;n<8;n++) // first bit to send: MSB
		{
			if((dat&0x80)==0) 	this.manchesterEdge(false,counter,signal); // generate falling edges ( 0 bits )
			else 				this.manchesterEdge(true,counter,signal); // rising edge ( 1 bit )
			counter+=this.manchesterNumberOfSamplesPerBit;	
			dat=dat<<1; // shift to next bit
		}
	}
	return signal;	


}


HexToSignal.prototype.getDifferentialManchesterCodedSignal = function (hexdata) {
		var laenge = hexdata.length;
		var signal = new Array((laenge*8)*this.manchesterNumberOfSamplesPerBit);
		var counter = 0;

		for (var count = 0; count < hexdata.length; count++) 
		{
			var dat = dat=hexdata[count];
				for (var n = 0; n < 8; n++) 
				{
					if((dat&0x80)==0)
						this.manchesterEdge(false,counter,signal); // low bit
					else
						counter += this.manchesterNumberOfSamplesPerBit;	

					dat = dat << 1; // shift to next bit
				}
		}

		return signal;
}

HexToSignal.prototype.flankensignal = function (hexdata) {

	var intro=this.startSequencePulses*this.lowNumberOfPulses+this.numStartBits*this.highNumberOfPulses+this.numStopBits*this.lowNumberOfPulses;
	var laenge=hexdata.length;
	var signal=new Array(intro+laenge*8*this.highNumberOfPulses);
	
	var sigState=-1;
	var counter=0;

		/** generate start sequence **/
		var numOfPulses=this.lowNumberOfPulses;
		for (var n=0; n<this.startSequencePulses; n++)
		{
			for(var k=0;k<numOfPulses;k++)
			{
				signal[counter++]=sigState;
			}
			sigState*=-1;
		}

		/** start: create 2 high-Bits **/
		numOfPulses=this.highNumberOfPulses;
		for (var n=0; n<this.numStartBits; n++)
		{
			for(var k=0;k<this.numOfPulses;k++)
			{
				signal[counter++]=sigState;
			}
			sigState*=-1;
		}

		/** create data signal **/
		var count=0;
		for(count=0;count<hexdata.length;count++)
		{
			var dat=hexdata[count];
			/** create one byte **/			
			for( var n=0;n<8;n++)
			{
				if((dat&0x80)==0)numOfPulses=this.lowNumberOfPulses;
				else numOfPulses=this.highNumberOfPulses;
				dat=dat<<1; // shift to next bit				
				for(var k=0;k<numOfPulses;k++)
				{
					signal[counter++]=sigState;
				}
				sigState*=-1;
			}
		}

		/** stop: create 1 low-Bit **/
		numOfPulses=this.lowNumberOfPulses;
		for (var n=0; n<this.numStopBits; n++)
		{
			for(var k=0;k<numOfPulses;k++)
			{
				signal[counter++]=sigState;
			}
			sigState*=-1;
		}

		/** cut to long signal */
		var sig2=new Array(counter);
		for(var n=0;n<sig2.length;n++) sig2[n]=signal[n];
		return sig2;

};


var BootFrame = function () {

this.command=0;
this.pageIndex=4;
this.totalLength=0;
this.crc=0x55AA;
this.pageStart = 7;
this.pageSize  = 64;
this.frameSize = this.pageStart + this.pageSize;
this.silenceBetweenPages=0.02;

};

BootFrame.prototype.setProgCommand = function () {
	this.command=2;
};


BootFrame.prototype.setRunCommand = function () {
	this.command=3;
};


BootFrame.prototype.setTestCommand = function () {
	this.command=1;
};


BootFrame.prototype.addFrameParameters = function (data) {
		data[0]=this.command;
		data[1]=this.pageIndex&0xFF;
		data[2]=(this.pageIndex>>8)&0xFF;

		data[3]=this.totalLength&0xFF;
		data[4]=(this.totalLength>>8)&0xFF;

		data[5]=this.crc&0xFF;
		data[6]=(this.crc>>8)&0xFF;		
		return data;
};


BootFrame.prototype.setFrameSize = function (frameSize) {
		this.frameSize = frameSize;
};

BootFrame.prototype.getFrameSize = function () {
		return this.frameSize;
};

BootFrame.prototype.setCommand = function (command) {
		this.command = command;
};

BootFrame.prototype.getCommand = function () {
		return this.command;
};

BootFrame.prototype.setPageIndex = function (pageIndex) {
	this.pageIndex = pageIndex;
};

BootFrame.prototype.getPageIndex = function () {
	return this.pageIndex;
};

BootFrame.prototype.setTotalLength = function (totalLength) {
	this.totalLength = totalLength;
};

BootFrame.prototype.getTotalLength = function () {
	return this.totalLength;
};

BootFrame.prototype.setCrc = function (crc) {
	this.crc = crc;
};

BootFrame.prototype.getCrc = function () {
	return this.crc;
};

BootFrame.prototype.setPageStart = function (pageStart) {
	this.pageStart = pageStart;
};

BootFrame.prototype.getPageStart = function () {
	return this.pageStart;
};

BootFrame.prototype.setPageSize = function (pageSize) {
	this.pageSize = pageSize;
};

BootFrame.prototype.getPageSize = function () {
	return this.pageSize;
};

BootFrame.prototype.setSilenceBetweenPages = function (silenceBetweenPages) {
	this.silenceBetweenPages = silenceBetweenPages;
};

BootFrame.prototype.getSilenceBetweenPages = function () {
	return this.silenceBetweenPages;
};



var WavCodeGenerator = function () {
	this.sampleRate = 44100;		// Samples per second
	this.fullSpeedFlag=true;

	this.frameSetup = new BootFrame();
}

WavCodeGenerator.prototype.appendSignal = function (sig1,  sig2) {
	var l1=sig1.length;
	var l2=sig2.length;
	var d=new Array(l1+l2);
	for(var n=0;n<l1;n++) d[n]=sig1[n];
	for(var n=0;n<l2;n++) d[n+l1]=sig2[n];		
	return d;
};

WavCodeGenerator.prototype.setSignalSpeed = function (fullSpeedFlag) {
	this.fullSpeedFlag = fullSpeedFlag;
};

WavCodeGenerator.prototype.generatePageSignal = function (data) {
		var h2s=new HexToSignal(this.fullSpeedFlag);
		var frameData=new Array(this.frameSetup.getFrameSize());

		// copy data into frame data
		for(var n=0;n<this.frameSetup.getPageSize();n++)
		{
			if ( n < data.length ) frameData[n+this.frameSetup.getPageStart()]=data[n];
			else frameData[n+this.frameSetup.getPageStart()]=0xFF;
		}
		
		this.frameSetup.addFrameParameters(frameData);

		var signal=h2s.manchesterCoding(frameData);

		return signal;
};

WavCodeGenerator.prototype.silence = function (duration) {
		var signal=new Array(duration * this.sampleRate);
		for (var i = 0; i < signal.length; i++) {
			signal[i] = 0;
		}
		return signal;
};

WavCodeGenerator.prototype.makeRunCommand = function () {
		var h2s=new HexToSignal(this.fullSpeedFlag);
		var frameData=new Array(this.frameSetup.getFrameSize());
		this.frameSetup.setRunCommand();
		this.frameSetup.addFrameParameters(frameData);
		var signal=h2s.manchesterCoding(frameData);
		return signal;
};

WavCodeGenerator.prototype.makeTestCommand = function () {
		var h2s=new HexToSignal(this.fullSpeedFlag);
		var frameData=new Array(this.frameSetup.getFrameSize());
		this.frameSetup.setTestCommand();
		this.frameSetup.addFrameParameters(frameData);
		var signal=h2s.manchesterCoding(frameData);
		return signal;
};

WavCodeGenerator.prototype.generateSignal = function (data) {
		var signal= [];
		this.frameSetup.setProgCommand(); // we want to programm the mc
		var pl=this.frameSetup.getPageSize();
		var total=data.length;
		var sigPointer=0;
		var pagePointer=0;
		
		while(total>0)
		{
			this.frameSetup.setPageIndex(pagePointer++);
			this.frameSetup.setTotalLength(data.length);

			var partSig=new Array(pl);
			
			for(var n=0;n<pl;n++)
			{
				if(n+sigPointer>data.length-1) partSig[n]=0xFF;
				else partSig[n]=data[n+sigPointer];
			}

			sigPointer+=pl;

			var sig=this.generatePageSignal(partSig);
						
			signal=this.appendSignal(signal,sig);
			signal=this.appendSignal(signal,this.silence(this.frameSetup.getSilenceBetweenPages()));
			
			total-=pl;
		}
		
		signal=this.appendSignal(signal,this.makeRunCommand()); // send mc "start the application"
		// added silence at sound end to time out sound fading in some wav players like from Mircosoft
		
		for(var k=0;k<10;k++)
		{
			signal=this.appendSignal(signal,this.silence(this.frameSetup.getSilenceBetweenPages()));
		}
				
		return signal;
};


WavCodeGenerator.prototype.playSignal = function (audioCtx, signal) {
        var frameCount	= signal.length;       
        var audio_buffer = audioCtx.createBuffer(1, frameCount, audioCtx.sampleRate);

        for(var i= 0; i<frameCount; i++) {
            audio_buffer.getChannelData(0)[i]= signal[i];
        }
        
        var src= audioCtx.createBufferSource();

        src.buffer= audio_buffer;
        src.connect(audioCtx.destination);
        src.start();
};