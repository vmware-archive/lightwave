Name:    vmware-sts
Summary: VMware Secure Token Service
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, likewise-open >= 6.2.10, vmware-directory = %{version}, vmware-afd = %{version}, vmware-ca = %{version}, openjre >= 1.8.0.112, commons-daemon >= 1.0.15, apache-tomcat >= 8.5.8, %{name}-client = %{version}
BuildRequires: coreutils >= 8.22, openssl-devel >= 1.0.2, likewise-open-devel >= 6.2.10, vmware-directory-client-devel = %{version}, vmware-afd-client-devel = %{version}, vmware-ca-client-devel = %{version}, openjdk >= 1.8.0.112, apache-ant >= 1.9.4

%define _dbdir %_localstatedir/lib/vmware/vmsts
%define _jarsdir %_prefix/jars
%define _binsdir %_prefix/bin
%define _webappsdir %_prefix/vmware-sts/webapps
%define _backupdir /tmp/sso
%define _lwisbindir %_likewise_open_prefix/bin

%if 0%{?_javahome:1} == 0
%define _javahome %_javahome
%endif

%description
VMware Secure Token Server

%debug_package

%package client
Summary: VMware Secure Token Service Client
Requires:  coreutils >= 8.22, openssl >= 1.0.2, openjre >= 1.8.0.45, vmware-directory-client >= %{version}, likewise-open >= 6.2.10
%description client
Client libraries to communicate with VMware Secure Token Service

%package samples
Summary: VMware Secure Token Service Samples
Requires:  vmware-sts-client >= %{version}
%description samples
Samples for VMware Secure Token Service

%build

cd build
autoreconf -mif .. &&
../configure --prefix=%{_prefix} \
             --libdir=%{_lib64dir} \
             --localstatedir=%{_dbdir} \
             --with-afd=%{_prefix} \
             --with-likewise=%{_likewise_open_prefix} \
             --with-jansson=%{_janssondir} \
             --with-curl=%{_curldir} \
             --with-ssl=%{_ssldir} \
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
if [[ $1 -gt 1 ]]
then
    if [ ! -d %{_backupdir} ];
    then
        /bin/mkdir "%{_backupdir}"
    fi
    /bin/cp "%{_prefix}/vmware-sts/conf/server.xml" "%{_backupdir}/server.xml"
fi

%post

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade
    /sbin/ldconfig

    /bin/mkdir -m 700 -p %{_dbdir}

case "$1" in
    1)

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
        %{_sbindir}/configure-build.sh "%{_backupdir}"
        ;;
esac

if [ -x "%{_lwisbindir}/lwregshell" ]
then
    %{_lwisbindir}/lwregshell list_keys "[HKEY_THIS_MACHINE\Software\VMware\Identity]" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        # add key if not exist
        %{_lwisbindir}/lwregshell add_key "[HKEY_THIS_MACHINE\Software]"
        %{_lwisbindir}/lwregshell add_key "[HKEY_THIS_MACHINE\Software\VMware]"
        %{_lwisbindir}/lwregshell add_key "[HKEY_THIS_MACHINE\Software\VMware\Identity]"
    fi

    %{_lwisbindir}/lwregshell list_values "[HKEY_THIS_MACHINE\Software\VMware\Identity]" | grep "Release" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        # add value if not exist
        %{_lwisbindir}/lwregshell add_value "[HKEY_THIS_MACHINE\Software\VMware\Identity]" "Release" REG_SZ "Lightwave"
    fi

    %{_lwisbindir}/lwregshell list_values "[HKEY_THIS_MACHINE\Software\VMware\Identity]" | grep "Version" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        # add value if not exist
        %{_lwisbindir}/lwregshell add_value "[HKEY_THIS_MACHINE\Software\VMware\Identity]" "Version" REG_SZ "%{_version}"
    else
        # set value if exists
        %{_lwisbindir}/lwregshell set_value "[HKEY_THIS_MACHINE\Software\VMware\Identity]" "Version" "%{_version}"
    fi
fi

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

if [ "$1" = 0 ]; then
    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then

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

            if [ -x "%{_lwisbindir}/lwregshell" ]
            then
                %{_lwisbindir}/lwregshell list_keys "[HKEY_THIS_MACHINE\Software\VMware\Identity]" > /dev/null 2>&1
                if [ $? -eq 0 ]; then
                    # delete key if exist
                    %{_lwisbindir}/lwregshell delete_tree "[HKEY_THIS_MACHINE\Software\VMware\Identity]"
                fi
            fi

            ;;
    esac

%files
%defattr(-,root,root,0755)
/lib/systemd/system/vmware-stsd.service
%{_sbindir}/vmware-stsd.sh
%{_sbindir}/configure-build.sh
%{_sbindir}/sso-config.sh
%{_includedir}/*.h
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
%{_jarsdir}/vmware-directory-rest-server.jar
%{_jarsdir}/vmware-identity-install.jar
%{_jarsdir}/vmware-identity-sso-config.jar
%{_jarsdir}/websso.jar
%{_jarsdir}/sts.jar
%{_jarsdir}/openidconnect-server.jar
%{_webappsdir}/lightwaveui.war
%{_webappsdir}/ROOT.war
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
%{_jarsdir}/vmware-directory-rest-common.jar
%{_jarsdir}/vmware-directory-rest-client.jar
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
%{_jarsdir}/vmware-directory-rest-client.jar
%{_includedir}/*.h
%{_lib64dir}/*.so*

%exclude %{_bindir}/*test

# %doc ChangeLog README COPYING

%files samples
%{_webappsdir}/openidconnect-sample-rp.war
%{_jarsdir}/vmware-identity-rest-idm-samples.jar


%changelog

