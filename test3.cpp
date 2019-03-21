#include <iostream>
using namespace std;
long long one = 1;
int parent(int index)
{
  return (((one<<(11)) + index)/2);
}
int main()
{
    cout<<parent(2);
    
    return 0;
}