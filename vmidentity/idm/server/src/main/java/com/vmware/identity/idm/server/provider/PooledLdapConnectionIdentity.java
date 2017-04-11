package com.vmware.identity.idm.server.provider;

import java.security.cert.X509Certificate;
import java.util.Collection;

import com.vmware.identity.idm.AuthenticationType;

public class PooledLdapConnectionIdentity {

    private final String tenantName;
    private final String username;
    private final String password;
    private final AuthenticationType authType;
    private final boolean useGCPort;
    private final String connectionString;
    private final int priority;
    private final Collection<X509Certificate> idsTrustedCertificates;
    private final Collection<X509Certificate> tenantTrustedCertificates;

    private PooledLdapConnectionIdentity(String tenantName, String connectionString,
            String username, String password, AuthenticationType authType, boolean useGCPort, int priority,
            Collection<X509Certificate> idsTrustedCertificates, Collection<X509Certificate> tenantTrustedCertificates)
    {
        this.tenantName = tenantName;
        this.username = username;
        this.password = password;
        this.authType = authType;
        this.useGCPort = useGCPort;
        this.connectionString = connectionString;
        this.priority = priority;
        this.idsTrustedCertificates = idsTrustedCertificates;
        this.tenantTrustedCertificates = tenantTrustedCertificates;
    }

    public String getUsername() {
        return username;
    }

    public String getConnectionString() {
        return connectionString;
    }

    public AuthenticationType getAuthType() {
        return authType;
    }

    public String getPassword() {
        return password;
    }

    public boolean isUseGCPort() {
        return useGCPort;
    }

    public String getTenantName() {
        return tenantName;
    }

    public int getPriority() {
        return priority;
    }

    public Collection<X509Certificate> getIdsTrustedCertificates() {
        return idsTrustedCertificates;
    }

    public Collection<X509Certificate> getTenantTrustedCertificates() {
        return tenantTrustedCertificates;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((authType == null) ? 0 : authType.hashCode());
        result = prime * result + ((connectionString == null) ? 0 : connectionString.hashCode());
        result = prime * result + ((idsTrustedCertificates == null) ? 0 : idsTrustedCertificates.hashCode());
        result = prime * result + ((password == null) ? 0 : password.hashCode());
        result = prime * result + priority;
        result = prime * result + ((tenantName == null) ? 0 : tenantName.hashCode());
        result = prime * result + ((tenantTrustedCertificates == null) ? 0 : tenantTrustedCertificates.hashCode());
        result = prime * result + (useGCPort ? 1231 : 1237);
        result = prime * result + ((username == null) ? 0 : username.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        PooledLdapConnectionIdentity other = (PooledLdapConnectionIdentity) obj;
        if (authType != other.authType)
            return false;
        if (connectionString == null) {
            if (other.connectionString != null)
                return false;
        } else if (!connectionString.equals(other.connectionString))
            return false;
        if (idsTrustedCertificates == null) {
            if (other.idsTrustedCertificates != null)
                return false;
        } else if (!idsTrustedCertificates.equals(other.idsTrustedCertificates))
            return false;
        if (password == null) {
            if (other.password != null)
                return false;
        } else if (!password.equals(other.password))
            return false;
        if (priority != other.priority)
            return false;
        if (tenantName == null) {
            if (other.tenantName != null)
                return false;
        } else if (!tenantName.equals(other.tenantName))
            return false;
        if (tenantTrustedCertificates == null) {
            if (other.tenantTrustedCertificates != null)
                return false;
        } else if (!tenantTrustedCertificates.equals(other.tenantTrustedCertificates))
            return false;
        if (useGCPort != other.useGCPort)
            return false;
        if (username == null) {
            if (other.username != null)
                return false;
        } else if (!username.equals(other.username))
            return false;
        return true;
    }

    @Override
    public String toString() {
        return "PooledLdapConnectionIdentity [tenantName=" + tenantName + ", username=" + username + ", authType="
                + authType + ", useGCPort=" + useGCPort + ", connectionString=" + connectionString + "]";
    }

    public static class Builder {
        private static final int DEFAULT_PRIORITY = 0;
        private String tenantName;
        private String username;
        private String password;
        private AuthenticationType authType;
        private boolean useGCPort;
        private String connectionString;
        private int priority;
        private Collection<X509Certificate> idsTrustedCertificates;
        private Collection<X509Certificate> tenantTrustedCertificates;

        public Builder(String connectionString, AuthenticationType authType) {
            this.connectionString = connectionString;
            this.authType = authType;
            this.priority = DEFAULT_PRIORITY;
        }

        public PooledLdapConnectionIdentity build() {
            return new PooledLdapConnectionIdentity(tenantName, connectionString, username, password,
                    authType, useGCPort, priority, idsTrustedCertificates, tenantTrustedCertificates);
        }

        public Builder setTenantName(String tenantName) {
            this.tenantName = tenantName;
            return this;
        }


        public Builder setUsername(String username) {
            this.username = username;
            return this;
        }

        public Builder setPassword(String password) {
            this.password = password;
            return this;
        }

        public Builder setUseGCPort(boolean useGCPort) {
            this.useGCPort = useGCPort;
            return this;
        }

        public Builder setPriority(int priority) {
            this.priority = priority;
            return this;
        }

        public Builder setIdsTrustedCertificates(Collection<X509Certificate> idsTrustedCertificates) {
            this.idsTrustedCertificates = idsTrustedCertificates;
            return this;
        }

        public Builder setTenantTrustedCertificates(Collection<X509Certificate> tenantTrustedCertificates) {
            this.tenantTrustedCertificates = tenantTrustedCertificates;
            return this;
        }
    }
}
