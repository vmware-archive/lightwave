Name:    vmware-lightwave-server
Summary: VMware Lightwave Server
Version: 6.0.3
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.1, likewise-open >= 6.2.9, vmware-directory >= 6.0.2, vmware-afd >= 6.0.2, vmware-ca >= 6.0.2, vmware-ic-config >= 1.0.3, vmware-sts >= 6.0, vmware-dns >= 6.0

%description
VMware Infrastructure Controller

%build

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

%post

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%postun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%files
%defattr(-,root,root,0755)

%changelog

