/* 
// I dont trust myself!
//
// lets keep all of the code that probably with have issues with memory in
// the same place so all the fixing is in the same place
*/
#include "SPIFFS.h"
#include <CayenneLPP.h>

bool initFileSystem(){
    if(!SPIFFS.begin(true)){
        return false;
    }else{
        return true;
    }
}

bool writeMessage(CayenneLPP message){
    int remainingFsBytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    int msgSize = message.getSize() + 256;
    if (msgSize > remainingFsBytes){
        return false;
    }else{
        File messageFile = SPIFFS.open("/msgs/{time}.msg", FILE_WRITE);
        messageFile.write(message.getBuffer(), message.getSize());
        messageFile.close();
    }
}