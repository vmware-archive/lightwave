Name:    vmware-lightwave-clients
Summary: VMware Infrastructure Client
Version: 6.6.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, likewise-open >= 6.2.9, vmware-directory-client >= 6.6.0, vmware-afd >= 6.6.0, vmware-ca-client >= 6.6.0, vmware-ic-config >= 6.6.0, vmware-dns-client >= 6.6.0

%description
VMware Infrastructure Controller Clients

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

