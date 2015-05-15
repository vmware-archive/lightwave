#ifndef __VMCA_OPEN_SSL_H__
#define __VMCA_OPEN_SSL_H__

// Open SSL Based Test Code to for VMCA interop Tests

DWORD
SSLGetSubjectNameFromCertificate(std::string CertFileName, PSTR* ppSubjectName);

DWORD
SSLCreateCSR(PSTR* CSRdata);

#endif // __VMCA_OPEN_SSL_H__
