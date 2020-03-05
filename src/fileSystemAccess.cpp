/* 
// I dont trust myself!
//
// lets keep all of the code that probably with have issues with memory in
// the same place so all the fixing is in the same place
*/
#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "SPIFFS.h"
#include "message.hpp"
#include "globals.hpp"
#include <Preferences.h>


bool initFileSystem(){
    if(!SPIFFS.begin(true)){
        return false;
    }else{
        return true;
    }
}

HTTPClient http;

SensorFile loadFile(File file){
    SensorFile out;
    file.read(out.fileBytes, FILE_SIZE);
    return out;
}

void rmFile(File file){
    
    SPIFFS.remove(file.name());
}


void sendMessages(WiFiClient &net){
    File root = SPIFFS.open("/msgs/");

    File file = root.openNextFile();
    while(file){
        http.begin(net, "http://finalyear.loosleyweb.co.uk/ingest/sensorIn");
        SensorFile record = loadFile(file);
        Sensorpacket recordPkt;
        memcpy(recordPkt.packetBytes, record.fileContent.packetBytes, PACKET_SIZE);
        http.addHeader("Content-Type", "application/json");
        http.POST(PacketToJson(recordPkt, record.fileContent.time));
        file = root.openNextFile();
        http.end();
    }
   
    
}

void rmDirContents(String Dir){

    char filePathChar[FILENAME_MAX];
    Dir.toCharArray(filePathChar, FILENAME_MAX);
    File root = SPIFFS.open(filePathChar);
    File file = root.openNextFile();
    while(file){
        rmFile(file);
        file = root.openNextFile();
    }
    preferences.putInt("fileCount", 0);
}

String listMessages(String Dir){
    String ls = "";
    char filePathChar[FILENAME_MAX];
    Dir.toCharArray(filePathChar, FILENAME_MAX);
    File root = SPIFFS.open(filePathChar);

    File file = root.openNextFile();
    ls.concat("<ul>");
    while(file){
        ls.concat("<li>");
        ls.concat(file.name());
        SensorFile record = loadFile(file);
        Sensorpacket recordPkt;
        memcpy(recordPkt.packetBytes, record.fileContent.packetBytes, PACKET_SIZE);
        ls.concat("\n");
        ls.concat("<ul>");
        ls.concat("<li> time: "); ls.concat(record.fileContent.time);
        ls.concat("<li> PM2.5: "); ls.concat(recordPkt.sensorContent.pm25/10.0);
        ls.concat("<li> lat: "); ls.concat(recordPkt.sensorContent.lat/10000.0000);
        ls.concat("<li> lng: "); ls.concat(recordPkt.sensorContent.lng/10000.0000);
        ls.concat("<li> nonce: "); ls.concat(recordPkt.sensorContent.nonce);
        ls.concat("<li> raw: ");
        for(int i = 0; i < PACKET_SIZE; i++){
            String hex = String(recordPkt.packetBytes[i], HEX);
            ls.concat(hex);
        }
        ls.concat("</ul>");
    
    
        file = root.openNextFile();
    }
    ls.concat("</ul>");
    return ls;
}

String lsDirHTML(String Dir){
    String ls = "";
    char filePathChar[FILENAME_MAX];
    Dir.toCharArray(filePathChar, FILENAME_MAX);
    File root = SPIFFS.open(filePathChar);

    File file = root.openNextFile();
    ls.concat("<ul>");
    while(file){
        ls.concat("<li>");
        ls.concat(file.name());
        ls.concat("\n");
    
        file = root.openNextFile();
    }
    ls.concat("</ul>");
    return ls;
}

String lsRootHTML(){
    String ls = "";
    File root = SPIFFS.open("/");

    File file = root.openNextFile();
    ls.concat("<ul>");
    while(file){
        ls.concat("<li>");
        ls.concat(file.name());
        ls.concat("\n");
    
        file = root.openNextFile();
    }
    ls.concat("</ul>");
    return ls;
}

String lsDir(String Dir){
    char filePathChar[FILENAME_MAX];
    Dir.toCharArray(filePathChar, FILENAME_MAX);
    String ls = "";
    File root = SPIFFS.open(filePathChar);
 
    File file = root.openNextFile();
    
    while(file){
    
        ls.concat(file.name());
        ls.concat("\n");
    
        file = root.openNextFile();
    }
    return ls;
}

bool writeMessage(Sensorpacket message){
    int remainingFsBytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    int msgSize = PACKET_SIZE + 256;
    if (msgSize > remainingFsBytes){
        return false;
    }else{
        String filename="/msgs/";
        char filenameChar[FILENAME_MAX];
        int filenum = preferences.getInt("fileCount", 0);
        filename.concat(filenum);
        filenum++;
        preferences.putInt("fileCount", filenum);
        filename.concat(".msg");
        Serial.println(filename);
        filename.toCharArray(filenameChar, FILENAME_MAX);
        File messageFile = SPIFFS.open(filenameChar, FILE_WRITE);
        
        SensorFile file;

        memcpy(file.fileContent.packetBytes, message.packetBytes, PACKET_SIZE);
        file.fileContent.time = gpslocTimeUnix;
        Serial.println(gpslocTimeUnix);
        Serial.println(file.fileContent.time);

        messageFile.write(file.fileBytes, FILE_SIZE);
        messageFile.close();
        return true;
    }
}

void failedmessageState(){
    Serial.printf("Send Failed, saving packet of size %d", PACKET_SIZE);
    writeMessage(LoraPacket);
    state=SLEEP;
}
