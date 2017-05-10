Name:    vmware-ic-config
Summary: VMware Infrastructure Controller Configuration Tool
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, likewise-open >= 6.2.10, vmware-directory-client = %{version}, vmware-afd-client = %{version}, vmware-ca-client = %{version}, gawk >= 4.1.3
BuildRequires: coreutils >= 8.22, openssl-devel >= 1.0.2, likewise-open-devel >= 6.2.10, vmware-directory-client-devel = %{version}, vmware-afd-client-devel = %{version}, vmware-ca-client-devel = %{version}

%define _jarsdir %{_prefix}/jars
%define _bindir %{_prefix}/bin
%define _configdir %{_prefix}/share/config
%define _serviceddir /lib/systemd/system
%if 0%{?_likewise_open_prefix:1} == 0
%define _likewise_open_prefix /opt/likewise
%endif

%if 0%{?_javahome:1} == 0
%define _javahome %{_javahome}
%endif

%if 0%{?_vmdir_prefix:1} == 0
%define _vmdir_prefix /opt/vmware
%endif

%if 0%{?_vmafd_prefix:1} == 0
%define _vmafd_prefix /opt/vmware
%endif

%if 0%{?_vmca_prefix:1} == 0
%define _vmca_prefix /opt/vmware
%endif

%if 0%{?_vmdns_prefix:1} == 0
%define _vmdns_prefix /opt/vmware
%endif

%if 0%{?_vmsts_prefix:1} == 0
%define _vmsts_prefix /opt/vmware
%endif

%description
VMware Infrastructure Controller Configuration Tool

%debug_package

%build

cd build
autoreconf -mif .. &&
../configure --prefix=%{_prefix} \
             --libdir=%{_lib64dir} \
             --with-java=%{_javahome} \
             --with-ant=%{_anthome} \
             --with-likewise=%{_likewise_open_prefix} \
             --with-vmdir=%{_vmdir_prefix} \
             --with-vmca=%{_vmca_prefix} \
             --with-vmdns=%{_vmdns_prefix} \
             --with-afd=%{_vmafd_prefix} \
             --with-sts=%{_vmsts_prefix} \
             --with-ssl=/usr
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

    /bin/systemctl enable firewall.service >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        /bin/ln -s %{_serviceddir}/firewall.service /etc/systemd/system/multi-user.target.wants/firewall.service
    fi

    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        /bin/systemctl daemon-reload
    fi
    /bin/systemctl start firewall.service

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade
    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then

         if [ -f /etc/systemd/system/firewall.service ]; then
             /bin/systemctl stop firewall.service
             /bin/systemctl disable firewall.service
             /bin/rm -f /etc/systemd/system/firewall.service
             /bin/systemctl daemon-reload
         fi

    fi

%postun

    /sbin/ldconfig

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%files
%defattr(-,root,root,0755)
%{_bindir}/ic-promote
%{_bindir}/ic-join
%{_bindir}/configure-lightwave-server
%{_bindir}/configure-identity-server
%{_bindir}/domainjoin.sh
%{_lib64dir}/*.so*
%{_jarsdir}/*.jar
%{_configdir}/firewall.json
%{_configdir}/setfirewallrules.py
%{_serviceddir}/firewall.service

%exclude %{_lib64dir}/*.a
%exclude %{_lib64dir}/*.la

# %doc ChangeLog README COPYING

%changelog
