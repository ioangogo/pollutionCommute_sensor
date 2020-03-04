/* 
// I dont trust myself!
//
// lets keep all of the code that probably with have issues with memory in
// the same place so all the fixing is in the same place
*/
#include "SPIFFS.h"
#include "message.hpp"
#include "globals.hpp"

bool initFileSystem(){
    if(!SPIFFS.begin(true)){
        return false;
    }else{
        return true;
    }
}

SensorFile loadFile (File file){
    SensorFile out;
    file.read(out.fileBytes, FILE_SIZE);
    return out;
}

bool writeMessage(Sensorpacket message){
    int remainingFsBytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    int msgSize = PACKET_SIZE + 256;
    if (msgSize > remainingFsBytes){
        return false;
    }else{
        String filename="/msgs/";
        String time = String(gpslocTimeUnix, 10);
        filename.concat(time);
        filename.concat(".msg");
        File messageFile = SPIFFS.open("/msgs/{time}.msg", FILE_WRITE);
        
        SensorFile file;

        memcpy(file.fileContent.packetBytes, message.packetBytes, PACKET_SIZE);
        file.fileContent.time = gpslocTimeUnix;

        messageFile.write(file.fileBytes, FILE_SIZE);
        messageFile.close();
        return true;
    }
}

void failedmessageState(){
    writeMessage(LoraPacket);
    state=SLEEP;
}
