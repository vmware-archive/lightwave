/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

import java.net.URI;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.SystemUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Pointer;
import com.sun.jna.TypeMapper;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.win32.W32APITypeMapper;
import com.vmware.identity.interop.PlatformUtils;

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/19/11
 * Time: 1:36 PM
 * To change this template use File | Settings | File Templates.
 */
class LdapConnection implements ILdapConnectionExWithGetConnectionString
{
    private static final Log log = LogFactory.getLog(LdapConnection.class);
    private final static Log perfLog =
            LogFactory.getLog(PlatformUtils.PERFDATASINK_LOG);
    private static int DEFAULT_MAX_SEARCHPAGE_RETURN = 1000;

    private Pointer _connection = Pointer.NULL;
    private LdapConnectionCtx _connectionContext;

    private String _connectionString;

    private ILdapClientLibrary _ldapClientLibrary;

    /**
     * @deprecated  replaced by {@link #LdapConnection(URI, List<LdapSetting>, boolean) LdapConnection}
     * @param hostname
     * @param port
     */
    @Deprecated
    LdapConnection(String hostname, int port)
    {
        _ldapClientLibrary = LdapClientLibraryFactory.getInstance().getLdapClientLibrary(false);
        this._connection = _ldapClientLibrary.ldap_init( hostname, port );
        this._connectionContext = new LdapConnectionCtx(this._connection, null, LdapSSLProtocols.getDefaultMinProtocol().getCode());
        this._connectionString = String.format("ldap://%s:%d", hostname, port);
    }

    LdapConnection(URI uri, List<LdapSetting> connOptions)
    {
        this(uri, connOptions, false);
    }

    LdapConnection(URI uri, List<LdapSetting> connOptions, boolean useWindowsOpenLdapLib) {
        _ldapClientLibrary = LdapClientLibraryFactory.getInstance().getLdapClientLibrary(useWindowsOpenLdapLib);
        this._connectionString = uri.toString();
        this._connectionContext = _ldapClientLibrary.ldap_initializeWithUri(
                uri, connOptions);
        this._connection = _connectionContext.getConnection();
    }

    @Override
    public void setOption(LdapOption option, int value)
    {
        this.validate();
        ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        IntByReference pVal = new IntByReference(value);

        ldapClientLibrary.ldap_set_option(
            this._connection, option.getCode(), pVal.getPointer()
        );
    }

    @Override
    public void setOption(LdapOption option, boolean value)
    {
        this.validate();
        ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        if (value)
        {
            IntByReference pVal = new IntByReference(1);

            ldapClientLibrary.ldap_set_option(
                                    _connection,
                                    option.getCode(),
                                    pVal.getPointer());
        }
        else
        {
            ldapClientLibrary.ldap_set_option(
                                    _connection,
                                    option.getCode(),
                                    Pointer.NULL);
        }
    }

