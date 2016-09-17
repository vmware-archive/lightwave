Name:    vmware-lightwave-clients
Summary: VMware Infrastructure Client
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, likewise-open >= 6.2.10, vmware-directory-client = %{version}, vmware-afd = %{version}, vmware-ca-client = %{version}, vmware-ic-config = %{version}, vmware-dns-client = %{version}

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

