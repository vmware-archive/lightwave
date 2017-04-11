package com.vmware.identity.vecs;

import java.security.cert.CRLException;
import java.security.cert.CertificateException;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.Enumeration;
import java.util.List;

public class VecsEntryEnumeration implements Enumeration<VecsEntry>,AutoCloseable {
    private final VMwareEndpointCertificateStore _certStore;
    private final String _storeName;
    private final String _serverName;
    private final String _userName;
    private final PointerRef _storeHandle;
    private final VecsEntryInfoLevel _infoLevel;
    private PointerRef _pEnumContext;
    private static final int ENUM_SIZE = 256;
    private final List<VecsEntryNative> _enumBuffer = new ArrayList<VecsEntryNative>();

    public VecsEntryEnumeration(VMwareEndpointCertificateStore certStore,
                                PointerRef storeHandle,
                                String serverName,
                                String userName, String storeName,
                                VecsEntryInfoLevel infoLevel) {
       _certStore = certStore;
       _storeHandle = storeHandle;
       _storeName = storeName;
       _serverName = serverName;
       _userName = userName;
       _infoLevel = infoLevel;

       _pEnumContext = beginEnumEntries();
       fetchMoreEntries();
    }

    @Override
    public boolean hasMoreElements() {
       checkStatusAndUpdateIfNecessary();
       return !_enumBuffer.isEmpty();
    }

    @Override
    public VecsEntry nextElement() {
       checkStatusAndUpdateIfNecessary();
       VecsEntry resultEntry = null;
       if (!_enumBuffer.isEmpty()) {
          try {
             resultEntry = convertVecsEntryNativeToVecsEntry(_enumBuffer
                   .remove(0));
          } catch (CertificateException ce) {
             IllegalStateException ise = new IllegalStateException(
                   String.format("Certificate generation from PEM string failed."
                         + "[Store: %s, Server: %s, User: %s]",
                         _storeName, _serverName, _userName),
                  ce);
             throw ise;
          } catch (CRLException crle) {
              IllegalStateException ise = new IllegalStateException(
                      String.format("CRL generation from PEM string failed."
                            + "[Store: %s, Server: %s, User: %s]",
                            _storeName, _serverName, _userName),
                     crle);
                throw ise;
             }
       }
       return resultEntry;
    }

    @Override
    protected void finalize() {
       endEnumEntries();
    }

    @Override
    public void close(){
        endEnumEntries();
    }

    private static VecsEntry convertVecsEntryNativeToVecsEntry(
            VecsEntryNative entryNative) throws CertificateException, CRLException {
         String alias = entryNative.alias;
         VecsEntryType type = VecsEntryType.getEntryType(entryNative.entryType);

         X509CRL crl = null;
         X509Certificate[] certs = null;

         if (type == VecsEntryType.CERT_ENTRY_TYPE_CRL) {
            crl = VecsUtils.getX509CRLFromString(entryNative.certificate);
         } else {
            certs = VecsUtils
                  .getX509CertificatesFromString(entryNative.certificate);
         }
         Date date = new Date(entryNative.date*1000);


         return new VecsEntry(type, date, alias, certs, crl);
      }

    private static void BAIL_ON_ERROR(final int error, final String format,
            Object... vargs) {
         switch (error) {
         case 0:
            break;
         default:
            throw new VecsGenericException(String.format(format, vargs), error);
         }
      }

    private void checkStatusAndUpdateIfNecessary() {
       if (_enumBuffer.isEmpty()) {
          fetchMoreEntries();
          if (_enumBuffer.isEmpty()) {
             endEnumEntries();
          }
       }
    }

    private void fetchMoreEntries() {
       List<VecsEntryNative> entries = enumEntries();
       _enumBuffer.addAll(entries);
    }

    private PointerRef beginEnumEntries() {
       PointerRef pEnumContext = new PointerRef();
       int error = VecsAdapter.VecsBeginEnumEntries(_storeHandle, ENUM_SIZE,
             _infoLevel.getValue(), pEnumContext);
       BAIL_ON_ERROR(error, "Begin enum on store '%s' failed. [Server: %s, User: %s]",
             _storeName, _serverName, _userName);

       return pEnumContext;
    }

    private List<VecsEntryNative> enumEntries() {
       List<VecsEntryNative> pEntries = new ArrayList<VecsEntryNative>();
       int error = VecsAdapter.VecsEnumEntriesW(_pEnumContext, pEntries);
       BAIL_ON_ERROR(error, "Enum of entries on store '%s' failed. [Server: %s, User: %s]", _storeName, _serverName, _userName);

       return pEntries;
    }

    private void endEnumEntries() {
       if (_pEnumContext != null && !PointerRef.isNull(_pEnumContext)) {
          int error = VecsAdapter.VecsEndEnumEntries(_pEnumContext);
          BAIL_ON_ERROR(error, "End Enum on store '%s' failed. [Server: %s, User: %s]", _storeName, _serverName, _userName);
          _pEnumContext = null;
       }
    }
}

