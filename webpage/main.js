window.addEventListener('load', init, false);
function init() {

    var hexdata = [14,192,29,192,28,192,27,192,26,192,49,192,24,192,23,192,22,192,21,192,20,192,19,192,18,192,17,192,16,192,17,36,31,190,207,229,210,224,222,191,205,191,32,224,160,230,176,224,1,192,29,146,169,54,178,7,225,247,4,208,119,192,224,207,8,149,8,149,129,183,129,191,92,208,250,223,250,223,254,207,128,183,128,127,128,191,128,183,128,104,128,191,140,181,128,100,140,189,143,239,141,189,128,183,135,96,128,191,8,149,31,146,15,146,15,182,15,146,17,36,47,147,63,147,143,147,159,147,175,147,191,147,128,145,97,0,144,145,98,0,160,145,99,0,176,145,100,0,48,145,96,0,38,224,35,15,45,55,48,240,41,232,35,15,3,150,161,29,177,29,3,192,2,150,161,29,177,29,32,147,96,0,128,147,97,0,144,147,98,0,160,147,99,0,176,147,100,0,128,145,101,0,144,145,102,0,160,145,103,0,176,145,104,0,1,150,161,29,177,29,128,147,101,0,144,147,102,0,160,147,103,0,176,147,104,0,191,145,175,145,159,145,143,145,63,145,47,145,15,144,15,190,15,144,31,144,24,149,138,181,130,96,138,189,138,181,129,96,138,189,131,183,136,127,131,96,131,191,120,148,137,183,130,96,137,191,152,223,134,177,136,119,134,104,134,185,55,154,8,149,248,148,255,207];
    
    var hex_string = "";

    //--webaudio
    var audioCtx;
    try {
        window.AudioContext= window.AudioContext||window.webkitAudioContext;
        audioCtx= new AudioContext();
    }
    catch(e) {
        alert('Web Audio API is not supported in this browser');
    }

    //--hex data input
    document.querySelector('#hexfileInput').onchange= function(e) {
        var file= e.target.files[0];
        if(file) {
            var reader= new FileReader();
            reader.onload= function(ee) {
                var hexData= ee.target.result;
                hex_string= hexData;
                //console.log('name: '+file.name+'\n'+'type: '+file.type+'\n'+'size: '+file.size+'\n'+'contents: '+hexData);
                if(hexData[0]===':') { //TODO also check min/max filesize?
                    document.querySelector('#hexRaw').value= hexData;
                    var event= new Event('change');
                    document.querySelector('#hexRaw').dispatchEvent(event);
                } else {
                    alert('Failed to load hex file - not a proper hex file');
                }
            }
            reader.readAsText(file);
        } else {
            alert('Failed to load hex file');
        }
    }

    //--hex decoded data
    document.querySelector('#hexRaw').onchange= function(e) {
        document.querySelector('#hexDecoded').value= hex_string;
    }
    
    var wg = new WavCodeGenerator();

    document.querySelector('#playButton').onclick= function() {

        var hex_array = decodeHexFile(hex_string);
        var signal = wg.generateSignal(hex_array);

        var frameCount= signal.length;
        
        var audio_buffer = audioCtx.createBuffer(1, frameCount, audioCtx.sampleRate);

        for(var i= 0; i<frameCount; i++) {
            audio_buffer.getChannelData(0)[i]= signal[i];
        }
        
        var src= audioCtx.createBufferSource();

        src.buffer= audio_buffer;
        src.connect(audioCtx.destination);
        src.start();
    
    }
}
