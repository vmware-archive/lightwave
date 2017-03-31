/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <io.h>
#endif

#include <boost/program_options.hpp>
#include <string>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmca_error.h>
#include <iostream>
#include <fstream>

#ifndef _WIN32
#include <termios.h>
#endif

#include "certool.h"

#ifdef _WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

namespace po = boost::program_options;
po::variables_map argsMap;

//
// Command Arguments
//
std::string argServerName;
std::string argSrpUpn;
std::string argSrpPwd;
std::string argCert;
std::string argConfig;
std::string argPrivateKey;
std::string argPublicKey;
std::string argCsr;
std::string argConfModule;
std::string argFilter;
std::string argHelpModule;
std::string argFQDN;
std::string argCrl;
std::string argCurrCrl;
bool argPKCS12 = false;
bool argStorePrivate;
bool argStoreTrusted;
bool argStoreRevoked;
bool argStoreAll;
std::string argOutPrivateKey;
std::string argOutCert;
std::string argOption;
int argPredates = VMCA_DEFAULT_CA_CERT_START_PREDATE;

std::string argPassword;
std::string argUserName;
std::string argDomainName;

int argWait;
int argPort;
int argErr;

//
// Config Values
//
std::string cfgName;
std::string cfgDomainComponent;
std::string cfgCountry;
std::string cfgOrganization;
std::string cfgOrgUnit;
std::string cfgState;
std::string cfgLocality;
std::string cfgIPAddress;
std::string cfgEmail;
std::string cfgHostName;


time_t now;
time_t expire;

PCSTR
ErrorCodeToName(int code)
{
    int i = 0;
    VMCA_ERROR_CODE_NAME_MAP VMCA_ERROR_Table[] =
                                 VMCA_ERROR_TABLE_INITIALIZER;

    if (code == 0) return "Success";
    for (i=0; i<sizeof(VMCA_ERROR_Table)/sizeof(VMCA_ERROR_Table[0]); i++)
    {
        if ( code == VMCA_ERROR_Table[i].code)
        {
            return VMCA_ERROR_Table[i].name;
        }
    }

    return UNKNOWN_STRING;
}

VMCA_FILE_ENCODING GetFileEncoding(std::ifstream& ifs, bool bSkipHeader)
{
    VMCA_FILE_ENCODING fEncoding = VMCA_FILE_ENCODING_UTF8;
    char bom[3] = { 0 };
    size_t headerSize = 0;

    ifs.read(bom, sizeof(bom));

    if (bom[0] == '\xff' &&
        bom[1] == '\xfe')
    {
        fEncoding = VMCA_FILE_ENCODING_UTF16LE;
        headerSize = 2;
    }
    else if (bom[0] == '\xfe' &&
             bom[1] == '\xff')
    {
        fEncoding = VMCA_FILE_ENCODING_UTF16BE;
        headerSize = 2;
    }
    else if (bom[0] == '\xef' &&
             bom[1] == '\xbb' &&
             bom[2] == '\xbf' )
    {
        fEncoding = VMCA_FILE_ENCODING_UTF8;
        headerSize = 3;
    }

    if (bSkipHeader)
    {
        ifs.seekg(headerSize);
    }
    else
    {
        ifs.seekg(0);
    }

    return fEncoding;
}

#ifdef _WIN32

std::wstring
string_to_w(const std::string& str)
{
    DWORD dwError = ERROR_SUCCESS;
    PWSTR pwszStr = NULL;
    std::wstring wstr;

    dwError = VMCAAllocateStringWFromA(str.c_str(), &pwszStr);
    BAIL_ON_ERROR(dwError);
    wstr = pwszStr;

cleanup:
    if (pwszStr)
    {
        VMCAFreeStringW((RP_PWSTR)pwszStr);
    }

    return wstr;
error:
    throw std::bad_alloc();
    goto cleanup;
}

#endif

DWORD
ParserConfigFile(po::options_description& config, const std::string& configFile)
{
    DWORD dwError = ERROR_SUCCESS;
#ifdef WIN32
    std::ifstream ifs(string_to_w(configFile).c_str(), std::ifstream::binary);
#else
    std::ifstream ifs(configFile.c_str(), std::ifstream::binary);
#endif
    if(!ifs)
    {
        std::cout << "Failed to open config file: " << configFile << "\n"
                  << "Exiting ...";
#ifdef WIN32
        dwError = GetLastError();
#else
        dwError = VMCAGetWin32ErrorCode(errno);
#endif
    }
    else
    {
        VMCA_FILE_ENCODING encoding = GetFileEncoding(ifs, true);

        if (encoding != VMCA_FILE_ENCODING_UTF8)
        {
            std::cout << "Config file: " << configFile
                      << "uses UTF16 encoding, certool supports only UTF8 or ASCII encoding for config files" << std::endl
                      << "Exiting ...";
            return VMCA_ARGUMENT_ERROR;
        }

        std::cout << "Using config file : " << configFile << std::endl;
        store(parse_config_file(ifs, config), argsMap);
        notify(argsMap);
    }
    return dwError;
}

