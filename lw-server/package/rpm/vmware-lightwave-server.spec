Name:    vmware-lightwave-server
Summary: VMware Lightwave Server
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, likewise-open >= 6.2.10, vmware-directory = %{version}, vmware-afd = %{version}, vmware-ca = %{version}, vmware-ic-config = %{version}, vmware-sts = %{version}, vmware-dns = %{version}

%description
VMware Infrastructure Controller

%build

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

%post

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade
case "$1" in
    1)
        # Configure syslog-ng
        LINE='@include "lightwave.conf.d"'
        FILE=/etc/syslog-ng/syslog-ng.conf
        if [ -f "$FILE" ]; then
            grep -qs "$LINE" "$FILE"
            if [ "$?" -ne 0 ]; then
                echo "$LINE" >> "$FILE"
                pid=$( pidof syslog-ng )
                if [ -n "$pid" ]; then
                    kill -HUP $pid
                fi
            fi
        fi
        ;;
esac

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%postun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%files
%defattr(-,root,root,0755)

%changelog

