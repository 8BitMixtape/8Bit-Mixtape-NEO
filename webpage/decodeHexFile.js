function decodeHexFile(str) {
    var res= [];
    var eof= false;
    var segment= 0;
    var extended= 0;

    function readLine(line) {
        if(line[0]!==':') { //check start code
            console.log('read error: no colon at beginning of line');
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
                    console.log('warning: Start Segment Address Record - not implemented');
                    break;
                case 4: //extended linear address
                    extended= parseInt(line.substr(9, 2), 16);
                    break;
                case 5: //start linear address
                    console.log('warning: Start Linear Address Record - not implemented');
                    break;
                default:
                    console.log('read error: no matching record type');
            }
            var checksum= parseInt(line.substr(byteCount*2+9, 2), 16);
            var sum= 0;
            for(var i= 0; i<data.length; i++) {
                sum= sum+data[i];
            }
            if(256-((byteCount+(address%255)+recordType+sum)&255)!=checksum) {
                console.log('checksum error');
                document.querySelector('#error').innerHTML= 'checksum error!';
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
            readLine(line);
        }
    }
    if(!eof) {
        console.log('read error: no end of file');
    }
    return res;
}
