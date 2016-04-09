Name:    vmware-event-devel
Summary: VMware Event SDK
Version: 6.5.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22
BuildRequires:  coreutils >= 8.22

%description
VMware Event Service Software Development Kit

%build
cd build
autoreconf -mif ..
../configure \
    --prefix=%{_prefix}

make

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build && make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_includedir}/*

%changelog

