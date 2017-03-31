package com.vmware.identity.idm.server.provider;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.pool2.KeyedPooledObjectFactory;
import org.apache.commons.pool2.PooledObject;
import org.apache.commons.pool2.impl.DefaultPooledObject;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.server.LdapCertificateValidationSettings;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.LdapScope;

class PooledLdapConnectionFactory implements KeyedPooledObjectFactory<PooledLdapConnectionIdentity, ILdapConnectionEx> {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(PooledLdapConnectionFactory.class);

    @Override
    public void activateObject(PooledLdapConnectionIdentity identity, PooledObject<ILdapConnectionEx> connection)
            throws Exception
    {
        // DO nothing
    }

    @Override
    public void destroyObject(PooledLdapConnectionIdentity identity, PooledObject<ILdapConnectionEx> connection)
            throws Exception
    {
        connection.getObject().close();
    }

    @Override
    public PooledObject<ILdapConnectionEx> makeObject(PooledLdapConnectionIdentity identity)
            throws Exception
    {
        Collection<String> connectionStrings = new ArrayList<>();
        connectionStrings.add(identity.getConnectionString());
        ILdapConnectionEx connection = ServerUtils.getLdapConnectionByURIs(
                ServerUtils.toURIObjects(connectionStrings),
                identity.getUsername(),
                identity.getPassword(),
                identity.getAuthType(),
                identity.isUseGCPort(),
                new LdapCertificateValidationSettings(identity.getIdsTrustedCertificates(),
                        identity.getTenantTrustedCertificates()));

        logger.info(String.format("New connection created in pool %s", identity.toString()));
        return new DefaultPooledObject<>(connection);
    }

    @Override
    public void passivateObject(PooledLdapConnectionIdentity identity, PooledObject<ILdapConnectionEx> connection)
            throws Exception
    {
        // DO nothing
    }

    @Override
    public boolean validateObject(PooledLdapConnectionIdentity identity, PooledObject<ILdapConnectionEx> connection) {
        ILdapConnectionEx ldapConnection = connection.getObject();
        if (ldapConnection == null) {
            return false;
        }

        try {
            String base = ServerUtils.getDomainDN(identity.getTenantName());
            String[] attributes = new String[0];
            ldapConnection.search(base, LdapScope.SCOPE_BASE, "objectclass=*", attributes, false);
            return true;
        } catch (Exception e) {
            logger.error(e);
            return false;
        }
    }
}
