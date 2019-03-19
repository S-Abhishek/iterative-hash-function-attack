#include <iostream>
#include <array>
#include <map>
using namespace std;

unsigned char* characters(long long b){

    unsigned char* str = new unsigned char[4]();
    str[3] = (char) b & 0xFF;
    b = b >> 8;
    str[2] = (char) b & 0xFF;
    b = b >> 8;
    str[1] = (char) b & 0xFF;
    b = b >> 8;
    str[0] = (char) b & 0xFF;
    b = b >> 8;
    return str;
}



void brute_force(unsigned char *hash1, unsigned char *hash2){
    
    map<string,string> collision;
    
    for(long long i = 0 ; i < (1<<32) ; i++)
    {
        unsigned char *message = characters(i);
        
        


    }

    unsigned char *message = characters(64);
    cout<<message;

}



int main(){

    unsigned char *hash1 = (unsigned char *)"abcd";
    unsigned char *hash2 = (unsigned char *)"efgh";
    brute_force(hash1,hash2);

    return 0;
}