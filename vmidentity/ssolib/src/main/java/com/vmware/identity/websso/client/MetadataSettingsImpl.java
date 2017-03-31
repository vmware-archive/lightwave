/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
 */
package com.vmware.identity.websso.client;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.apache.commons.lang.Validate;

/**
 * default implementation of MetadataSettings interface
 *
 */
public class MetadataSettingsImpl implements MetadataSettings {

    private final List<SPConfiguration> spConfigurations;

    private final List<IDPConfiguration> idpConfigurations;

    private final ReentrantReadWriteLock readWriteLock_SP;
    private final ReentrantReadWriteLock readWriteLock_IDP;
    private final Lock readLock_SP;
    private final Lock writeLock_SP;
    private final Lock readLock_IDP;
    private final Lock writeLock_IDP;

    public MetadataSettingsImpl() {
        this.spConfigurations = new LinkedList<SPConfiguration>();
        this.idpConfigurations = new LinkedList<IDPConfiguration>();
        this.readWriteLock_SP = new ReentrantReadWriteLock();
        this.readWriteLock_IDP = new ReentrantReadWriteLock();
        this.readLock_SP = this.readWriteLock_SP.readLock();
        this.writeLock_SP = this.readWriteLock_SP.writeLock();
        this.readLock_IDP = this.readWriteLock_IDP.readLock();
        this.writeLock_IDP = this.readWriteLock_IDP.writeLock();
    }

    /*
     * (non-Javadoc) Add sp configuration.
     */
    @Override
    public void addSPConfiguration(SPConfiguration spConfiguration) {
        if (spConfiguration == null) {
            return;
        }
        try {
            this.writeLock_SP.lock();
            this.spConfigurations.add(spConfiguration);
        } finally {
            this.writeLock_SP.unlock();
        }
    }

