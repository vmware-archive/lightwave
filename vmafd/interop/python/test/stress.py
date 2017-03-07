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

import sys
from identity.vmkeystore import VmKeyStore

def fetch(ks, store, alias):
    ks.load(store)
    key = ks.get_key(alias)
    cert = ks.get_certificate(alias)
    return (key, cert)

def test_keystore(ks, store, key, cert, count):

    print "*** create ***"

    ks.create(store)

    print "*** load ***"

    ks.load(store)

    print "*** set_key_entry ***"

    for i in range(count):
        ks.set_key_entry("alias"+str(i), key, cert)

    print "*** size ***"

    count = ks.size()
    print "size: %d" % count

    print "*** aliases ***"

    print "len(list(ks.aliases()) = %d" % len(list(ks.aliases()))

if __name__ == "__main__":

    store = sys.argv[1]
    count = int(sys.argv[2])

    ks = VmKeyStore('VKS')
    (key, cert) = fetch(ks, '__SYSTEM_STORE__', '__MACHINE_CERT')

    test_keystore(ks, store, key, cert, count)
