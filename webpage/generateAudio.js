
function generateAudio(audioCtx) {

    //ONLY 2sec white noise for now - TODO

    var frameCount= audioCtx.sampleRate*2.0;
    var arr= audioCtx.createBuffer(1, frameCount, audioCtx.sampleRate);
    for(var i= 0; i<frameCount; i++) {
        arr.getChannelData(0)[i]= Math.random()*2-1;
    }
    return arr;
}
