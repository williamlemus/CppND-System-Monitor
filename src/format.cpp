#include <string>

#include "format.h"

using std::string;

string zeroPad(int s){
  return s < 10 ? "0" + std::to_string(s) : std::to_string(s);
}

string Format::ElapsedTime(long seconds) {
    int hours, minutes, remainingSeconds;
    remainingSeconds =  seconds % 60;
    minutes = (seconds / 60) % 60;
    hours = seconds / 3600;
    return string(zeroPad(hours) + ":" + zeroPad(minutes) + ":" + zeroPad(remainingSeconds));
}