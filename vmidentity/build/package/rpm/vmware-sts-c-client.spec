Name:    vmware-sts-c-client
Summary: VMware Secure Token Service C Client
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  openssl >= 1.0.2
BuildRequires: openssl-devel >= 1.0.2

%define _dbdir %_localstatedir/lib/vmware/vmsts

%description
C Client libraries to communicate with VMware Secure Token Service

%debug_package

%build

cd build
autoreconf -mif .. &&
../configure --prefix=%{_prefix} \
             --libdir=%{_lib64dir} \
             --localstatedir=%{_dbdir} \
             --with-afd=%{_prefix} \
             --with-likewise=%{_likewise_open_prefix} \
             --with-jansson=%{_janssondir} \
             --with-curl=%{_curldir} \
             --with-ssl=%{_ssldir} \
             --with-java=%{_javahome} \
             --with-commons-daemon=%{_commons_daemondir} \
             --with-ant=%{_antdir} \
             --with-tomcat=%{_tomcatdir} \
             --with-jax-ws=%{_jaxwsdir} \
             --with-maven=%{_mavendir}

cd ssoclients
make

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build/ssoclients && make install DESTDIR=%{buildroot}

%files
%defattr(-,root,root,0755)
%{_includedir}/*.h
%{_lib64dir}/*.so*
%{_lib64dir}/*.la
%{_lib64dir}/*.a

%exclude %{_bindir}/*test

# %doc ChangeLog README COPYING

%changelog

