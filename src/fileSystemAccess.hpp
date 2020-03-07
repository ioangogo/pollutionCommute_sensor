#include <WiFiClient.h>

void failedmessageState();
void sendMessages(WiFiClient &net);
String lsDirHTML(String Dir);
String lsRootHTML();
String listMessages(String Dir);
void rmDirContents(String Dir);
bool initFileSystem();