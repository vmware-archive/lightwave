/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * **********************************************************************/
package com.vmware.identity.installer;

import java.io.Serializable;
import java.net.URI;
import java.security.cert.X509Certificate;

/**
 * Represents the information stored in a service endpoint
 */
public final class ServiceEndpoint implements Serializable {

   private static final long serialVersionUID = -2111461638747671865L;

   /**
    * Enumerates various endpoint communication protocols
    */
   public static enum EndpointProtocol {
      /**
       * The endpoint protocol is the vSphere API over SOAP.
       */
      vmomi,

      /**
       * The endpoint communication protocol is WS-Trust 1.4
       */
      wsTrust,

      /**
       * Endpoint protocol is REST
       */
      rest,

      /**
       * Endpoint protocol is HTTP or HTTPS.
       */
      http,

      /**
       * The endpoint protocol is not known.
       */
      unknown
   };

   /**
    * The location (URL) of the endpoint
    */
   private final URI _address;

   /**
    * The SSL trust anchor used to validate the service's SSL certificate chain
    */
   private final X509Certificate _sslTrustAnchor;

   /**
    * The protocol used by the service
    */
   private final EndpointProtocol _protocol;

   /**
    * Create a service endpoint setting all its properties. None of them can be
    * <code>null</code>
    *
    * @param address
    *           URL of the endpoint. Cannot be <code>null</code>
    * @param sslTrustAnchor
    *           The SSL trust anchor. Can be <code>null</code>
    * @param protocol
    *           Protocol used by the service. Cannot be <code>null</code>
    */
   public ServiceEndpoint(URI address, X509Certificate sslTrustAnchor,
      EndpointProtocol protocol) {
      ValidateUtil.validateNotNull(address, "Endpoint address");
      ValidateUtil.validateStringMaxLength(address.toASCIIString(),
         "Endpoint address", ValidateUtil.ENDPOINT_MAX_LENGTH);
      ValidateUtil.validateNotNull(protocol, "Endpoint protocol");
      if (sslTrustAnchor != null) {
         ValidateUtil.validateCertificate(sslTrustAnchor);
      }

      _address = address;
      _sslTrustAnchor = sslTrustAnchor;
      _protocol = protocol;
   }

   /**
    * @return The location of the endpoint
    */
   public URI getAddress() {
      return _address;
   }

   /**
    * @return The SSL trust anchor used to validate the service's SSL
    *         certificate chain
    */
   public X509Certificate getSslTrustAnchor() {
      return _sslTrustAnchor;
   }

   /**
    * @return The protocol used by the service
    */
   public EndpointProtocol getProtocol() {
      return _protocol;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + _address.hashCode();
      result = prime * result + _protocol.hashCode();
      if (_sslTrustAnchor != null) {
         result = prime * result + _sslTrustAnchor.hashCode();
      }
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null) {
         return false;
      }
      if (getClass() != obj.getClass()) {
         return false;
      }
      ServiceEndpoint other = (ServiceEndpoint) obj;
      boolean result = _address.equals(other._address)
         && _protocol.equals(other._protocol);
      if (_sslTrustAnchor == null) {
         result = result && other._sslTrustAnchor == null;
      } else {
         result = result && _sslTrustAnchor.equals(other._sslTrustAnchor);
      }
      return result;
   }
}
