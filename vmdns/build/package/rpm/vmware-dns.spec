Name:    vmware-dns
Summary: DNS Service
Version: 6.0.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open >= 6.2.0 
BuildRequires:  coreutils >= 8.22, openssl-devel >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open-devel >= 6.2.0

%if 0%{?_sasl_prefix:1} == 0
%define _sasl_prefix /usr
%endif

%if 0%{?_krb5_prefix:1} == 0
%define _krb5_prefix /usr
%endif

%if 0%{?_likewise_open_prefix:1} == 0
%define _likewise_open_prefix /opt/likewise
%endif

%define _likewise_open_bindir %{_likewise_open_prefix}/bin

%define _krb5_lib_dir %{_krb5_prefix}/lib64
%define _krb5_gss_conf_dir /etc/gss

%description
VMware DNS Service

%package client
Summary: VMware DNS Client
Requires:  coreutils >= 8.22, openssl >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open >= 6.2.0
%description client
Client libraries to communicate with DNS Service

%package client-devel
Summary: VMware DNS Client Development Library
Requires: vmware-directory-client = %{version}
%description client-devel
Development Libraries to communicate with DNS Service

%build
export CFLAGS="-Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare"
cd build
autoreconf -mif ..
../configure \
    --prefix=%{_prefix} \
    --libdir=%{_lib64dir} \
    --localstatedir=%{_localstatedir}/lib/vmware/vmdir \
    --with-vmdir=%{_prefix} \
    --with-likewise=%{_likewise_open_prefix} \
    --with-ssl=/usr 
make

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build && make install DESTDIR=$RPM_BUILD_ROOT

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    if [ -z "`pidof lwsmd`" ]; then
        /bin/systemctl start lwsmd
    fi

%pre client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    if [ -z "`pidof lwsmd`" ]; then
        /bin/systemctl start lwsmd
    fi

%post

    /sbin/ldconfig

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns.reg
            %{_likewise_open_bindir}/lwsm -q refresh
            sleep 2
            ;;         
        2)
            %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdns.reg
            %{_likewise_open_bindir}/lwsm -q refresh
            sleep 2
            ;;
    esac

%post client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns.reg
            ;;         
        2)
            %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdns.reg
            ;;
    esac

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            ;;
    esac

%preun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            ;;
    esac

%postun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig




%postun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade
    case "$1" in
        0)
            ;;
    esac

%files
%defattr(-,root,root)
%{_sbindir}/*
%{_datadir}/config/vmdns.reg

%files client
%defattr(-,root,root)
%{_bindir}/vmdns-cli
%{_bindir}/dnstest
%{_lib64dir}/libvmdnsclient.*
%{_lib64dir}/libvmsock.*

%files client-devel
%defattr(-,root,root,0755)
%{_includedir}/vmdns.h
%{_includedir}/vmdnstypes.h
%{_lib64dir}/libvmdnsclient.*
%{_lib64dir}/libvmsock.*

%exclude %{_bindir}/vmdns-cli
%exclude %{_bindir}/dnstest
%exclude %{_prefix}/etc/init.d/vmdnsd
%exclude /usr/lib/vmware-vmdns/firewall/vmdns-firewall.json
%exclude /usr/lib/vmware-vmdns/firstboot/vmdns-firstboot.py

%changelog
