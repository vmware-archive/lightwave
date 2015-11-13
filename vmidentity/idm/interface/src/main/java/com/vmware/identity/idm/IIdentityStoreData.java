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

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/8/11
 * Time: 4:37 PM
 * To change this template use File | Settings | File Templates.
 */
public interface IIdentityStoreData extends java.io.Serializable
{
    /**
     * Get the name of the identity store (this is synonymous to domain name ).
     *
     * @return  The name of the identity store, which is the equivalent to the domain name.
     */
    public String getName(); // same as domain name

    /**
     * Get the type of the domain this IdentityStore represents.
     *
     * @return  The type of the domain that this IdentityStore represents.
     *          See the {@link DomainType} enumeration.
     */
    public DomainType getDomainType();

    /**
     * gets the extended information about the Identity Store (See the {@link IIdentityStoreDataEx} interface.).
     * This is only available for identity stores whose <code>DomainType</code> is
     * <code>DomainType.ExternalDomain</code>. For other domain types this will be <code>null</code>.
     *
     * @return  If the identity store's domain type is <code>DomainType.ExternalDomain</code>,
     *          extended information about the Identity Store (See the {@link IIdentityStoreDataEx} interface.) and
     *          <code>null</code> otherwise.
     */
    public IIdentityStoreDataEx getExtendedIdentityStoreData();
}
