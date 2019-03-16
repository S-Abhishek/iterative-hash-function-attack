#include<iostream>
#include <map>
using namespace std;

int main()
{
    unsigned char *c = NULL;
    unsigned char *x = (unsigned char *)"abded";
    unsigned char *b = (unsigned char *)"abded";
    if((c == NULL))
    {
        c = x;
    }
    cout<<c;
    int i;
    for(i = 0 ; i < 5 ; i++)
    {
        if(b[i]!=c[i])
        {
            break;
        }
    }
    if(i==5)
    {
        cout<<"Equal";
    }

    unsigned char *as = (unsigned char *)"abcd";
    string a(reinterpret_cast<char *>(as));
    cout<<a;


    map<string,string> lala;
    string abs="as";
    string baa = "asda";
    lala.insert(abs,baa);

    cout<<lala[abs];
    
}