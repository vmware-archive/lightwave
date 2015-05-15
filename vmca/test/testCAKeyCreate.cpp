#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

typedef long DWORD;
typedef void VOID;

#include <pkcs_types.h>
#include <pkcs_botan.h>
#include <pkcs_csr.h>

using namespace std;
int main(int argc, char **argv)
{
    std::string rootDir("/home/anue/vmca-final/vmca/obj/test/");
    std:string delCmd("rm -rf ");
    delCmd.append(rootDir);
    delCmd.append("*.pem");

    std::string CertName("testRootCA");
    system(delCmd.c_str());
    //VMCACreateCAKeys((char*)rootDir.c_str(),(char*)CertName.c_str(),"",1024);
}