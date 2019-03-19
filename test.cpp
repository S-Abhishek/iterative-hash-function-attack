#include <iostream>
#include <array>
#include <map>
using namespace std;
int main() {
  std::cout << "Hello World!\n";
  
  map<string,string> collision;

  collision.insert(pair<string,string>("abcd","efgh")); 


   map<string, string>::iterator itr; 
   
    for (itr = collision.begin(); itr != collision.end(); ++itr) { 
        cout << '\t' << itr->first 
             << '\t' << itr->second << '\n'; 
    } 
    cout << endl;


    int x =  3071 & 0xFF;
    cout<<x<<endl;



}