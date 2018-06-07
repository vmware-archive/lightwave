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

package com.vmware.identity.idm;

import java.io.Serializable;

/**
 *  OperatorAccessPolicy specifies setting around operator access.
 *      for example: enabled/disabled; etc.
 *
 */
public class OperatorAccessPolicy implements Serializable {

    private static final long serialVersionUID = -4421732797183065313L;

    private boolean _enabled = false;
    private String _userBaseDn = null;
    private String _groupBaseDn = null;

    /**
     * @param enabled
     * @param userBaseDn
     * @param groupBaseDn
     */
    private OperatorAccessPolicy(boolean enabled, String userBaseDn, String groupBaseDn) {
        this._enabled = enabled;
        this._userBaseDn = userBaseDn;
        this._groupBaseDn = groupBaseDn;
    }

    /*
     * if operators access enabled or not
     */
    public boolean enabled() {
        return this._enabled;
    }

    /*
     * user Base Dn
     */
    public String userBaseDn() {
        return this._userBaseDn;
    }

    /*
     * group Base Dn
     */
    public String groupBaseDn() {
        return this._groupBaseDn;
    }

    public static class Builder {

        private boolean _enabled = false;
        private String _userBaseDn = null;
        private String _groupBaseDn = null;

        public Builder() {}

        public Builder withEnabled(boolean enabled) {
            this._enabled = enabled;
            return this;
        }
        public Builder withUserBaseDn(String userBaseDn) {
            this._userBaseDn = userBaseDn;
            return this;
        }

        public Builder withGroupBaseDn(String groupBaseDn) {
            this._groupBaseDn = groupBaseDn;
            return this;
        }

        public OperatorAccessPolicy build() {

            // either not set, or dn
            if (ValidateUtil.isEmpty(this._userBaseDn)) {
                this._userBaseDn = null;
            } else {
                ValidateUtil.validateDNFormat(this._userBaseDn);
            }
            // either not set, or dn
            if (ValidateUtil.isEmpty(this._groupBaseDn)) {
                this._groupBaseDn = null;
            } else {
                ValidateUtil.validateDNFormat(this._groupBaseDn);
            }

            return new OperatorAccessPolicy(this._enabled, this._userBaseDn, this._groupBaseDn);
        }
    }
}
