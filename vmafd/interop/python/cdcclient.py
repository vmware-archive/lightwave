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

    def affinitizedDC(self, domainName):
        affinitized_dc = self._client_context.GetAffinitizedDC(domainName)
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