    /*
     * (non-Javadoc) update IDP configuration with given SPConsiguration
     */
    @Override
    public void updateSPConfiguration(SPConfiguration spConfiguration) throws WebssoClientException {

        this.writeLock_SP.lock();
        try {
            removeSPConfiguration(spConfiguration.getAlias());
            addSPConfiguration(spConfiguration);
        } finally {
            this.writeLock_SP.unlock();
        }

    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.websso.client.MetadataSettings#getAllSPConfigurations
     * ()
     */
    @Override
    public Collection<SPConfiguration> getAllSPConfigurations() {
        return this.spConfigurations;
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.websso.client.MetadataSettings#getSPConfiguration
     * (java.lang.String)
     */
    @Override
    public SPConfiguration getSPConfiguration(String alias) {

        Validate.notNull(alias, (this.getClass() + " null argument"));
        SPConfiguration retConfig = null;

        try {
            this.readLock_SP.lock();

            Iterator<SPConfiguration> iterator = this.spConfigurations.iterator();
            while (iterator.hasNext()) {
                SPConfiguration config = iterator.next();
                if (config.getAlias().equals(alias)) {
                    retConfig = config;
                    break;
                }
            }
        } finally {
            this.readLock_SP.unlock();
        }
        return retConfig;
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.websso.client.MetadataSettings#getSPConfiguration
     * (java.lang.String)
     */
    @Override
    public SPConfiguration getSPConfigurationByEntityID(String entityId) {

        Validate.notNull(entityId, (this.getClass() + " null argument"));
        SPConfiguration retConfig = null;

        try {
            this.readLock_SP.lock();

            Iterator<SPConfiguration> iterator = this.spConfigurations.iterator();
            while (iterator.hasNext()) {
                SPConfiguration config = iterator.next();
                if (config.getEntityID().equals(entityId)) {
                    retConfig = config;
                    break;
                }
            }
        } finally {
            this.readLock_SP.unlock();
        }
        return retConfig;
    }

    /*
     * (non-Javadoc) Remove SP configuration identified by its alias name
     */
    @Override
    public SPConfiguration removeSPConfiguration(String alias) {

        Validate.notNull(alias, (this.getClass() + " null argument"));
        SPConfiguration retConfig = null;
        try {
            this.writeLock_SP.lock();
            Iterator<SPConfiguration> iterator = this.spConfigurations.iterator();
            while (iterator.hasNext()) {
                SPConfiguration config = iterator.next();
                if (config.getAlias().equals(alias)) {
                    this.spConfigurations.remove(config);
                    retConfig = config;
                    break;
                }
            }
        } finally {
            this.writeLock_SP.unlock();
        }
        return retConfig;
    }

    /*
     * (non-Javadoc) Add an IDP configuration.
     */
    @Override
    public void addIDPConfiguration(IDPConfiguration config) {
        if (config == null) {
            return;
        }
        try {
            this.writeLock_IDP.lock();
            this.idpConfigurations.add(config);
        } finally {
            this.writeLock_IDP.unlock();
        }
    }

    /*
     * (non-Javadoc) Update IDP configuration with a passed in configuration.
     */
    @Override
    public void updateIDPConfiguration(IDPConfiguration idpConfiguration) throws WebssoClientException {
        try {
            this.writeLock_IDP.lock();
            removeIDPConfiguration(idpConfiguration.getAlias());
            addIDPConfiguration(idpConfiguration);
        } finally {
            this.writeLock_IDP.unlock();
        }
    }

    /*
     * (non-Javadoc) Return all IDP configurations.
     */
    @Override
    public Collection<IDPConfiguration> getAllIDPConfigurations() {
        return this.idpConfigurations;
    }

    /*
     * (non-Javadoc) Get IDP configuration identified by alias name
     */
    @Override
    public IDPConfiguration getIDPConfiguration(String alias) {

        Validate.notNull(alias, (this.getClass() + " null argument"));
        IDPConfiguration retConfig = null;

        try {
            this.readLock_IDP.lock();
            Iterator<IDPConfiguration> iterator = this.idpConfigurations.iterator();
            while (iterator.hasNext()) {
                IDPConfiguration config = iterator.next();
                if (config.getAlias().equals(alias)) {
                    retConfig = config;
                    break;
                }
            }
        } finally {
            this.readLock_IDP.unlock();
        }
        return retConfig;
    }

    /*
     * (non-Javadoc) Get IDP configuration identified by entityID.
     */
    @Override
    public IDPConfiguration getIDPConfigurationByEntityID(String id) {

        Validate.notNull(id, (this.getClass() + " null argument"));
        IDPConfiguration retConfig = null;

        try {
            this.readLock_IDP.lock();

            Iterator<IDPConfiguration> iterator = this.idpConfigurations.iterator();
            while (iterator.hasNext()) {
                IDPConfiguration config = iterator.next();
                if (config.getEntityID().equals(id)) {
                    retConfig = config;
                    break;
                }
            }

            if (retConfig == null) {
                /**
                 * handle fail-over scenarios: compare the id with each registered node of the domain.
                 */
                iterator = this.idpConfigurations.iterator();
                while (iterator.hasNext()) {
                    IDPConfiguration config = iterator.next();
                    if (config.isSameEntity(id)) {
                        retConfig = config;
                        break;
                    }
                }
            }
        } finally {
            this.readLock_IDP.unlock();
        }
        return retConfig;
    }

    /*
     * (non-Javadoc) Remove IDP configuration identified by alias name
     */
    @Override
    public IDPConfiguration removeIDPConfiguration(String alias) {
        Validate.notNull(alias, (this.getClass() + " null argument"));
        IDPConfiguration retConfig = null;

        try {
            this.writeLock_IDP.lock();

            Iterator<IDPConfiguration> iterator = this.idpConfigurations.iterator();
            while (iterator.hasNext()) {
                IDPConfiguration config = iterator.next();
                if (config.getAlias().equals(alias)) {
                    this.idpConfigurations.remove(config);
                    retConfig = config;
                    break;
                }
            }
        } finally {
            this.writeLock_IDP.unlock();
        }
        return retConfig;
    }

    /*
     * Clean up collections.
     */
    @Override
    public void clear() {
        try {
            this.writeLock_SP.lock();
            this.writeLock_IDP.lock();
            this.spConfigurations.clear();
            this.idpConfigurations.clear();
        } finally {
            this.writeLock_SP.unlock();
            this.writeLock_IDP.unlock();
        }
    }

    @Override
    public void StartRebuilding() {
        this.writeLock_SP.lock();
        this.writeLock_IDP.lock();
    }

    @Override
    public void EndRebuilding() {
        this.writeLock_SP.unlock();
        this.writeLock_IDP.unlock();
    }

}
