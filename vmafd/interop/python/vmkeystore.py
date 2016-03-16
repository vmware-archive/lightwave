#
# Copyright 2012-2015 VMware, Inc.  All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"): you may not
# use this file except in compliance with the License.  You may obtain a copy
# of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, without
# warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
# License for the specific language governing permissions and limitations
# under the License.

# KeyStore

import sys
import identity.vmafd as vmafd

def getDefaultType():
    return "VKS"

class VmKeyStore:
    """VmKeyStore Class"""

    _client = None
    _store_type = None
    _store_context = None

    def __init__(self, store_type):
        self._client = vmafd.client('localhost')
        self._store_type = store_type

    def __del__(self):
        if self._store_context is not None:
            self._client.CloseCertStore(self._store_context)
            self._store_context = None

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        if self._store_context is not None:
            self._client.CloseCertStore(self._store_context)
            self._store_context = None

    def aliases(self):
        self.enum_context = self._client.BeginEnumAliases(self._store_context, 10)
        while True:
            l = self._client.EnumAliases(self.enum_context)
            if len(l) == 0:
                break
            for a in l:
                yield a
        self._client.EndEnumAliases(self.enum_context)

    def contains_alias(self, alias):
        pass

    def delete_entry(self, alias):
        self._client.DeleteCert(self._store_context, alias)

    def get_certificate(self, alias):
        cert_data = self._client.GetCertByAlias(self._store_context, alias)
        return cert_data

    def get_certificate_chain(self, alias):
        chain = self._client.GetCertChain(self._store_context, alias)
        return chain

    def get_entry(self, alias, protParam):
        pass

    def get_instance(self, type, provider):
        pass

    def get_key(self, alias, password=''):
        key_data = self._client.GetPrivateKeyByAlias(self._store_context, alias, password)
        return key_data

    def get_provider(self):
        pass

    def get_type(self):
        pass

    def is_certificate_entry(self, alias):
        pass

    def is_key_entry(self, alias):
        pass

    def load(self, store_name, password=''):
        if self._store_context is not None:
            self._client.CloseCertStore(self._store_context)
        self._store_context = self._client.OpenCertStore(store_name, password)

    def set_certificate_entry(self, alias, cert, password=''):
        key = ''
        entry_type = 3
        self._client.AddCert(self._store_context, entry_type, alias, cert, key, password, True)

    def set_entry(self, alias, entry, protParam):
        pass

    def set_key_entry(self, alias, key, cert, password=''):
        entry_type = 1
        self._client.AddCert(self._store_context, entry_type, alias, cert, key, password, True)

    def size(self):
        count = self._client.GetEntryCount(self._store_context)
        return count

    def store(self, param):
        pass

    def create(self, name, password=''):
        self._client.CreateCertStore(name, password)

    def close(self):
        self._client.CloseCertStore(self._store_context)