#ifndef _WIN32

static
DWORD
VMCAReadPassword(
    PCSTR pszUser,
    PCSTR pszPrompt,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    struct termios orig, nonecho;
    CHAR  szPassword[33] = "";
    PSTR  pszPassword = NULL;
    DWORD iChar = 0;

    memset(szPassword, 0, sizeof(szPassword));

    if (IsNullOrEmptyString(pszPrompt))
    {
        fprintf(stdout, "Enter password for %s: ", pszUser);
    }
    else
    {
        fprintf(stdout, "%s:", pszPrompt);
    }
    fflush(stdout);

    tcgetattr(0, &orig); // get current settings
    memcpy(&nonecho, &orig, sizeof(struct termios)); // copy settings
    nonecho.c_lflag &= ~(ECHO); // don't echo password characters
    tcsetattr(0, TCSANOW, &nonecho); // set current settings to not echo

    // Read up to 32 characters of password

    for (; iChar < sizeof(szPassword)-1; iChar++)
    {
        ssize_t nRead = 0;
        CHAR ch;

        if ((nRead = read(STDIN_FILENO, &ch, 1)) < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMCA_ERROR(dwError);
        }

        if (nRead == 0 || ch == '\n')
        {
            fprintf(stdout, "\n");
            fflush(stdout);
            break;
        }
        else if (ch == '\b') /* backspace */
        {
            if (iChar > 0)
            {
                iChar--;
                szPassword[iChar] = '\0';
            }
        }
        else
        {
            szPassword[iChar] = ch;
        }
    }

    if (IsNullOrEmptyString(szPassword))
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateStringA(szPassword, &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszPassword = pszPassword;

cleanup:

    tcsetattr(0, TCSANOW, &orig);

    return dwError;

error:

    *ppszPassword = NULL;

    goto cleanup;
}

#else

static
DWORD
VMCAReadPassword(
    PCSTR pszUser,
    PCSTR pszPrompt,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    CHAR  szPassword[33] = "";
    PSTR  pszPassword = NULL;
    DWORD iChar = 0;
    BOOLEAN bConsole = FALSE;

    memset(szPassword, 0, sizeof(szPassword));

    if (IsNullOrEmptyString(pszPrompt))
    {
        fprintf(stdout, "Enter password for %s: ", pszUser);
    }
    else
    {
        fprintf(stdout, "%s:", pszPrompt);
    }
    fflush(stdout);

    bConsole = _isatty(0); // Is stdin console?

    // Read up to 32 characters of password

    for (; iChar < sizeof(szPassword); iChar++)
    {
        CHAR ch = bConsole ? _getch() : getchar();

        if (ch == EOF || ch == '\r' || ch == '\n')
        {
            fprintf(stdout, "\r\n");
            fflush(stdout);
            break;
        }
        else if (ch == '\b') /* backspace */
        {
            if (iChar > 0)
            {
                iChar--;
                szPassword[iChar] = '\0';
            }
        }
        else
        {
            szPassword[iChar] = ch;
        }
    }

    if (IsNullOrEmptyString(szPassword))
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateStringA(szPassword, &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszPassword = pszPassword;

cleanup:

    return dwError;

error:

    *ppszPassword = NULL;

    goto cleanup;
}

#endif

void
AddGeneralOptions(po::options_description& desc)
{
    desc.add_options()
    ("help", po::value<std::string>(&argHelpModule)->implicit_value(""),
     "help <command> for help with each command\n"
     "Commands are :\n"
     "certool --help=init - shows help for all initialization relevant functions \n"
     "certool --help=functions - shows help for all other functions \n"
     "certool --help=config - shows help for the parameters of the config file \n"
    )
    ("server",po::value<std::string>(&argServerName)->default_value("localhost"),
     "The name of the VMware Certficate Server.")
    ("srp-upn", po::value<std::string>(&argSrpUpn),
     "SRP logon UPN authentication identity.")
    ("srp-pwd", po::value<std::string>(&argSrpPwd),
     "SRP logon password.")
    ("version", "print version string")
    ("viewerror", po::value<int>(&argErr)->default_value(0),
     "Prints out the Error String for the specific error code.");


}

void
AddConfigOptions(po::options_description& config)
{
    config.add_options()
    ("Country",         po::value<std::string>(&cfgCountry),"\tdefault: Country = US ")
    ("Name",            po::value<std::string>(&cfgName),"\tdefault: Name = Acme")
    ("DomainComponent", po::value<std::string>(&cfgDomainComponent),"\tdefault: DomainComponent = acme.local")
    ("Organization",    po::value<std::string>(&cfgOrganization), "\tdefault: Organization = AcmeOrg")
    ("OrgUnit",         po::value<std::string>(&cfgOrgUnit), "\tdefault:. OrgUnit = AcmeOrg Engineering")
    ("State",           po::value<std::string>(&cfgState),"\tdefault: State = California")
    ("Locality",        po::value<std::string>(&cfgLocality),"\tdefault: Locality = Palo Alto")
    ("IPAddress",       po::value<std::string>(&cfgIPAddress),"\tdefault: IPAddress = 127.0.0.1")
    ("Email",           po::value<std::string>(&cfgEmail),"\tdefault: Email = email@acme.com")
    ("Hostname",        po::value<std::string>(&cfgHostName),"\tdefault: Hostname = server.acme.com");
}

void
AddInitOptions(po::options_description& init)
{
    init.add_options()

    ("initcsr","\tThis is deprecated. Use gencsr instead.\n"
     "Generates a Certificate Signing Request\n"
     "A PKCS10 file and a key pair is generated in this mode.\n"
     "Different flags used for this command are :\n"
     "--initcsr - required, the command flag\n"
     "--privkey - required, file name for private key\n"
     "--pubkey  - required, file name for public key\n"
     "--csrfile - required, file name for the CSR\n"
     "--config  - optional, default value \"certool.cfg\" will be used.\n\n"
     "Example : certool --initcsr --privkey=<filename> --pubkey=<filename> --csrfile=<filename>\n\n"
    )

    ("gencsr","\tGenerate Certificate Request\n"
     "Generates a Certificate Signing Request\n"
     "A PKCS10 file and a key pair is generated in this mode.\n"
     "Different flags used for this command are :\n"
     "--gencsr - required, the command flag\n"
     "--privkey - required, file name for private key\n"
     "--pubkey  - required, file name for public key\n"
     "--csrfile - required, file name for the CSR\n"
     "--config  - optional, default value \"certool.cfg\" will be used.\n\n"
     "Example : certool --gencsr --privkey=<filename> --pubkey=<filename> --csrfile=<filename>\n\n"
    )

    ("selfca","\tCreates Self Signed CA\n"
     "Provisions the VMCA Server with a Self"
     "Signed Root CA Certificate.\n"
     "This is one of the simplest way to\n"
     "provision the VMware Certificate Server.\n"
     "However it is not a recommended way to provision.\n"
     "Different flags used for this command are :\n"
     "--selfca  - required, the command flag\n"
     "--predate  - optional, set valid not before field of root cert n minutes before current time\n"
     "--config  - optional, default value \"certool.cfg\" will be used.\n"
     "--server  - optional, default value of \"localhost\" will be used.\n\n"
     "Example : certool --selfca \n\n"
    )

    ("rootca","\tProvisions VMware Certificate Server with\n"
     "user supplied certificate and private key.\n"
     "Different flags used for this command are :\n"
     "--rootca  - required, the command flag\n"
     "--cert    - required, file name for cert file\n"
     "--privkey - required, file name for private key\n"
     "--server  - optional, default value of \"localhost\" will be used.\n"
     "it is assumed that both of these files are in PEM encoded format.\n\n"
     "Example :certool --rootca --cert=root.cert --privkey=privatekey.pem\n\n"

    )

    ("getdc","\tGet Default Domain Name\n"
     "Returns the default domain name used by Lotus.\n"
     "Different flags used for this command are :\n"
     "--server - optional, if left empty will connect to localhost\n"
     "--port - optional, if left empty will use port 389 ( in future LDAP_PORT)\n\n"
     "Example: certool --getdc\n\n"
    )

    ("WaitVMDIR","\tWait until VMDir is running or specified time out happens.\n"
     "--wait - optional, Number of Minutes to wait, Defaults to 3 mins.\n"
     "--server - optional, defaults to local host.\n"
     "--port - optional, defaults to port 389.\n\n"
     "Example : certool --WaitVMdir\n\n"
    )

    ("WaitVMCA","\tWait until VMCA is running or specified time out happens.\n"
     "--wait - optional, Number of Minutes to wait, Defaults to 3 mins.\n"
     "--server - optional, defaults to local host.\n\n"
     "Example : certool --WaitVMCA --wait=5\n\n"
    )

    ("publish-roots","\t Notifies the VMCA Service to publish its CA root certificates to the Directory Service\n"
    "Arguments for this commands are : \n"
    "--server - , optional, defaults to localhost\n"
    "Note : This command requires administrative privileges"
    );

}

void
AddFunctionsOptions(po::options_description& functions)
{
    functions.add_options()

    ("genkey","\tGenerates a private and public key pair\n"
     "Different flags used for this command are :\n"
     "--genkey  - required, the command flag\n"
     "--privkey - required, file name for private key\n"
     "--pubkey  - required, file name for public key\n\n"
     "Example: certool --genkey --privkey=<filename> --pubkey=<filename>\n"
    )

    ("gencert", "\tGenerates a new certificate\n"
     "based on values in the config file.\n"
     "Different flags used for this command are :\n"
     "--gencert  - required, the command flag\n"
     "--cert    - required, file name for cert file\n"
     "--privkey - required, file name for private key\n"
     "--config  - optional, default value \"certool.cfg\" will be used.\n"
     "--server  - optional, default value of \"localhost\" will be used.\n\n"
     "Example: certool --gencert --privkey=<filename> --cert=<filename>\n\n"
    )

    ("getrootca","\tPrints the Root CA certificate\n"
     "in human readable form\n"
     "Different flags used for this command are :\n"
     "--getrootca  - required, the command flag\n"
     "--server     - optional, default value of \"localhost\" will be used.\n\n"
     "Example: certool --getrootca --server=remoteserver\n"
    )

    ("revokecert","\tRevokes a certificate\n"
     "Different flags used for this command are :\n"
     "--revokecert - required, the command flag\n"
     "--cert       - required ,file name for the cert\n"
     "--server     - optional, default value of \"localhost\" will be used.\n\n"
     "Example: certool --revokecert --cert=<filename>\n"
    )

    ("viewcert","\tPrints out all the fields in a Certificate\n"
     "in human readbale form\n"
     "Different flags used for this command are :\n"
     "--viewcert - required, the command flag\n"
     "--cert       - required ,file name for the cert\n\n"
     "Example: certool --viewcert --cert=<filename>\n"
    )

    ("enumcert","\tList all the certs in the server\n"
     "Different flags used for this command are :\n"
     "--enumcert - required, the command flag\n"
     "--filter   - required ,values are [all,revoked,active]\n\n"
     "Example: certool --enumcert --filter=revoked\n"
    )

    ("genCIScert", "\tUse 'gencert' instead. 'genCIScert' is going to be deprecated.\n"
     "'genCISCert' is equivalent to 'gencert',\n"
     "except the certificate CN is generated using the scheme\n"
     "CN=<Name>, DC=<domain>, OU=mID-<MachineID> and the certificate with its key is pushed to VECS store.\n"
     "PKCS12 file (pfx) is not generated anymore. And the cert, key are not added to VECS. \n"
     "based on values in the config file.\n"
     "Different flags used for this command are :\n"
     "--genCIScert  - required, the command flag\n"
     "--cert     - required, file name for cert file\n"
     "--privkey  - required, file name for private key\n"
     "--Name     - required, Name of the Solution User Account\n"
     "--server   - optional, default value of \"localhost\" will be used.\n\n"
     "Example: certool --genCIScert --privkey=<filename> --cert=<filename> --Name=sso\n"
    )

    ("status","\tTakes a certificate and sends it server to \n"
     "check if the certificate has been revoked.\n"
     "Different flags used for this command are :\n"
     "--status     - required, the command flag\n"
     "--cert       - required ,file name for the cert\n"
     "--server  - optional, default value of \"localhost\" will be used.\n\n"
     "Example: certool --status --cert=<filename>\n"
    )

    ("gencrl","\t Causes VMCA to re-generate CRLs\n"
    "This command should NOT be used, unless you are an\n"
    "Admin of VMCA and really wants to overwrite the current\n"
    "CRL List.\n"
    )

    ("getcrl","\t Allows users to download current CRL from VMCA\n"
    "Different flags used for this command are :\n"
    "--crl=<fileName> - CRL file Name \n\n"
    "Example: certool --getcrl --crl=<filename>\n"
    )

    ("viewcrl", "\t Prints out the CRL in human readable format\n"
     "Different flags used for this commands are : \n"
     "--crl=<fileName>\n"
    )

    ("genselfcacert", "\tGenerate a self signed CA certificate\n"
     "based on values in the config file.\n"
     "Different flags used for this command are :\n"
     "--genselfcacert  - required, the command flag\n"
     "--outcert        - required, file name for cert file\n"
     "--outprivkey     - required, file name for private key\n"
     "--config         - optional, default value \"certool.cfg\" will be used.\n\n"
     "Example: certool --genselfcert --privkey=<filename> --cert=<filename>\n"
    )

    ("gencsrfromcert", "\tGenerate a CSR from a certificate\n"
     "Different flags used for this command are :\n"
     "--gencsrfromcert - required, the command flag\n"
     "--cert           - required, file name of certificate\n"
     "--privkey        - required, file name of private key\n"
     "--csrfile        - required, file name for the csr\n"
     "Example: certool --gencsrfromcert --privkey=<filename> --cert=<filename> --csrfile=<csrfilename>\n"
    )

    ("enableserveroption", "\tEnable server option\n"
     "Different flags used for this command are :\n"
     "--option         - required, the server option to enable\n"
     "--server         - optional, default value of \"localhost\" will be used.\n\n"
     "Example: certool --enableserveroption --option=multiplesan\n"
    )

    ("disableserveroption", "\tDisable server option\n"
     "Different flags used for this command are :\n"
     "--option         - required, the server option to disable\n"
     "--server         - optional, default value of \"localhost\" will be used.\n\n"
     "Example: certool --disableserveroption --option=multiplesan\n"
    )

    ("getserveroption", "\tGet enabled server options\n"
     "--server         - optional, default value of \"localhost\" will be used.\n"
    );
}

void
AddFilesOptions(po::options_description& files)
{
    files.add_options()
    ("config", po::value<std::string>(&argConfig)->default_value("certool.cfg"),
     "configration file used in creation of certificates and CSRs. Default is certool.cfg")
    ("privkey",po::value<std::string>(&argPrivateKey),"private key file" )
    ("cert",po::value<std::string>(&argCert),"private key file" )
    ("pubkey",po::value<std::string>(&argPublicKey),"private key file" )
    ("csrfile", po::value<std::string>(&argCsr),"PKCS10 (CSR) is written to this file")
    ("port", po::value<int>(&argPort)->default_value(389), "Port for the LDAP Server")
    ("wait", po::value<int>(&argWait)->default_value(3), "Wait in Minutes")
    ("InitVMCA", po::value<int>(&argWait)->default_value(3), "Initial VMCA bootup")
    ("FQDN" , po::value<std::string>(&argFQDN), "Adds the FQDN into the Subject Alt Name" )
    ("PKCS12", po::value(&argPKCS12)->zero_tokens(), "Generates a PKCS12 File, Containing Cert and Private Key")
    ("password", po::value<std::string>(&argPassword), "Password for Lotus User")
    ("user", po::value<std::string>(&argUserName),  "user name for Lotus Domain")
    ("domain", po::value<std::string>(&argDomainName)->default_value("VSPHERE.LOCAL"),"Domain name, Default is VSPHERE.LOCAL")
    ("prv", po::value(&argStorePrivate)->zero_tokens(),"Private certificate Store")
    ("trs", po::value(&argStoreTrusted)->zero_tokens(),"Trusted certificate Store")
    ("rvk", po::value(&argStoreRevoked)->zero_tokens(),"Revoked certificate Store")
    ("all", po::value(&argStoreAll)->zero_tokens(),"All certificate Store")
    ("crl",po::value<std::string>(&argCrl),"File Name for the New CRL")
    ("currcrl",po::value<std::string>(&argCurrCrl),"Existing CRL, if you have one")
    ("outprivkey",po::value<std::string>(&argOutPrivateKey),"Privatekey File to be generated")
    ("outcert",po::value<std::string>(&argOutCert),"Certificate to be generated")
    ("predate",po::value<int>(&argPredates),"Certificate Valid from predated in minutes");
}

void
AddParamOptions(po::options_description& params)
{
    params.add_options()
    ("filter",po::value<std::string>(&argFilter),"\tPossible values for filter are\n"
     "active - all certificates which are active\n"
     "revoked - lists all revoked certificates\n"
     //"expired - all certificates which are expired\n"
     "all - lists all certificates\n"
    )
    ("option",po::value<std::string>(&argOption),"\tPossible values for option are\n"
     "multiplesan - allow multiple SAN in CSR\n"
    );
}

DWORD
SetSrpAuthenticationInfo()
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszUpn = NULL;

    if (!argSrpUpn.empty() &&
        argSrpPwd.empty())
    {
        PSTR pszTempPassword = NULL;
        dwError = VMCAReadPassword(
                        argSrpUpn.c_str(),
                        NULL,
                        &pszTempPassword
                        );
        if (dwError == ERROR_SUCCESS)
        {
            argSrpPwd = pszTempPassword;
            VMCA_SAFE_FREE_MEMORY(pszTempPassword);
        }
        else
        {
            std::cout << "Error : Invalid password\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
    }

    if (argSrpUpn.empty())
    {
        // TEMP hack untill every one started using command line options
        dwError = VMCAGetMachineAccountInfoA(
                      &pszAccount,
                      &pszPassword);
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VMCAAccountDnToUpn(pszAccount, &pszUpn);
            if (dwError != ERROR_SUCCESS)
            {
                /* Don't use registry entries if UPN conversion fails */
                VMCA_SAFE_FREE_MEMORY(pszAccount);
                VMCA_SAFE_FREE_MEMORY(pszUpn);
            }
        }

        /* Command line upn/pwd has precedence */
        if (pszUpn && argSrpUpn.length() == 0)
        {
            argSrpUpn = pszUpn;
            argSrpPwd = pszPassword;
        }
        else
        {
            // Try with no authentication
            dwError =  ERROR_SUCCESS;
        }
    }

error:
    VMCA_SAFE_FREE_MEMORY(pszAccount);
    VMCA_SAFE_FREE_MEMORY(pszUpn);
    VMCA_SAFE_FREE_MEMORY(pszPassword);
    return dwError;
}

DWORD
DispatchInitFunctions(po::variables_map argsMap,po::options_description& config)
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = SetSrpAuthenticationInfo();
    BAIL_ON_ERROR(dwError);

    if (argsMap.count("selfca")) {
        if ( ParserConfigFile(config, argConfig) != 0 )  {
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        if (argPredates > VMCA_MAX_PREDATE_PERMITED || argPredates < 0)
        {
            std::cout << "Invalid start time predate specified it should be between "
                << VMCA_MAX_PREDATE_PERMITED << " and 0 "<<std::endl;
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleCreateSelfSignedCA();
        BAIL_ON_ERROR(dwError);
    }
    if (argsMap.count("rootca")) {
        if(!argsMap.count("privkey") ||
                !argsMap.count("cert")) {
            std::cout << "To upload a Root CA Cert, we need both Cert and Private Key.\n\n"
                      << "Example: certool --rootca --cert=root.cert --privkey=privatekey.pem\n"
                      << std::endl;
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        dwError = HandleRootCACertificate();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("gencsr") || argsMap.count("initcsr")) {
        if (argsMap.count("initcsr")) {
            std::cout << "This is deprecated. Use gencsr instead.\n";
        }
        if(!argsMap.count("privkey") ||
                !argsMap.count("pubkey")  ||
                !argsMap.count("csrfile")) {
            std::cout << "To create a CSR we need a private key path.\n\n"
                      << "Example : certool --gencsr --privkey=<filename> --pubkey=<filename>"
                      << " --csrfile=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        if ( ParserConfigFile(config, argConfig) != 0 )  {
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleInitCSR();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("getdc")) {
        dwError = HandleGetDC();
        BAIL_ON_ERROR(dwError);
    }


    if(argsMap.count("WaitVMCA")) {
        dwError = HandleWaitVMCA();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("WaitVMDIR")) {
        dwError = HandleWaitVMDIR();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("publish-roots")){
        dwError = HandlePublishRoots();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("updateschema")) {
        dwError = HandleUpdateSchema();
        BAIL_ON_ERROR(dwError);
    }

error:
    return dwError;
}


DWORD
DispatchFunctions(po::variables_map argsMap,po::options_description& config)
{
    DWORD dwError = ERROR_SUCCESS;

    if (argsMap.count("version")) {
            dwError = HandleVersionRequest();
            BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("gencert")) {
        if(!argsMap.count("privkey") ||
                !argsMap.count("cert") ||
                !argsMap.count("config") )  {
            std::cout << "Error : To generate a certificate we need access to a private key."
                      << "Example : certool --gencert --privkey=<filename> --cert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        if ( ParserConfigFile(config, argConfig) != 0 )  {
            std::cout << "Unable to read the config file";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleGenCert();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("genkey")) {
        if(!argsMap.count("privkey") ||
                !argsMap.count("pubkey")) {
            std::cout << "Error : To generate a key pair we need public and private key files names to write to."
                      << "Example : certool --genkey --privkey=<filename> --pubkey=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        dwError = HandleGenKey();
        BAIL_ON_ERROR(dwError);

    }

    if(argsMap.count("revokecert")) {
        if(!argsMap.count("cert")) {
            std::cout << "Error : To revoke a cert, you need a path to the certificate"
                      << "Example : certool --revokecert --cert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        dwError = HandleRevokeCert();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("viewcert")) {
        if(!argsMap.count("cert")) {
            std::cout << "Error : To view a cert, you need a path to the certificate"
                      << "Example : certool --viewcert --cert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        dwError = HandleViewCert();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("getrootca")) {
        dwError = HandleGetRootCA();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("enumcert")) {
        if(!argsMap.count("filter")) {
            std::cout << "Error : To enum certs, you need to specify certificate status"
                      << "Example: certool --enumcert --filter=revoked\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        dwError = HandleEnumCerts();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("genCIScert")) {
        if(!argsMap.count("privkey") ||
                !argsMap.count("cert") ||
                !argsMap.count("config") )  {
            std::cout << "Error : To generate a certificate we need access to a private key."
                      << "Example : certool --genCIScert --privkey=<filename> --cert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        dwError = HandleGenCISCert();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("status")) {
        if(!argsMap.count("cert")) {
            std::cout << "Error : To check the status of a cert, you need a path to the certificate"
                      << "Example : certool --status --cert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        dwError = HandleStatusCert();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("vxls")){
        if (!argsMap.count("prv") ||
            !argsMap.count("trs") ||
            !argsMap.count("rvk") ||
            !argsMap.count("all")){
            std::cout << "No Store Argument found, Enumerating Private Store:\n";
            argStorePrivate = true;
        }
        dwError = HandleVecsEnum();
        BAIL_ON_ERROR(dwError);
    }

    if ( argsMap.count("getcrl")){
        if(!argsMap.count("crl")) {

            std::cout << "Error : to download a CRL file, you need a path to the CRL file"
                      << "Example : certool --getcrl --crl=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleGetCRL();
        BAIL_ON_ERROR(dwError);
    }

    if ( argsMap.count("gencrl")){
        dwError = HandleGenCRL();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("crlinfo")){
        if(!argsMap.count("crl")) {

            std::cout << "Error : CRL file Path is missing"
                      << "Example : certool --crlinfo --crl=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }


        dwError = HandleCRLInfo();
        BAIL_ON_ERROR(dwError);
    }

    if(argsMap.count("viewcrl")) {
         if(!argsMap.count("crl")) {

            std::cout << "Error : CRL file Path is missing"
                      << "Example : certool --viewcrl --crl=<filename> \n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandlePrintCRL();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("genselfcacert"))
    {
        if(!argsMap.count("outprivkey"))
        {
            std::cout << "Error : No private key file specified.\n"
                      << "Example : certool --genselfcacert --outprivkey=<filename> --outcert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        else if(!argsMap.count("outcert"))
        {
            std::cout << "Error : No certficate file specified.\n"
                      << "Example : certool --genselfcacert --outprivkey=<filename> --outcert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        else if(!argsMap.count("config"))
        {
            std::cout << "Error : No configuration file specified.\n"
                      << "Example : certool --genselfcacert --outprivkey=<filename> --outcert=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        if ( ParserConfigFile(config, argConfig) != 0 )  {
            std::cout << "Unable to read the config file";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleGenSelfCert();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("gencsrfromcert"))
    {
        if(!argsMap.count("privkey"))
        {
            std::cout << "Error : No private key file specified.\n"
                      << "Example : certool --gencsrfromcert --privkey=<filename> --cert=<filename> --csrfile=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        else if(!argsMap.count("csrfile"))
        {
            std::cout << "Error : No output csr file specified.\n"
                      << "Example : certool --gencsrfromcert --privkey=<filename> --cert=<filename> --csrfile=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }
        else if(!argsMap.count("cert"))
        {
            std::cout << "Error : No certficate file specified.\n"
                      << "Example : certool --gencsrfromcert --privkey=<filename> --cert=<filename> --csrfile=<filename>\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleGenCSRFromCert();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("enableserveroption"))
    {
        if(!argsMap.count("option"))
        {
            std::cout << "Error : No server option specified.\n"
                      << "Example : certool --enableserveroption --option=multiplesan\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleSetServerOption();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("disableserveroption"))
    {
        if(!argsMap.count("option"))
        {
            std::cout << "Error : No server option specified.\n"
                      << "Example : certool --disableserveroption --option=multiplesan\n";
            dwError =  VMCA_ARGUMENT_ERROR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = HandleUnsetServerOption();
        BAIL_ON_ERROR(dwError);
    }

    if (argsMap.count("getserveroption"))
    {
        dwError = HandleGetServerOption();
        BAIL_ON_ERROR(dwError);
    }

error :
    return dwError;
}

DWORD
DispatchCmd(po::variables_map argsMap, po::options_description& config)
{
    DWORD dwError = ERROR_SUCCESS;
    dwError = DispatchInitFunctions(argsMap, config);
    BAIL_ON_ERROR(dwError);
    dwError = DispatchFunctions(argsMap, config);
    if( dwError == VMCA_ENUM_END)
    {
        dwError = ERROR_SUCCESS; // this is the Standard Success
    }
    BAIL_ON_ERROR(dwError);

error :
    std::cout << "Status : " << ((dwError == 0) ? "Success" : "Failed") << std::endl;
    if(dwError != 0)
    {
        PSTR szErrorMessage = NULL;
        DWORD dwErrTemp = 0;

        std::cout << "Error Code : " << dwError << std::endl;

        dwErrTemp = VMCAGetErrorString(dwError, &szErrorMessage);
        if (dwErrTemp == ERROR_SUCCESS &&
            szErrorMessage)
        {
            std::cout << "Error Message : " << szErrorMessage << std::endl;
            VMCA_SAFE_FREE_STRINGA(szErrorMessage);
        }
        else
        {
            std::cout << "Error Message : " << ErrorCodeToName(dwError) << std::endl;
        }
    }
    return dwError;
}

DWORD
ParseArgs(int argc, char* argv[])

// This function parses arguments given to the client application
//
// argc : The number of arguments
// argv : The array pointing to the arguments
{
    try {

        po::options_description certtoolArgs("");

        po::options_description desc("certool general command options");
        po::options_description config("configuration file format ( for example see : certool.cfg )");
        po::options_description init("CA server initialization functions");
        po::options_description functions("14 managment functions");
        po::options_description files("certificate data files");
        po::options_description params("additional params");

        AddGeneralOptions(desc);
        AddConfigOptions(config);
        AddInitOptions(init);
        AddFunctionsOptions(functions);
        AddFilesOptions(files);
        AddParamOptions(params);

        certtoolArgs.
        add(desc).
        add(config).
        add(init).
        add(functions).
        add(files).
        add(params);

        po::store(po::parse_command_line(argc, argv, certtoolArgs), argsMap);
        po::notify(argsMap);

        if (argsMap.count("help") || argc == 1) {

            if (argHelpModule.compare("init") == 0) {
                std::cout << init << "\n";
                return ERROR_SUCCESS;
            }

            if (argHelpModule.compare("functions") == 0) {
                std::cout << functions << "\n";
                return ERROR_SUCCESS;
            }

            if (argHelpModule.compare("config") == 0) {
                std::cout << config << "\n";
                return ERROR_SUCCESS;
            }

            if (argHelpModule.compare("files") == 0) {
                std::cout << files << "\n";
                return ERROR_SUCCESS;
            }

            std::cout << desc << "\n";
            return ERROR_SUCCESS;
        }
        return DispatchCmd(argsMap, config);

    }
    catch (std::bad_alloc& e)
    {
        std::cerr << "error - Allocation failed -" << e.what() << std::endl;
        return VMCA_OUT_MEMORY_ERR;
    }
    catch (std::exception& e)
    {
        std::cerr << "error: " << e.what() << "\n";
        return VMCA_UNKNOW_ERROR;
    } catch(...)
    {
        std::cerr << "Exception of unknown type!\n";
        return VMCA_UNKNOW_ERROR;
    }

}

VOID
VMCAFreeCommandLineA(
    int argc,
    PSTR* pUtf8Args
    )
{
    if (pUtf8Args)
    {
        for (int i = 0; i < argc; ++i)
        {
            VMCA_SAFE_FREE_STRINGA(pUtf8Args[i]);
        }

        VMCA_SAFE_FREE_MEMORY(pUtf8Args);
    }
}

DWORD
VMCAAllocateComandLineAFromW(int argc, WCHAR** argv, PSTR** ppUtf8Args)
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR* pUtf8Args = NULL;

    dwError = VMCAAllocateMemory( argc * sizeof(PSTR), (PVOID*)&pUtf8Args);
    BAIL_ON_ERROR(dwError);

    for(int i = 0; i < argc; ++i)
    {
        PSTR utf8Arg = NULL;
        dwError = VMCAAllocateStringAFromW(argv[i], &utf8Arg);
        BAIL_ON_ERROR(dwError);

        pUtf8Args[i] = utf8Arg;
    }

    *ppUtf8Args = pUtf8Args;
    pUtf8Args = NULL;

cleanup:
    return dwError;

error:
    VMCAFreeCommandLineA(argc, pUtf8Args);
    goto cleanup;
}

#ifdef WIN32
int wmain(int argc, WCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    DWORD dwError = ERROR_SUCCESS;

#ifdef WIN32
    PSTR* utf8Args = NULL;
    dwError = VMCAAllocateComandLineAFromW(argc, argv, &utf8Args);
    BAIL_ON_ERROR(dwError);
#else
    PSTR* utf8Args = argv;
#endif
    dwError = ParseArgs(argc, utf8Args);
cleanup:
#ifdef WIN32
    VMCAFreeCommandLineA(argc, utf8Args);
#endif 
    return (int)dwError;
error:
    goto cleanup;
}
