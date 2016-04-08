Name:    vmware-ic-config
Summary: VMware Infrastructure Controller Configuration Tool
Version: 6.5.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.1, likewise-open >= 6.2.9, vmware-directory-client >= 6.5.0, vmware-afd-client >= 6.5.0, vmware-ca-client >= 6.5.0, gawk >= 4.1.0
BuildRequires: coreutils >= 8.22, openssl-devel >= 1.0.1, likewise-open-devel >= 6.2.9, vmware-directory-client-devel >= 6.5.0, vmware-afd-client-devel >= 6.5.0, vmware-ca-client-devel >= 6.5.0

%define _jarsdir %{_prefix}/jars
%define _bindir %{_prefix}/bin

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

%if 0%{?_vmsts_prefix:1} == 0
%define _vmsts_prefix /opt/vmware
%endif

%description
VMware Infrastructure Controller Configuration Tool

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

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%postun

    /sbin/ldconfig

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%files
%defattr(-,root,root,0755)
%{_bindir}/ic-promote
%{_bindir}/ic-join
%{_bindir}/configure-lightwave-server
%{_bindir}/domainjoin.sh
%{_lib64dir}/*.so*
%{_jarsdir}/*.jar

%exclude %{_lib64dir}/*.a
%exclude %{_lib64dir}/*.la

# %doc ChangeLog README COPYING

%changelog

