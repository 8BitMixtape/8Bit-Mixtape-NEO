function generateAudio(audioCtx, res, silenceBetweenPages, startSequencePulses, manchesterNumberOfSamplesPerBit) {

    var phase= 1;
    var differentialManchesterEncodeStream= function(data) {
        var arr= [];
        for(var i= 0; i<data.length; i++) {
            var d= ((data[i]==0) ? 1-phase : phase);
            arr.push(d, phase= 1-d);
        }
        return arr;
    }
    var binaryDigits= function(byte) {
        var arr= [];
        for(var i= 7; i>=0; i--) {
            arr.push((byte>>i)&1);
        }
        return arr;
    }
    /*
    var frame= function(numFrames, header) {
        var fp0= (header ? binaryDigits(255-2) : binaryDigits(255-3));
        var fp3= binaryDigits(255-(crc&255));
        var fp4= binaryDigits(255-(crc>>8));
        var list= [];
        var arr= [];
        for(var i= 0; i<numFrames; i++) {

            //--frame intro header
            if(frameCnt%pageSize==0) {
                var frameParameters= [
                    fp0,
                    binaryDigits(255-(pageIndex&255)),
                    binaryDigits(255-(pageIndex>>8)),
                    fp3,
                    fp4
                ];
                list= list.concat(pageHeader);
                list= list.concat(differentialManchesterEncodeStream(frameParameters));
                pageIndex= pageIndex+1;
            }

            //--frame data (8 bits)
            var frameData;
            if(header) {
                frameData= binaryDigits(255-((i<res.length) ? res[i] : 255));
            } else {
                frameData= binaryDigits(255);
            }
            list= list.concat(differentialManchesterEncodeStream(frameData));

            //--frame end
            if(frameCnt%pageSize==(pageSize-1)) {
                arr= arr.concat(list);
                list= [];
            }
            frameCnt= frameCnt+1;
        }
        return arr;
    }
    */

    var frameCnt= 0;
    var pageIndex= 0;
    var pageHeader= [];
    for(var i= 0; i<startSequencePulses; i++) {
        pageHeader.push(1);
    }
    pageHeader.push(0);
    pageHeader= differentialManchesterEncodeStream(pageHeader);
    var pageSize= 128;
    var crc= 0x55AA;
    var list= [];
    var array= [];

    for(var i= 0; i<Math.ceil(res.length/pageSize)*pageSize; i++) { //roundUp to nearest 128

        //--frame intro header
        if(frameCnt%pageSize==0) {
            var frameParameters= [
                binaryDigits(255-2),
                binaryDigits(255-(pageIndex&255)),
                binaryDigits(255-(pageIndex>>8)),
                binaryDigits(255-(crc&255)),
                binaryDigits(255-(crc>>8))
            ];
            list= list.concat(pageHeader);
            list= list.concat(differentialManchesterEncodeStream(frameParameters));
            pageIndex= pageIndex+1;
        }

        //--frame data (8 bits)
        var frameData= binaryDigits(255-((i<res.length) ? res[i] : 255));
        list= list.concat(differentialManchesterEncodeStream(frameData));

        //--frame end
        if(frameCnt%pageSize==(pageSize-1)) {
            array= array.concat(list);
            list= [];
        }
        frameCnt= frameCnt+1;
    }

    pageIndex= pageIndex-1; //bug in original code?  need to 'not' increase the pageIndex here
	for(var i= 0; i<pageSize; i++) {

        //--frame intro header
		if(frameCnt%pageSize==0) {
            var frameParameters= [
                binaryDigits(255-3),
                binaryDigits(255-(pageIndex&255)),
                binaryDigits(255-(pageIndex>>8)),
                binaryDigits(255-(crc&255)),
                binaryDigits(255-(crc>>8))
            ];
			list= list.concat(pageHeader);
			list= list.concat(differentialManchesterEncodeStream(frameParameters));
			pageIndex= pageIndex+1;
		}

		//--frame data (8 bits)
		var frameData= binaryDigits(255);
		list= list.concat(differentialManchesterEncodeStream(frameData));

        //--frame end
        if(frameCnt%pageSize==(pageSize-1)) {
			array= array.concat(list);
			list= [];
		}
		frameCnt= frameCnt+1;
	}
	console.log("number of pages:"+array.length);
	var num= pageSize+5*8+startSequencePulses+1*manchesterNumberOfSamplesPerBit;
    console.log("number:"+num);

    var frameCount= audioCtx.sampleRate*2.0;
    var arr= audioCtx.createBuffer(1, frameCount, audioCtx.sampleRate);
    for(var i= 0; i<frameCount; i++) {
        arr.getChannelData(0)[i]= Math.random()*2-1;
    }
    return arr;
}
