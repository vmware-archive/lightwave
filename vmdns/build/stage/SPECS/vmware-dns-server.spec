%global __os_install_post %{nil}
%define MajorVersion 1
%define MinorVersion 0
%define Revision     0
%define _prefix       /usr/lib/vmware-vmdns
%define _libdir       %{_prefix}/lib64
%define _bootstrapdir %{_prefix}/firstboot
%define _lwisbindir   /opt/likewise/bin
%define _firewalldir  %{_prefix}/firewall
%define _sysfirewalldir /etc/vmware/appliance/firewall

Name:    %{PackageName}
Summary: VMware DNS Service
Version: %{MajorVersion}.%{MinorVersion}.%{Revision}
Release: %{_build_number}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
Url:     http://www.vmware.com
PreReq:  grep, sh-utils, vmware-lwis >= 6.1.0
BuildRoot: %{_tmppath}

%description
VMware DNS Service

# Install
%install

source .buildenv

${INSTALL} -m755 -d %{buildroot}%{_firewalldir}
#${INSTALL} -m755 -d %{buildroot}%{_sysfirewalldir}
#/bin/ln -sf %{_firewalldir}/vmdns-firewall.json %{buildroot}%{_sysfirewalldir}/vmware-vmdns

# Pre-install
%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    if [ -z "`pidof lwsmd`" ]; then
        /etc/init.d/lwsmd start
    fi

# Post-install
%post

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

case "$1" in
    1)
        /sbin/chkconfig -a vmdnsd

        %{_lwisbindir}/lwregshell import %{_datadir}/config/vmdns.reg

        /etc/init.d/lwsmd reload
        ;;
    2)
        /sbin/chkconfig -c vmdnsd
        if [ $? -ne 0 ]; then
            /sbin/chkconfig -a vmdnsd
        fi

        %{_lwisbindir}/lwregshell upgrade %{_datadir}/config/vmdns.reg

        /etc/init.d/lwsmd reload
        ;;
esac

# Pre-uninstall
%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

if [ "$1" = 0 ]; then
    /sbin/chkconfig -d vmdnsd
    %{_lwisbindir}/lwsm stop vmdns
    %{_lwisbindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmdns'
    /etc/init.d/lwsmd restart
fi

# Post-uninstall
%postun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%files
%defattr(-,root,root,0755)
%{_sbindir}/vmdnsd
%{_bindir}/vmdns-cli
%{_libdir}/libvmdnsclient.*
%{_libdir}/libvmsock.*
%{_datadir}/config/*.reg
#%{_sysfirewalldir}/vmware-vmdns
%attr(755, root, root) %{_sysconfdir}/init.d/vmdnsd
%attr(700, root, root) %{_bootstrapdir}/vmdns-firstboot.py
%attr(644, root, root) %{_firewalldir}/vmdns-firewall.json

# %doc ChangeLog README COPYING

%changelog
