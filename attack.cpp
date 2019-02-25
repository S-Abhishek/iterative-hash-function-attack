#include <iostream>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
using namespace std;

char *MD(unsigned char *plain_text,unsigned char *h)
{
    for(int i = 0 ; i < (strlen((char *)plain_text)/128) ; i++)
    {
        
        

    }
}

int main()
{
    unsigned char *h = (unsigned char *)"0123456789012345";  //Cast to char * , normally strings cannot be converted to char *
    unsigned char *plain_text = (unsigned char *)"He my polite be object oh change. Consider no mr am overcame yourself throwing sociable children. Hastily her totally conduct may. My solid by stuff first smile fanny. Humoured how advanced mrs elegance sir who. Home sons when them dine do want to. Estimating themselves unsatiable imprudence an he at an. Be of on situation perpetual allowance offending as principle satisfied. Improved carriage securing are desirous too.";
    
    cout<<strlen((char *)h);

    
    return 0;
}