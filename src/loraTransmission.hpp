#include "message.hpp"

constexpr char const appEui[] = "70B3D57ED00259BF";

void ttnHandling(void * param);

void LoraSend(void * param);

void loraInit();

void loraLoop();