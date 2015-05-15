Name:    vmware-ic-config
Summary: VMware Infrastructure Controller Configuration Tool
Version: 1.0.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.1, likewise-open >= 6.2.0, vmware-directory-client >= 6.0, vmware-afd-client >= 6.0, vmware-ca-client >= 6.0
BuildRequires: coreutils >= 8.22, openssl-devel >= 1.0.1, likewise-open-devel >= 6.2.0, vmware-directory-client-devel >= 6.0, vmware-afd-client-devel >= 6.0, vmware-ca-client-devel >= 6.0

%description
VMware Infrastructure Controller Configuration Tool

%build

cd build
autoreconf -mif .. &&
../configure --prefix=%{_prefix} \
             --libdir=%{_lib64dir} \
             --with-likewise=%{_likewise_open_prefix} \
             --with-vmdir=%{_prefix} \
             --with-vmca=%{_prefix} \
             --with-afd=%{_prefix} \
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
%{_lib64dir}/*.so*

%exclude %{_lib64dir}/*.a
%exclude %{_lib64dir}/*.la

# %doc ChangeLog README COPYING

%changelog