    @Override
    public void bindConnection(String dn, String cred, LdapBindMethod method)
    {
        this.validate();

        ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        long startedAt = System.nanoTime();
        try {
            ldapClientLibrary.ldap_bind_s( this._connectionContext, dn, cred, method.getCode() );
        }finally {
            if (perfLog.isTraceEnabled()) {
                perfLog.trace(String.format(
                    "bindConnection took [%d]ms", TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedAt)));
            }
        }
    }

    @Override
    public void bindSaslConnection(String userName, String domainName, String userPassword)
    {
        this.validate();

        ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        long startedAt = System.nanoTime();
        try{
            ldapClientLibrary.ldap_sasl_bind_s(this._connection, userName, domainName, userPassword);
        }
        finally
        {
            if (perfLog.isTraceEnabled()) {
                perfLog.trace(String.format(
                    "bindConnection took [%d]ms", TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedAt)));
            }
        }
    }

    @Override
    public void addObject(String dn, LdapMod[] attributes)
    {
        this.addObject(dn, ( (attributes != null) ? Arrays.asList(attributes) : null ) );
    }

    @Override
    public void addObject(final String dn, final Collection<LdapMod> attributes)
    {
        this.validate();

        final ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        final Pointer[] attrs;
        LdapModNative[] nativeAttrs = null;//hold on to the allocated native memory

        try {
            if ( (attributes != null) && (attributes.isEmpty() == false) )
            {
                nativeAttrs = new LdapModNative[attributes.size()];
                attrs = new Pointer[attributes.size() + 1];

                if (log.isTraceEnabled()) {
                    log.trace(String.format("addObject: dn=[%s]", dn));
                }
                int iAttr = 0;
                for (LdapMod attr : attributes)
                {
                    nativeAttrs[iAttr] = new LdapModNative(attr, ldapClientLibrary.getTypeMapper());
                    attrs[iAttr] = nativeAttrs[iAttr].getPointer();
                    iAttr++;
                }

                attrs[attrs.length - 1] = Pointer.NULL;
            }
            else
            {
                attrs = null;
            }

            if (perfLog.isTraceEnabled()) {
                perfLog.trace(String.format("addObject context: dn=[%s]", dn));
            }
            execute(
                    new Callable<Void>() {
                        @Override
                        public Void call() {
                            ldapClientLibrary.ldap_add_s(
                                    LdapConnection.this.getRawConnection(),
                                    dn,
                                    attrs);
                            return null;
                        }
                    });
        } finally
        {
            disposeLdapModNative(nativeAttrs);
        }
    }

    @Override
    public void modifyObject(String dn, LdapMod[] attributes)
    {
        this.modifyObject( dn,  ((attributes != null) ? Arrays.asList(attributes) : null) );
    }

    @Override
    public void modifyObject( String dn, LdapMod attribute )
    {
        this.modifyObject( dn, new LdapMod[] {attribute} );
    }

    @Override
    public void modifyObject(final String dn, Collection<LdapMod> attributes)
    {
        this.validate();

        final ILdapClientLibrary ldapClientLibrary = getLdapLibrary();
        final Pointer[] attrs;
        LdapModNative[] nativeAttrs = null;//hold on to the allocated native memory

        try {
            if (attributes != null && attributes.size() > 0)
            {
                nativeAttrs =
                        new LdapModNative[attributes.size() + 1];
                attrs = new Pointer[attributes.size()];

                if (log.isTraceEnabled()) {
                    log.trace(String.format("modifyObject: dn=[%s]", dn));
                }
                int iAttr = 0;
                for (LdapMod attr : attributes)
                {
                    nativeAttrs[iAttr] = new LdapModNative(attr, ldapClientLibrary.getTypeMapper());

                    attrs[iAttr] = nativeAttrs[iAttr].getPointer();

                    iAttr++;
                }
            }
            else
            {
                attrs = null;
            }

            if (perfLog.isTraceEnabled()) {
                perfLog.trace(String.format("modifyObject context: dn=[%s]", dn));
            }
            execute(
                    new Callable<Void>() {
                        @Override
                        public Void call() {
                            ldapClientLibrary.ldap_modify_s(
                                    LdapConnection.this.getRawConnection(),
                                    dn,
                                    attrs);
                            return null;
                        }
                    });
        }
        finally {
            disposeLdapModNative(nativeAttrs);
        }
    }

    @Override
    public ILdapMessage search(String base, LdapScope scope, String filter, String[] attributes, boolean attributesOnly)
    {
        return this.search(
            base, scope, filter, ((attributes != null) ? Arrays.asList(attributes) : null), attributesOnly
        );
    }

    @Override
    public ILdapMessage search(final String base, final LdapScope scope, final String filter,
            Collection<String> attributes, final boolean attributesOnly)
    {
        this.validate();

        final ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        final String[] attrs;
        // null terminate array if needed
        if ( attributes != null )
        {
            attrs = new String[attributes.size() + 1]; // + 1 for null termination
            attributes.toArray(attrs);
            attrs[attrs.length - 1] = null; // null termination
        }
        else
        {
            attrs = null;
        }

        if (perfLog.isTraceEnabled()) {
            perfLog.trace(String.format(
                "search context: base=[%s], scope: [%s], filter: [%s], attributes: [%s], attributesOnly: [%s]",
                base, scope, filter, Arrays.toString(attrs), Boolean.toString(attributesOnly)));
        }
        Pointer p = execute(
                new Callable<Pointer>() {
                    @Override
                    public Pointer call() {
                        return ldapClientLibrary.ldap_search_s(
                              LdapConnection.this.getRawConnection(),
                              base,
                              scope.getCode(),
                              filter,
                              attrs,
                              attributesOnly ? 1 : 0);
                    }
                });

       Set<String> requestedAttributes = new HashSet<String>();
       if ( ( attributes != null ) && (attributes.size() > 0) )
       {
           requestedAttributes = new HashSet<String>(attributes.size());
           for(String attribute : attributes)
           {
               requestedAttributes.add(attribute);
           }
       }

       return new LdapMessage(this, p, requestedAttributes);
    }

    @Override
    public ILdapMessage search_ext(final String base,
                                   final LdapScope scope,
                                   final String filter,
                                   final Collection<String> attributes,
                                   final boolean attributesOnly,
                                   Collection<LdapControlNative> sctrls,
                                   Collection<LdapControlNative> cctrls,
                                   final TimevalNative timeout,
                                   final int sizelimit)
    {
        this.validate();

        final ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        final String[] attrs;
        // null terminate array if needed
        if ( attributes != null )
        {
            attrs = new String[attributes.size() + 1]; // + 1 for null termination
            attributes.toArray(attrs);
            attrs[attrs.length - 1] = null; // null termination
        }
        else
        {
            attrs = null;
        }

        final Pointer[] sctrlPtrs;
        int iCtrl = 0;

        if (sctrls != null)
        {
            sctrlPtrs = new Pointer[sctrls.size()];

            for (LdapControlNative sctrl : sctrls)
            {
                sctrlPtrs[iCtrl++] = sctrl.getPointer();
            }
        }
        else
        {
            sctrlPtrs = null;
        }

        final Pointer[] cctrlPtrs;

        if (cctrls != null)
        {
            cctrlPtrs = new Pointer[cctrls.size()];

            iCtrl = 0;
            for (LdapControlNative cctrl : cctrls)
            {
                cctrlPtrs[iCtrl++] = cctrl.getPointer();
            }
        }
        else
        {
            cctrlPtrs = null;
        }

         Pointer p = execute(
                    new Callable<Pointer>() {
                        @Override
                        public Pointer call() {
                            return ldapClientLibrary.ldap_search_ext_s(
                                    LdapConnection.this.getRawConnection(),
                                    base,
                                    scope.getCode(),
                                    filter,
                                    attrs,
                                    attributesOnly ? 1 : 0,
                                    sctrlPtrs,
                                    cctrlPtrs,
                                    (timeout != null) ? timeout.getPointer() : null,
                                    sizelimit);
                        }
                    });

        Set<String> requestedAttributes = new HashSet<String>();
        if ( ( attributes != null ) && (attributes.size() > 0) )
        {
            requestedAttributes = new HashSet<String>(attributes.size());
            for(String attribute : attributes)
            {
                requestedAttributes.add(attribute);
            }
        }

        return new LdapMessage(this, p, requestedAttributes);
    }

    @Override
    public
    ILdapPagedSearchResult
    search_one_page(
        final String base,
        final LdapScope scope,
        final String filter,
        final Collection<String> attributes,
        final int pageSize,
        final ILdapPagedSearchResult prevPagedSearchResult
        )
    {
        final String[] attrs;
        // null terminate array if needed
        if ( attributes != null )
        {
            attrs = new String[attributes.size() + 1]; // + 1 for null termination
            attributes.toArray(attrs);
            attrs[attrs.length - 1] = null; // null termination
        }
        else
        {
            attrs = null;
        }

        Set<String> requestedAttributes = new HashSet<String>();
        if ( ( attributes != null ) && (attributes.size() > 0) )
        {
            requestedAttributes = new HashSet<String>(attributes.size());
            for(String attribute : attributes)
            {
                requestedAttributes.add(attribute);
            }
        }

        LdapPagedSearchResultPtr pagedSearchPtr = search_one_page_internal(
                                                            base,
                                                            scope,
                                                            filter,
                                                            attrs,
                                                            pageSize,
                                                            prevPagedSearchResult!=null ?
                                                                    ((LdapPagedSearchResult)prevPagedSearchResult).berCookiePtr() : Pointer.NULL);

        LdapMessage ldapMessage = new LdapMessage(this, pagedSearchPtr.getSearchLdapMessagePtr(),requestedAttributes);

        return new LdapPagedSearchResult(ldapMessage,
                                         pagedSearchPtr.getCurrentCookiePtr(),
                                         pagedSearchPtr.isSearchFinished());
    }

    /* This API will be used in IDM - AD provider
    /* Limit the behavior of the pagedSearch to return no more than limit # of search results
     *
     * This protects IDM not to cache too many search results
     * (non-Javadoc)
     * @see com.vmware.identity.interop.ldap.ILdapConnectionEx#paged_search(java.lang.String, com.vmware.identity.interop.ldap.LdapScope, java.lang.String, java.util.Collection, int)
     */
    @Override
    public
    Collection<ILdapMessage>
    paged_search(
        final String base,
        final LdapScope scope,
        final String filter,
        final Collection<String> attributes,
        final int pageSize,
        final int limit
        )
    {
        int pagedCount = 0;
        ArrayList<ILdapMessage> messages = new ArrayList<ILdapMessage>();

        final String[] attrs;
        // null terminate array if needed
        if ( attributes != null )
        {
            attrs = new String[attributes.size() + 1]; // + 1 for null termination
            attributes.toArray(attrs);
            attrs[attrs.length - 1] = null; // null termination
        }
        else
        {
            attrs = null;
        }

        Set<String> requestedAttributes = new HashSet<String>();
        if ( ( attributes != null ) && (attributes.size() > 0) )
        {
            requestedAttributes = new HashSet<String>(attributes.size());
            for(String attribute : attributes)
            {
                requestedAttributes.add(attribute);
            }
        }

        boolean isSearchFinished = false;
        Pointer berCookiePtr = Pointer.NULL;
        boolean continueToSearch = true;
        int adjusted_pageSize = pageSize;

        // if client wants less than the current page size
        // simply set the pageSize to be limit (no more than what client wants)
        if (limit > 0 && limit < pageSize)
        {
            adjusted_pageSize = limit;
        }

        try
        {
            while (!isSearchFinished && continueToSearch)
            {
                int currPageSize = adjusted_pageSize;
                if (limit <= 0)
                {
                    if (pagedCount >= DEFAULT_MAX_SEARCHPAGE_RETURN)
                    {
                        throw new SizeLimitExceededLdapException(LdapErrors.LDAP_SIZELIMIT_EXCEEDED.getCode(),
                                                                 String.format("paged_search has more than %d pages of results to return",
                                                                 DEFAULT_MAX_SEARCHPAGE_RETURN));
                    }
                }
                else
                {
                    if (pagedCount < limit/adjusted_pageSize)
                    {
                        currPageSize = adjusted_pageSize;
                    }
                    else
                    {
                        currPageSize = limit%adjusted_pageSize;
                        if (currPageSize == 0)
                        {
                            break;
                        }
                        continueToSearch = false;
                    }
                }

                LdapPagedSearchResultPtr pagedSearchPtr = search_one_page_internal(
                                                                          base,
                                                                          scope,
                                                                          filter,
                                                                          attrs,
                                                                          currPageSize,
                                                                          berCookiePtr);

                // free berCookiePtr
                if (berCookiePtr != null && berCookiePtr != Pointer.NULL)
                {
                    getLdapLibrary().ber_bvfree(berCookiePtr);
                    berCookiePtr = Pointer.NULL;
                }

                if (pagedSearchPtr != null)
                {
                    berCookiePtr = pagedSearchPtr.getCurrentCookiePtr();
                    isSearchFinished = pagedSearchPtr.isSearchFinished();

                    messages.add(new LdapMessage(this, pagedSearchPtr.getSearchLdapMessagePtr(), requestedAttributes));
                    pagedCount++;
                }
            }
        }
        finally
        {
            if (berCookiePtr != null && berCookiePtr != Pointer.NULL)
            {
                getLdapLibrary().ber_bvfree(berCookiePtr);
                berCookiePtr = Pointer.NULL;
            }
        }

        return messages;
    }

    @Override
    public void deleteObject(final String dn)
    {
        this.validate();

        final ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        if (perfLog.isTraceEnabled()) {
            perfLog.trace(String.format("deleteObject dn=[%s]", dn));
        }
        execute(
                new Callable<Void>(){
                    @Override
                    public Void call() {
                        ldapClientLibrary.ldap_delete_s(
                                LdapConnection.this.getRawConnection(),
                                dn);
                        return null;
                    }
                });
    }

    @Override
    public void deleteObjectTree(String dn)
    {
        validate();

        deleteObjectTreeInternal(getLdapLibrary(), dn);
    }
    @Override
    public void bindSaslConnection(String userName, String domainName,
            String userPassword, String krbConfPath) {
        bindSaslConnection(userName, domainName, userPassword);
    }

    @Override
    public Collection<ILdapMessage> paged_search(String base, LdapScope scope,
            String filter, Collection<String> attributes, int pageSize) {
        // compatibility impl. '-1' will allow no limit checking
        return paged_search(base, scope, filter, attributes,pageSize, -1);
    }

    @Override
    public String getConnectionString() {
        return this._connectionString;
    }

    @Override
    public void bindSaslSrpConnection(String upn, String userPassword) {
        this.validate();

        ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        long startedAt = System.nanoTime();
        try {
            ldapClientLibrary.ldap_sasl_srp_bind_s(this._connection, upn, userPassword);
        }finally {
            if (perfLog.isTraceEnabled()) {
                perfLog.trace(String.format(
                    "bindSaslSrpConnection took [%d]ms", TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedAt)));
            }
        }
    }

    @Override
    protected void finalize() throws Throwable
    {
        try
        {
            this.close();
        }
        finally
        {
            super.finalize();
        }
    }

    @Override
    public void close()
    {
        if (_connection != Pointer.NULL)
        {

            try
            {
                getLdapLibrary().ldap_unbind(_connection);
            }
            catch(LdapException ex) // we don't want close to throw;
            {
                log.warn("Exception when trying to unbind connection", ex);
            }
            _connection = Pointer.NULL;
        }
    }

    private <T> T execute(Callable<T> ldapOp)
    {
        T result  = null;
        long startedAt = System.nanoTime();
        try {
          result = ldapOp.call();
        }
        catch (LdapException le)
        {
            throw le;
        }
        catch (Exception e) {
            throw new LdapException( LdapErrors.LDAP_OTHER.getCode(), e.getMessage() );
        }
        finally
        {
            if (perfLog.isTraceEnabled())
            {
                perfLog.trace(String.format(
                    "ldap operation took [%d]ms", TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedAt)));
            }
        }

        return result;
    }

    private LdapPagedSearchResultPtr search_one_page_internal(
        final String base,
        final LdapScope scope,
        final String filter,
        final String[] attributes, // null-terminated attributes array
        final int pageSize,
        final Pointer berCookiePtr
        )
    {
        this.validate();

        final ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        return execute(
                    new Callable<LdapPagedSearchResultPtr>() {
                        @Override
                        public LdapPagedSearchResultPtr call() {
                            return ldapClientLibrary.ldap_one_paged_search(
                                    LdapConnection.this.getRawConnection(),
                                    base,
                                    scope.getCode(),
                                    filter,
                                    attributes,
                                    pageSize,
                                    berCookiePtr);
                        }
                    });
    }

    private void deleteObjectTreeInternal(final ILdapClientLibrary library, final String dn)
    {
        if (perfLog.isTraceEnabled()) {
            perfLog.trace(String.format("deleteObjectTreeInternal dn=[%s]", dn));
        }
        final String attributes[] = { "objectclass", null };

        if (perfLog.isTraceEnabled()) {
            perfLog.trace(String.format(
                "search context: base=[%s], scope: [%s], filter: [%s], attributes: [%s], attributesOnly: [%s]",
                dn, LdapScope.SCOPE_ONE_LEVEL, "(objectclass=*)", Arrays.toString(attributes), Boolean.toString(false)));
        }

        Pointer p = execute(
                new Callable<Pointer>() {
                    @Override
                    public Pointer call() {
                        return library.ldap_search_s(
                                LdapConnection.this.getRawConnection(),
                                dn,
                                LdapScope.SCOPE_ONE_LEVEL.getCode(),
                                "(objectclass=*)",
                                attributes,
                                0);
                    }
                });
        ILdapMessage message = new LdapMessage(this, p, null);

        try
        {
            ILdapEntry[] entries = message.getEntries();

            if (entries != null && entries.length > 0)
            {
                for (ILdapEntry entry : entries)
                {
                    // delete child object
                    deleteObjectTreeInternal(library, entry.getDN());
                }
            }

            if (perfLog.isTraceEnabled()) {
                perfLog.trace(String.format("delete context: dn=[%s]",dn));
            }
            execute(
                    new Callable<Void>() {
                        @Override
                        public Void call() {
                            library.ldap_delete_s(
                                    LdapConnection.this.getRawConnection(),
                                    dn); // delete current object
                            return null;
                        }
                    });
        }
        finally
        {
            message.close();
        }
    }

    /**
       Exposes the underlying native pointer. Caller MUST never close (unbind) over this pointer.
       This pointer is owned by the current instance of the connection.
    */
    Pointer getRawConnection()
    {
        this.validate();
        return _connection;
    }

    boolean isValid()
    {
        return ( ( this._connection != null ) && (this._connection != Pointer.NULL) );
    }

    private void validate()
    {
        if ( this.isValid() == false )
        {
            throw new IllegalStateException("Attempting to use a closed connection!");
        }
    }

    private static void disposeLdapModNative(LdapModNative[] nativeAttrs)
    {
        if ( nativeAttrs != null )
        {
            for(int i = 0; i < nativeAttrs.length; i++)
            {
                if (nativeAttrs[i] != null)
                {
                    nativeAttrs[i].close();
                    nativeAttrs[i] = null;
                }
            }
        }
    }

    ILdapClientLibrary getLdapLibrary() {
        return this._ldapClientLibrary;
    }
}


