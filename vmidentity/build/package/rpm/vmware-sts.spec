Name:    vmware-sts
Summary: VMware Secure Token Service
Version: 6.5.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, likewise-open >= 6.2.9, vmware-directory >= 6.5.0, vmware-afd >= 6.5.0, vmware-ca >= 6.5.0, openjre >= 1.8.0.92, commons-daemon >= 1.0.15, apache-tomcat >= 8.0.35, %{name}-client >= %{version}
BuildRequires: coreutils >= 8.22, openssl-devel >= 1.0.2, likewise-open-devel >= 6.2.9, vmware-directory-client-devel >= 6.5.0, vmware-afd-client-devel >= 6.5.0, vmware-ca-client-devel >= 6.5.0, openjdk >= 1.8.0.92, apache-ant >= 1.9.4

%define _dbdir %_localstatedir/lib/vmware/vmsts
%define _jarsdir %_prefix/jars
%define _binsdir %_prefix/bin
%define _webappsdir %_prefix/vmware-sts/webapps

%if 0%{?_javahome:1} == 0
%define _javahome %_javahome
%endif

%description
VMware Secure Token Server

%package client
Summary: VMware Secure Token Service Client
Requires:  coreutils >= 8.22, openssl >= 1.0.2, openjre >= 1.8.0.45, vmware-directory-client >= 6.5.0, likewise-open >= 6.2.9 
%description client
Client libraries to communicate with VMware Secure Token Service

%build

cd build
autoreconf -mif .. &&
../configure --prefix=%{_prefix} \
             --libdir=%{_lib64dir} \
             --localstatedir=%{_dbdir} \
             --with-afd=%{_prefix} \
             --with-likewise=%{_likewise_open_prefix} \
             --with-ssl=/usr \
             --with-java=%{_javahome} \
             --with-commons-daemon=%{_commons_daemondir} \
             --with-ant=%{_antdir} \
             --with-tomcat=%{_tomcatdir} \
             --with-jax-ws=%{_jaxwsdir} \
             --with-maven=%{_mavendir}
make

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build && make install DESTDIR=%{buildroot}

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

%post

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /sbin/ldconfig

    /bin/mkdir -m 700 -p %{_dbdir}

case "$1" in
    1)

        /bin/systemctl enable vmware-idmd.service >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            /bin/ln -s /lib/systemd/system/vmware-idmd.service /etc/systemd/system/multi-user.target.wants/vmware-idmd.service
        fi

        /bin/systemctl enable vmware-stsd.service >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            /bin/ln -s /lib/systemd/system/vmware-stsd.service /etc/systemd/system/multi-user.target.wants/vmware-stsd.service
        fi
        /bin/systemctl >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            /bin/systemctl daemon-reload
        fi
        ;;

    2)
        ;;
esac

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

if [ "$1" = 0 ]; then
    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then

         if [ -f /etc/systemd/system/vmware-idmd.service ]; then
             /bin/systemctl stop vmware-idmd.service
             /bin/systemctl disable vmware-idmd.service
             /bin/rm -f /etc/systemd/system/vmware-idmd.service
             /bin/systemctl daemon-reload
         fi

         if [ -f /etc/systemd/system/vmware-stsd.service ]; then
             /bin/systemctl stop vmware-stsd.service
             /bin/systemctl disable vmware-stsd.service
             /bin/rm -f /etc/systemd/system/vmware-stsd.service
             /bin/systemctl daemon-reload
         fi
    fi
fi

%postun

    /sbin/ldconfig

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            /bin/rm -rf %{_dbdir}
            ;;
    esac

%files
%defattr(-,root,root,0755)
/lib/systemd/system/vmware-idmd.service
/lib/systemd/system/vmware-stsd.service
%{_sbindir}/vmware-idmd.sh
%{_sbindir}/vmware-stsd.sh
%{_lib64dir}/*.so*
%{_binsdir}/test-ldapbind
%{_binsdir}/test-logon
%{_binsdir}/test-svr
%{_jarsdir}/openidconnect-client-lib.jar
%{_jarsdir}/openidconnect-common.jar
%{_jarsdir}/openidconnect-protocol.jar
%{_jarsdir}/samlauthority.jar
%{_jarsdir}/vmware-identity-diagnostics.jar
%{_jarsdir}/vmware-identity-idm-server.jar
%{_jarsdir}/vmware-identity-rest-afd-server.jar
%{_jarsdir}/vmware-identity-rest-core-server.jar
%{_jarsdir}/vmware-identity-rest-idm-server.jar
%{_jarsdir}/vmware-identity-install.jar
%{_webappsdir}/idm.war
%{_webappsdir}/afd.war
%{_webappsdir}/openidconnect.war
%{_webappsdir}/sts.war
%{_webappsdir}/websso.war
%{_webappsdir}/ROOT.war
%{_webappsdir}/vmdir.war
%{_datadir}/config/idm/*
%config %attr(600, root, root) %{_prefix}/vmware-sts/bin/setenv.sh
%config %attr(600, root, root) %{_prefix}/vmware-sts/bin/vmware-identity-tomcat-extensions.jar

%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/catalina.policy
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/catalina.properties
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/context.xml
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/logging.properties
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/server.xml
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/web.xml
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/tomcat-users.xml

%exclude %{_lib64dir}/*.la
%exclude %{_lib64dir}/*.a

%files client
%defattr(-,root,root)
%{_jarsdir}/samltoken.jar
%{_jarsdir}/vmware-identity-rest-idm-common.jar
%{_jarsdir}/vmware-identity-rest-core-common.jar
%{_jarsdir}/vmware-identity-websso-client.jar
%{_jarsdir}/vmware-identity-platform.jar
%{_jarsdir}/vmware-identity-wsTrustClient.jar
%{_jarsdir}/vmware-identity-rest-afd-common.jar
%{_jarsdir}/openidconnect-common.jar
%{_jarsdir}/vmware-identity-depends.jar
%{_jarsdir}/openidconnect-client-lib.jar
%{_jarsdir}/vmware-identity-idm-client.jar
%{_jarsdir}/vmware-identity-idm-interface.jar
%{_jarsdir}/vmware-identity-rest-afd-client.jar
%{_jarsdir}/vmware-identity-rest-core-client.jar
%{_jarsdir}/vmware-identity-rest-idm-client.jar
%{_webappsdir}/openidconnect-sample-rp.war
# %doc ChangeLog README COPYING

%changelog

