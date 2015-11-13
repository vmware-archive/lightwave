/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */
package com.vmware.identity.idm.server.config.directory;

import java.io.Serializable;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.ObjectUtils;

import com.vmware.identity.idm.ValidateUtil;

public final class TenantAttributes implements Serializable
{
    private static final long serialVersionUID = 713263201913174644L;

    private TokenPolicy _tokenPolicy;
    private final String _signatureAlgorithm;
    private final String _brandName;
    private final String _logonBannerTitle;
    private final String _logonBannerContent;
    private boolean _logonBannerEnableCheckbox;
    private final String _entityId;
    private final String _entityAlias;
    private final int[] _authnTypes;
    private final boolean _enableIdpSelection;

    public TenantAttributes( TokenPolicy tokenPolicy, String signatureAlgorithm, String brandName,
            String logonBannerTitle, String logonbannerContent, boolean logonBannerEnableCheckbox,
            String entityId, String entityAlias, int[] authnTypes, boolean enableIdpSelection)
    {
        ValidateUtil.validateNotNull( tokenPolicy, "tokenPolicy" );

        this._tokenPolicy = tokenPolicy;
        this._signatureAlgorithm = signatureAlgorithm;
        this._brandName = brandName;
        this._logonBannerTitle = logonBannerTitle;
        this._logonBannerContent = logonbannerContent;
        this._logonBannerEnableCheckbox = logonBannerEnableCheckbox;
        this._entityId = entityId;
        this._entityAlias = entityAlias;
        this._authnTypes = authnTypes;
        this._enableIdpSelection = enableIdpSelection;
    }

    public TokenPolicy getTokenPolicy()
    {
        return this._tokenPolicy;
    }

    public String getSignatureAlgorithm()
    {
        return this._signatureAlgorithm;
    }

    public String getBrandName()
    {
        return this._brandName;
    }

    public String getLogonBannerTitle()
    {
        return this._logonBannerTitle;
    }

    public String getLogonBannerContent()
    {
        return this._logonBannerContent;
    }

    public boolean getLogonBannerCheckboxFlag()
    {
        return this._logonBannerEnableCheckbox;
    }

    public String getEntityId()
    {
        return this._entityId;
    }

    public String getAlias()
    {
        return this._entityAlias;
    }

    public int[] getAuthnTypes(){
        return this._authnTypes;
    }

    public boolean isIDPSelectionEnabled() {
        return this._enableIdpSelection;
    }

   @Override
   public int hashCode()
   {
      final int prime = 31;
      int result = 1;

      result = prime * result + _tokenPolicy.hashCode();
      result = prime * result
                 + ((_signatureAlgorithm == null) ? 0 : _signatureAlgorithm.hashCode());
      result = prime * result
              + ((_brandName == null) ? 0 : _brandName.hashCode());
      result = prime * result
              + ((_logonBannerTitle == null) ? 0 : _logonBannerTitle.hashCode());
      result = prime * result
              + ((_logonBannerContent == null) ? 0 : _logonBannerContent.hashCode());
      result = prime * result
              + ((_entityId == null) ? 0 : _entityId.hashCode());
      result = prime * result
              + ((_entityAlias == null) ? 0 : _entityAlias.hashCode());
      result = prime * result
              + ArrayUtils.hashCode(_authnTypes);

      return result;
   }

   @Override
   public boolean equals(Object other)
   {
       boolean result = false;

       if (this == other)
       {
           result = true;
       }
       else if (other != null && other instanceof TenantAttributes)
       {
           TenantAttributes peer = (TenantAttributes)other;

           result = ( peer._tokenPolicy == _tokenPolicy ) &&
                    ( ObjectUtils.equals(_signatureAlgorithm, peer._signatureAlgorithm) ) &&
                    ( ObjectUtils.equals(_brandName, peer._brandName) ) &&
                    ( ObjectUtils.equals(_logonBannerTitle, peer._logonBannerTitle) ) &&
                    ( ObjectUtils.equals(_logonBannerContent, peer._logonBannerContent) ) &&
                    ( ObjectUtils.equals(_entityId, peer._entityId) ) &&
                    ( ObjectUtils.equals(_entityAlias, peer._entityAlias) ) &&
                    ( ArrayUtils.isEquals(_authnTypes, peer._authnTypes) );
       }

       return result;
   }
}
