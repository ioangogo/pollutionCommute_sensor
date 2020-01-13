#include "message.hpp"

constexpr char const appEui[] = "70B3D57ED00259BF";

// Device EUI in string format.
constexpr char const devEui[] = "26AC432A2144429E";
// Application key in string format.
constexpr char const appKey[] = "9059D1622094165B2DF48EC1FF22D8BF";

void ttnHandling(void * param);

void LoraSend(void * param);