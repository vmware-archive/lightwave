#
# Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the “License”); you may not
# use this file except in compliance with the License.  You may obtain a copy
# of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an “AS IS” BASIS, without
# warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
import sys
sys.path.append("/opt/vmware/lib64")
import vmca
import time

# for i in dir(vmca.request):print i
# for i in dir(vmca.client): print i

client  = vmca.client("localhost")
print client.GetVersion()

req = client.GetRequest()

req.country = "US"
req.name = "PyTest"
req.organization = "VMware"
req.orgunit = "VMware Engineering"
req.state = "california"
req.locality = "Palo Alto"
req.ipaddress = "127.0.0.1"
req.email = "anu.engineer@gmail.com"
req.uri = "host.machine.com"

keys = req.CreateKeyPair(2048)
print keys.privatekey

# print req.GetCSR()
beginTime = int(time.time())
endTime = int(beginTime + ( 365 * 24 * 60 * 60))

## get a certificate object signed by VMCA
certificate = client.GetCertificate(req, keys, beginTime, endTime)

## get a self signed Certificate
selfsigned = client.GetSelfSignedCertificate(req, keys, beginTime, endTime)

print "VMCA Cert"
print "===================="
print certificate.pem
print "===================="


print "Self Signed Certificate"
print "===================="
print selfsigned.pem
print selfsigned
print "===================="

print "CRL Functions"
print "=== Got CRL from VMCA==="
print client.GetCRL("VMCA.crl").filepath

# #enum certtificates
handle = client.OpenEnumHandle("active")
x = 0
while True:
   cert = client.GetNextCertificate(handle)
   if cert is None :
	break
   client.Revoke(cert)
   x = x + 1
   if x > 10:
    break
client.CloseEnumHandle(handle)


print "CRL Functions"
print "=== Got CRL from VMCA==="
print client.GetCRL("VMCA1.crl").filepath



# print "Root CA Certificate"
# print client.GetRootCACert()


# for x in range(0,10000):
# 	version = vmca.GetVersion("127.0.0.1")
# 	print version

