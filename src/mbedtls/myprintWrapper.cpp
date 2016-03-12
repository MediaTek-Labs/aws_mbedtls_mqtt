#include <Arduino.h>

extern "C" {
void myprint(char* str)
{
    Serial.print(str);
}
}

extern "C" {
void myprintln(char* str)
{
    Serial.println(str);
}
}