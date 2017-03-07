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

# KeyStore

import sys
import identity.vmafd as vmafd

def getDefaultType():
    return "CDC"

class CdcClient:
    """CdcClient Class"""

    _client_context = None

    def __init__(self, serverName):
        if not serverName:
            serverName = 'localhost'
        self._client_context = vmafd.client(serverName)

    def __del__(self):
        if self._client_context is not None:
            pass

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        if self._client_context is not None:
            pass

    def enableClientAffinity(self):
        self._client_context.EnableClientAffinity()

    def disableClientAffinity(self):
        self._client_context.DisableClientAffinity()

    def affinitizedDC(self, domainName, forceRefresh=0):
        affinitized_dc = self._client_context.GetAffinitizedDC(domainName, forceRefresh)
        return affinitized_dc

    def state(self):
        cdc_state = self._client_context.GetCdcState()
        return cdc_state

    def isEnabled(self):
        cdc_state = self._client_context.GetCdcState()
        if cdc_state == 'UNKNOWN' or cdc_state == 'DISABLED':
            return False
        return True

    def cachedDCEntries(self):
        l = self._client_context.EnumDCEntries()
        for a in l:
            yield a
