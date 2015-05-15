Name:    vmware-afd
Summary: Authentication Framework Service
Version: 6.0.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open >= 6.2.0, vmware-directory-client >= 6.0, vmware-afd-client = %{version}
BuildRequires:  coreutils >= 8.22, openssl-devel >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open-devel >= 6.2.0, vmware-directory-client-devel >= 2.0, sqlite-autoconf, python2-devel >= 2.7.8

%define _dbdir %_localstatedir/lib/vmware/vmafd
%define _vecsdir %{_dbdir}/vecs
%define _crlsdir %{_dbdir}/crl

%define _likewise_open_bindir %{_likewise_open_prefix}/bin

%description
VMware Authentication Framework

%package client
Summary: VMware Authentication Framework Client
Requires:  coreutils >= 8.22, openssl >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open >= 6.2.0, vmware-directory-client >= 6.0
%description client
Client libraries to communicate with VMware Authentication Framework Service

%package client-devel
Summary: VMware Authentication Framework Client Development Library
Requires: vmware-afd-client = %{version}
%description client-devel
Development Libraries to communicate with VMware Authentication Framework Service

%build

export CFLAGS="-Wno-pointer-sign -Wno-unused-but-set-variable -Wno-implicit-function-declaration -Wno-address"
cd build
autoreconf -mif .. &&
../configure --prefix=%{_prefix} \
            --libdir=%{_lib64dir} \
            --localstatedir=/var/lib/vmware/vmafd \
            --with-vmdir=%{_prefix} \
            --with-likewise=%{_likewise_open_prefix} \
            --with-ssl=/usr \
            --with-sqlite=/usr \
            --with-python=/usr

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build && make install DESTDIR=%{buildroot}

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    if [ -z "`pidof lwsmd`" ]; then
        /bin/systemctl start lwsmd
    fi

%post

    /sbin/ldconfig

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /bin/mkdir -m 700 -p %{_dbdir}
    /bin/mkdir -m 700 -p %{_vecsdir}
    /bin/mkdir -m 700 -p %{_crlsdir}

    case "$1" in
        1)
            %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmafd.reg
            %{_likewise_open_bindir}/lwsm -q refresh
            ;;
        2)
            %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmafd.reg
            %{_likewise_open_bindir}/lwsm -q refresh
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

%files client
%defattr(-,root,root)
%{_bindir}/dir-cli
%{_bindir}/vdcpromo
%{_bindir}/vecs-cli
%{_bindir}/vmafd-cli
%{_lib64dir}/libvmafcfgapi.so*
%{_lib64dir}/libvmafdclient.so*
%{_lib64dir}/libvmeventclient.so*
%{_lib64dir}/libvmauthsvcclient.so*

%files client-devel
%defattr(-,root,root)
%{_includedir}/vecsclient.h
%{_includedir}/vmafd.h
%{_includedir}/vmafdclient.h
%{_includedir}/vmafdtypes.h
%{_lib64dir}/libvmafdclient.a
%{_lib64dir}/libvmafdclient.la
%{_lib64dir}/libvmafcfgapi.a
%{_lib64dir}/libvmafcfgapi.la
%{_lib64dir}/libvmeventclient.a
%{_lib64dir}/libvmeventclient.la
%{_lib64dir}/libvmauthsvcclient.a
%{_lib64dir}/libvmauthsvcclient.la

%exclude %{_sysconfdir}/vmware/java/vmware-override-java.security
%exclude %{_sysconfdir}/vmware/vm-support/vmafd.mfx
%exclude %{_prefix}/etc/init.d/vmafdd
%exclude %{_lib64dir}/libvecsdb.a
%exclude %{_lib64dir}/libvecsdb.la
%exclude %{_datadir}/config/java.security.linux
%exclude %{_lib64dir}/vmware-vmafd/*

%clean

rm -rf $RPM_BUILD_ROOT

# %doc ChangeLog README COPYING

%changelog

