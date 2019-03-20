#include <iostream>
#include <omp.h>
using namespace std;

void thre(int offset, int size)
{
    #pragma omp parallel
    {
        cout<<"kasdo"<<endl;
        int x = 5;
        #pragma omp for
        for(int i = 1+offset ; i < size ; i+=2)
        {
            cout<<i<<" "<<omp_get_thread_num()<<endl;
        }
    }
}
int main()
{
    
    thre(0,6);
    thre(7,12);
    /*#pragma omp parallel
    {
        int x = 5;
        #pragma omp for
        for(int i = 1 ; i < 10 ; i+=2)
        {
            cout<<i<<" "<<omp_get_thread_num()<<endl;
        }
    }
    */
    return 0;
}
