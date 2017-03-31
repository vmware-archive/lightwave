#!/usr/bin/env python

#
# Copyright (C) 2012-2015 VMware, Inc.  All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy
# of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, without
# warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
# License for the specific language governing permissions and limitations
# under the License.
#

import os
import sys
import ssl
from contextlib import contextmanager, nested
import tempfile
import socket
import pprint

from identity.vmkeystore import VmKeyStore

@contextmanager
def tempinput(data):
    temp = tempfile.NamedTemporaryFile(delete=False)
    temp.write(data)
    temp.close()
    yield temp.name
    os.unlink(temp.name)

def get_ca_cert():
    ks = VmKeyStore('VKS')
    ks.load("TRUSTED_ROOTS")
    ca_cert = ks.get_certificate("VMCA-Root-Certificate")
    print ca_cert
    return ca_cert

def run_https_client(ip, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ca_cert = get_ca_cert()
    with tempinput(ca_cert) as tempcertname:
        ssl_sock = ssl.wrap_socket(s,
                                   ca_certs=tempcertname,
                                   cert_reqs=ssl.CERT_REQUIRED)
        ssl_sock.connect((ip, port))

        print repr(ssl_sock.getpeername())
        print pprint.pformat(ssl_sock.getpeercert())

        ssl_sock.write("GET / HTTP/1.0\r\nHost: www.vmware.com\r\n\r\n")
        data = ssl_sock.read()
        ssl_sock.close()

        print 'Received from HTTPS server:'
        print data

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print "Usage: %s <address> <port>" % sys.argv[0]
    else:
        address = sys.argv[1]
        port = int(sys.argv[2])

        run_https_client(address, port)
