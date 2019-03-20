#include <iostream>
#include <fstream>
#include <string>
#include <random>
using namespace std;

int main()
{
    ofstream msgfile("message.txt");
    

    long long NO_OF_MSGS = (long long) 1 << 10;
    long long MAX = (long long) 1 << 16;
    //long long ad = 4194304;
    
    //long long array[1024];


    random_device mno;
    mt19937 rng(mno());
    
    if(msgfile.is_open())
    {
        
        while(NO_OF_MSGS--)
        {
            uniform_int_distribution<mt19937::result_type> dist(0,MAX);
            msgfile<<dist(rng)<<endl;

        }
    }
    return 0;
}

