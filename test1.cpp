#include <iostream>
#include <array>
#include <map>
using namespace std;
unsigned char* characters(long long b){

    unsigned char* str = new unsigned char[4]();
    str[0] = (char) b & 0xFF;
    b = b >> 8;
    str[1] = (char) b & 0xFF;
    b = b >> 8;
    str[2] = (char) b & 0xFF;
    b = b >> 8;
    str[3] = (char) b & 0xFF;
    b = b >> 8;
    return str;
}

int main() {
 
  unsigned char *x = characters(65);

  cout<<x;


}