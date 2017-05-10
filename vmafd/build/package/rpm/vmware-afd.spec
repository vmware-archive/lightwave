Name:    vmware-afd
Summary: Authentication Framework Service
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open >= 6.2.10, vmware-directory-client = %{_version}, vmware-afd-client = %{version}, vmware-dns-client = %{version}
BuildRequires:  coreutils >= 8.22, openssl-devel >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open-devel >= 6.2.10, vmware-directory-client-devel = %{version}, sqlite-autoconf, python2-devel >= 2.7.8, openjdk >= 1.8.0.112, apache-ant >= 1.9.4, ant-contrib >= 1.0b3, vmware-dns-client-devel = %{version}, apache-maven >= 3.3.9, boost = 1.60.0

%define _dbdir %_localstatedir/lib/vmware/vmafd
%define _vecsdir %{_dbdir}/vecs
%define _crlsdir %{_dbdir}/crl
%define _jarsdir  %{_prefix}/jars
%define _logdir /var/log/lightwave
%define _logconfdir /etc/syslog-ng/lightwave.conf.d
%define _pymodulesdir /opt/vmware/site-packages/identity

%if 0%{?_javahome:1} == 0
%define _javahome %{_javahome}
%endif

%define _jreextdir %{_javahome}/jre/lib/ext

%if 0%{?_likewise_open_prefix:1} == 0
%define _likewise_open_prefix /opt/likewise
%endif

%define _likewise_open_bindir %{_likewise_open_prefix}/bin
%define _likewise_open_sbindir %{_likewise_open_prefix}/sbin

%if 0%{?_vmdir_prefix:1} == 0
%define _vmdir_prefix /opt/vmware
%endif

%if 0%{?_vmdns_prefix:1} == 0
%define _vmdns_prefix /opt/vmware
%endif

%description
VMware Authentication Framework

%debug_package

%package client
Summary: VMware Authentication Framework Client
Requires:  coreutils >= 8.22, openssl >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open >= 6.2.10, vmware-directory-client >= %{version}
%description client
Client libraries to communicate with VMware Authentication Framework Service

%package client-devel
Summary: VMware Authentication Framework Client Development Library
Requires: vmware-afd-client = %{version}
%description client-devel
Development Libraries to communicate with VMware Authentication Framework Service

%package client-python
Summary: VMware Authentication Framework Python Files
Requires: vmware-afd-client, boost = 1.60.0
%description client-python
Python files included in vmafd

%build

export CFLAGS="-Wall -Werror -Wno-pointer-sign -Wno-unused-but-set-variable -Wno-address"
cd build
autoreconf -mif .. &&
../configure --prefix=%{_prefix} \
            --libdir=%{_lib64dir} \
            --localstatedir=/var/lib/vmware/vmafd \
            --with-vmdir=%{_vmdir_prefix} \
            --with-vmdns=%{_vmdns_prefix} \
            --with-likewise=%{_likewise_open_prefix} \
            --with-ssl=/usr \
            --with-sqlite=/usr \
            --with-python=/usr \
            --with-jdk=%{_javahome} \
            --with-ant=%{_anthome} \
            --with-maven=%{_mavendir} \
            --with-boost=/usr \
            --enable-krb5-default=yes

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build && make install DESTDIR=%{buildroot}

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        if [ -z "`pidof lwsmd`" ]; then
            /bin/systemctl start lwsmd
        fi
    fi

%post

    /sbin/ldconfig

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /bin/mkdir -m 700 -p %{_dbdir}
    /bin/mkdir -m 700 -p %{_vecsdir}
    /bin/mkdir -m 700 -p %{_crlsdir}

    /bin/mkdir -m 755 -p %{_logdir}
    /bin/mkdir -m 755 -p %{_logconfdir}
    if [ -a %{_logconfdir}/vmafdd-syslog-ng.conf ]; then
        /bin/rm %{_logconfdir}/vmafdd-syslog-ng.conf
    fi
    /bin/ln -s %{_datadir}/config/vmafdd-syslog-ng.conf %{_logconfdir}/vmafdd-syslog-ng.conf

    case "$1" in
        1)
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmafd.reg
                %{_likewise_open_bindir}/lwsm -q refresh
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmafd.reg
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi
            ;;
        2)
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmafd.reg
                %{_likewise_open_bindir}/lwsm -q refresh
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmafd.reg
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi
            ;;
    esac

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            %{_likewise_open_bindir}/lwsm info vmafd > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                echo "Stopping the AFD Service..."
                %{_likewise_open_bindir}/lwsm stop vmafd
                echo "Removing service configuration..."
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmafd'
                echo "Restarting service control manager..."
                /bin/systemctl restart lwsmd
                sleep 2
                echo "Autostart services..."
                %{_likewise_open_bindir}/lwsm autostart
            fi
            ;;
    esac

%postun

    /sbin/ldconfig

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            echo "Existing VECS files kept under [%{_dbdir}]"
            ;;
    esac

%files
%defattr(-,root,root)
%{_sbindir}/*
%{_datadir}/config/vmafd.reg
%{_datadir}/config/vmafdd-syslog-ng.conf

%files client
%defattr(-,root,root)
%{_bindir}/cdc-cli
%{_bindir}/dir-cli
%{_bindir}/domainjoin
%{_bindir}/vdcpromo
%{_bindir}/vecs-cli
%{_bindir}/vmafd-cli
%{_bindir}/sl-cli
%{_bindir}/lw-support-bundle.sh
%{_sysconfdir}/vmware/java/vmware-override-java.security
%{_datadir}/config/java.security.linux
%{_lib64dir}/libvecsjni.so*
%{_lib64dir}/libcdcjni.so*
%{_lib64dir}/libheartbeatjni.so*
%{_jreextdir}/vmware-endpoint-certificate-store.jar
%{_jreextdir}/client-domain-controller-cache.jar
%{_jreextdir}/afd-heartbeat-service.jar
%{_jarsdir}/*.jar
%{_lib64dir}/libvmafcfgapi.so*
%{_lib64dir}/libvmafdclient.so*
%{_lib64dir}/libvmeventclient.so*

%files client-python
%defattr(-,root,root)
%{_pymodulesdir}/vmafd.*
%{_pymodulesdir}/*.py

%files client-devel
%defattr(-,root,root)
%{_includedir}/vmafd.h
%{_includedir}/vmafdtypes.h
%{_includedir}/vmafdclient.h
%{_includedir}/vecsclient.h
%{_includedir}/cdcclient.h
%{_includedir}/vmsuperlogging.h
%{_lib64dir}/libcdcjni.a
%{_lib64dir}/libcdcjni.la
%{_lib64dir}/libvecsjni.a
%{_lib64dir}/libvecsjni.la
%{_lib64dir}/libheartbeatjni.a
%{_lib64dir}/libheartbeatjni.la
%{_lib64dir}/libvmafdclient.a
%{_lib64dir}/libvmafdclient.la
%{_lib64dir}/libvmafcfgapi.a
%{_lib64dir}/libvmafcfgapi.la
%{_lib64dir}/libvmeventclient.a
%{_lib64dir}/libvmeventclient.la

%exclude %{_lib64dir}/libvecsdb.a
%exclude %{_lib64dir}/libvecsdb.la

%clean

rm -rf $RPM_BUILD_ROOT

# %doc ChangeLog README COPYING

%changelog

