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
from BaseHTTPServer import HTTPServer
from SimpleHTTPServer import SimpleHTTPRequestHandler
from contextlib import contextmanager, nested
import tempfile

from identity.vmkeystore import VmKeyStore

@contextmanager
def tempinput(data):
    temp = tempfile.NamedTemporaryFile(delete=False)
    temp.write(data)
    temp.close()
    yield temp.name
    os.unlink(temp.name)

def get_ssl_cert_and_key(store, alias):
    ks = VmKeyStore('VKS')
    ks.load(store)
    cert = ks.get_certificate(alias)
    key = ks.get_key(alias)
    return (cert, key)

def run_https_server(ip, port, key, cert):
    server = HTTPServer((ip, port), SimpleHTTPRequestHandler)
    with nested(tempinput(key), tempinput(cert)) as (tempkeyname, tempcertname):
        server.socket = ssl.wrap_socket(server.socket,
                                        keyfile=tempkeyname,
                                        certfile=tempcertname,
                                        server_side=True)
        sa = server.socket.getsockname()
        print 'Serving HTTPS on %s port %s ...' % (sa[0], sa[1])
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print "Bye"

if __name__ == '__main__':
    if len(sys.argv) < 5:
        print "Usage: %s <address> <port> <store> <alias>" % sys.argv[0]
    else:
        address = sys.argv[1]
        port = int(sys.argv[2])
        store = sys.argv[3]
        alias = sys.argv[4]
        (cert, key) = get_ssl_cert_and_key(store, alias)
        run_https_server(address, port, key, cert)
