Name:    lightwave
Summary: VMware Lightwave
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64

Requires: pkgconfig(openssl) >= 1.0.2, pkgconfig(krb5) >= 1.14, pkgconfig(sqlite3) >= 3.14, pkgconfig(libsasl2) >= 2.1, coreutils >= 8.22, likewise-open >= 6.2.10, gawk >= 4.1.3, boost = 1.60.0, apache-commons-daemon >= 1.0.15, tomcat >= 8.5.8, lightwave-client = %{_version}
%if 0%{?fedora} >= 21
Requires: java-1.8.0-openjdk >= 1.8.0.112
%else
Requires: openjdk >= 1.8.0.112
%endif

BuildRequires: openssl-devel >= 1.0.2, coreutils >= 8.22, likewise-open-devel >= 6.2.10, python2-devel >= 2.7.8, boost-devel = 1.60.0
%if 0%{?fedora} >= 21
BuildRequires: java-1.8.0-openjdk >= 1.8.0.112, ant >= 1.9.4, maven >= 3.3.9
%else
BuildRequires: openjdk >= 1.8.0.112, apache-ant >= 1.9.4, apache-maven >= 3.3.9
%endif

%description
VMware Lightwave Server

#
# The _unpackaged_files_terminate_build macro, if set to 1,
# tells rpmbuild to exit if it finds files that are in the
# $RPM_BUILD_ROOT directory but not listed as part of the
# package.
#
# Set this macro to 0 to turn off the Fascist build policy
#
%define _unpackaged_files_terminate_build 0

%define _jarsdir %{_prefix}/jars
%define _bindir %{_prefix}/bin
%define _webappsdir %{_prefix}/vmware-sts/webapps
%define _configdir %{_prefix}/share/config
%define _servicedir /lib/systemd/system

%if 0%{?_likewise_open_prefix:1} == 0
%define _likewise_open_prefix /opt/likewise
%endif

%define _likewise_open_bindir %{_likewise_open_prefix}/bin
%define _likewise_open_sbindir %{_likewise_open_prefix}/sbin

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

%if 0%{?_sasl_prefix:1} == 0
%define _sasl_prefix /usr
%endif

%if 0%{?_krb5_prefix:1} == 0
%define _krb5_prefix /usr
%endif

%if 0%{?_vmevent_prefix:1} == 0
%define _vmevent_prefix /opt/vmware
%endif

%if 0%{?_jansson_prefix:1} == 0
%define _jansson_prefix /usr
%endif

%if 0%{?_copenapi_prefix:1} == 0
%define _copenapi_prefix /usr
%endif

%if 0%{?_oidc_prefix:1} == 0
%define _oidc_prefix /opt/vmware
%endif

%define _sasl2dir %{_sasl_prefix}/lib64/sasl2
%define _krb5_lib_dir %{_krb5_prefix}/lib64
%define _krb5_gss_conf_dir /etc/gss
%define _logdir /var/log/lightwave
%define _logconfdir /etc/syslog-ng/lightwave.conf.d
%define _pymodulesdir /opt/vmware/site-packages/identity
%define _jreextdir %{_javahome}/jre/lib/ext

%define _lwraft_dbdir %{_localstatedir}/lwraft
%define _vmca_dbdir   %{_localstatedir}/vmca
%define _vmdir_dbdir  %{_localstatedir}/vmdir
%define _vmafd_dbdir  %{_localstatedir}/vmafd
%define _vmsts_dbdir  %{_localstatedir}/vmsts

%define _vecsdir %{_vmafd_dbdir}/vecs
%define _crlsdir %{_vmafd_dbdir}/crl

%package client
Summary: Lightwave Client
Requires: pkgconfig(openssl) >= 1.0.2, pkgconfig(krb5) >= 1.14, pkgconfig(libsasl2) >= 2.1, coreutils >= 8.22, likewise-open >= 6.2.9, openjdk >= 1.8.0.112
%description client
Client libraries to communicate with Lightwave Services

%package devel
Summary: Lightwave Client Development Library
Requires: lightwave-client = %{_version}
%description devel
Development Libraries to communicate with Lightwave Services

%package raft
Summary: Lightwave Raft Service
Requires: lightwave-client = %{_version}
%description raft
Lightwave Raft Service

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            #
            # New Installation
            #
            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                # Not in chroot
                if [ -z "`pidof lwsmd`" ]; then
                    /bin/systemctl >/dev/null 2>&1
                    if [ $? -ne 0 ]; then
                        /bin/systemctl start lwsmd
                    fi
                fi
            fi
            ;;

        2)
            #
            # Upgrade
            #
            if [ ! -d %{_backupdir} ];
            then
                /bin/mkdir "%{_backupdir}"
            fi
            /bin/cp "%{_prefix}/vmware-sts/conf/server.xml" "%{_backupdir}/server.xml"
            ;;

    esac

%pre client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            #
            # New Installation
            #
            /bin/systemctl >/dev/null 2>&1
            if [ $? -eq 0 ]; then
                if [ -z "`pidof lwsmd`" ]; then
                    /bin/systemctl start lwsmd
                fi
            fi
            ;;

        2)
            #
            # Upgrade
            #
            ;;
    esac

%pre raft

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            #
            # New Installation
            #
            /bin/systemctl >/dev/null 2>&1
            if [ $? -eq 0 ]; then
                if [ -z "`pidof lwsmd`" ]; then
                    /bin/systemctl start lwsmd
                fi
            fi
            ;;

        2)
            #
            # Upgrade
            #
            ;;
    esac

%post

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /sbin/ldconfig

# config

    /bin/systemctl enable firewall.service >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        /bin/ln -s %{_servicedir}/firewall.service /etc/systemd/system/multi-user.target.wants/firewall.service
    fi

    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        /bin/systemctl daemon-reload
    fi
    /bin/systemctl start firewall.service

# vmdir

    /bin/mkdir -m 700 -p %{_vmdir_dbdir}

    if [ -a %{_sasl2dir}/vmdird.conf ]; then
        /bin/rm %{_sasl2dir}/vmdird.conf
    fi

    # add vmdird.conf to sasl2 directory
    /bin/ln -s %{_datadir}/config/saslvmdird.conf %{_sasl2dir}/vmdird.conf

    /bin/mkdir -m 755 -p %{_logconfdir}
    if [ -a %{_logconfdir}/vmdird-syslog-ng.conf ]; then
        /bin/rm %{_logconfdir}/vmdird-syslog-ng.conf
    fi
    /bin/ln -s %{_datadir}/config/vmdird-syslog-ng.conf %{_logconfdir}/vmdird-syslog-ng.conf

# vmdns

    /bin/mkdir -m 755 -p %{_logdir}
    /bin/mkdir -m 755 -p %{_logconfdir}
    if [ -a %{_logconfdir}/vmdnsd-syslog-ng.conf ]; then
        /bin/rm %{_logconfdir}/vmdnsd-syslog-ng.conf
    fi
    /bin/ln -s %{_datadir}/config/vmdnsd-syslog-ng.conf %{_logconfdir}/vmdnsd-syslog-ng.conf

# vmca

    /bin/mkdir -m 700 -p %{_vmca_dbdir}
    /bin/mkdir -m 755 -p %{_logdir}
    /bin/mkdir -m 755 -p %{_logconfdir}
    if [ -a %{_logconfdir}/vmcad-syslog-ng.conf ]; then
        /bin/rm %{_logconfdir}/vmcad-syslog-ng.conf
    fi
    /bin/ln -s %{_datadir}/config/vmcad-syslog-ng.conf %{_logconfdir}/vmcad-syslog-ng.conf

    case "$1" in
        1)
            #
            # New Installation
            #
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmca.reg
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 2
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmca.reg
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi

        /bin/systemctl enable vmware-stsd.service >/dev/null 2>&1
        if [ $? -ne 0 ]; then
            /bin/ln -s /lib/systemd/system/vmware-stsd.service /etc/systemd/system/multi-user.target.wants/vmware-stsd.service
        fi
        /bin/systemctl >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            /bin/systemctl daemon-reload
        fi

            ;;

        2)
            #
            # Upgrade
            #

            %{_sbindir}/configure-build.sh "%{_backupdir}"

            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdns.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmca.reg
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 2
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmca.reg
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi
            ;;
    esac

if [ -x "%{_lwisbindir}/lwregshell" ]
then
    %{_lwisbindir}/lwregshell list_keys "[HKEY_THIS_MACHINE\Software\VMware\Identity]" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        # add key if not exist
        %{_lwisbindir}/lwregshell add_key "[HKEY_THIS_MACHINE\Software\VMware\Identity]"
    fi

    %{_lwisbindir}/lwregshell list_values "[HKEY_THIS_MACHINE\Software\VMware\Identity]" | grep "Release" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        # add value if not exist
        %{_lwisbindir}/lwregshell add_value "[HKEY_THIS_MACHINE\Software\VMware\Identity]" "Release" REG_SZ "Lightwave"
    fi

    %{_lwisbindir}/lwregshell list_values "[HKEY_THIS_MACHINE\Software\VMware\Identity]" | grep "Version" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        # add value if not exist
        %{_lwisbindir}/lwregshell add_value "[HKEY_THIS_MACHINE\Software\VMware\Identity]" "Version" REG_SZ "%{_version}"
    else
        # set value if exists
        %{_lwisbindir}/lwregshell set_value "[HKEY_THIS_MACHINE\Software\VMware\Identity]" "Version" "%{_version}"
    fi
fi


%post client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /bin/mkdir -m 755 -p %{_logdir}

    # add libgssapi_srp.so to GSSAPI plugin directory
    if [ ! -h %{_krb5_lib_dir}/gss/libgssapi_srp.so ]; then
        /bin/ln -s %{_lib64dir}/libgssapi_srp.so %{_krb5_lib_dir}/gss/libgssapi_srp.so
    fi

    # Add GSSAPI SRP plugin configuration to GSS mech file
    if [ -f %{_krb5_gss_conf_dir}/mech ]; then
        if [ `grep -c  "1.2.840.113554.1.2.10" %{_krb5_gss_conf_dir}/mech` -lt 1 ]; then
            echo "srp  1.2.840.113554.1.2.10 libgssapi_srp.so" >> %{_krb5_gss_conf_dir}/mech
        fi
    fi

    # Restore commented out NTLM mech oid if found
    if [ `grep -c  "#ntlm " %{_krb5_gss_conf_dir}/mech` -ge 1 ]; then
        /bin/mv %{_krb5_gss_conf_dir}/mech %{_krb5_gss_conf_dir}/mech-$$
        /bin/cat %{_krb5_gss_conf_dir}/mech-$$ | sed 's|^#ntlm|ntlm|' > %{_krb5_gss_conf_dir}/mech
        if [ -s %{_krb5_gss_conf_dir}/mech ]; then
            /bin/rm %{_krb5_gss_conf_dir}/mech-$$
        fi
    fi

    /bin/mkdir -m 700 -p %{_vmafd_dbdir}
    /bin/mkdir -m 700 -p %{_vecsdir}
    /bin/mkdir -m 700 -p %{_crlsdir}

    /bin/mkdir -m 755 -p %{_logdir}
    /bin/mkdir -m 755 -p %{_logconfdir}
    if [ -a %{_logconfdir}/vmafdd-syslog-ng.conf ]; then
        /bin/rm %{_logconfdir}/vmafdd-syslog-ng.conf
    fi
    /bin/ln -s %{_datadir}/config/vmafdd-syslog-ng.conf %{_logconfdir}/vmafdd-syslog-ng.conf

    case "$1" in
        1)
            #
            # New Installation
            #
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmafd.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir-client.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns-client.reg
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmafd.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir-client.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns-client.reg
                if [ $started_lwregd = true ]; then
                    kill `pidof lwregd`
                    wait
                fi
            fi
            ;;

        2)
            #
            # Upgrade
            #
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmafd.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir-client.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdns-client.reg
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmafd.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir-client.reg
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdns-client.reg
                if [ $started_lwregd = true ]; then
                    kill `pidof lwregd`
                    wait
                fi
            fi
            ;;
    esac

%post raft

    /bin/mkdir -m 700 -p %{_lwraft_dbdir}

    if [ -a %{_sasl2dir}/lwraftd.conf ]; then
        /bin/rm %{_sasl2dir}/lwraftd.conf
    fi

    # add lwraftd.conf to sasl2 directory
    /bin/ln -s %{_datadir}/config/sasllwraftd.conf %{_sasl2dir}/lwraftd.conf

    /bin/mkdir -m 755 -p %{_logdir}
    /bin/mkdir -m 755 -p %{_logconfdir}
    if [ -a %{_logconfdir}/lwraftd-syslog-ng.conf ]; then
        /bin/rm %{_logconfdir}/lwraftd-syslog-ng.conf
    fi
    /bin/ln -s %{_datadir}/config/lwraftd-syslog-ng.conf %{_logconfdir}/lwraftd-syslog-ng.conf

    case "$1" in
        1)
            #
            # New Installation
            #
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/lwraft.reg
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 2
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/lwraft.reg
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi
            ;;

        2)
            #
            # Upgrade
            #
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/lwraft.reg
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 2
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/lwraft.reg
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi
            ;;
    esac

%preun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            #
            # Uninstall
            #
            /bin/systemctl >/dev/null 2>&1
            if [ $? -eq 0 ]; then
                 if [ -f /etc/systemd/system/vmware-stsd.service ]; then
                     /bin/systemctl stop vmware-stsd.service
                     /bin/systemctl disable vmware-stsd.service
                     /bin/rm -f /etc/systemd/system/vmware-stsd.service
                     /bin/systemctl daemon-reload
                 fi
            fi

            %{_likewise_open_bindir}/lwsm info vmca > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                echo "Stopping the Certificate Authority Service..."
                %{_likewise_open_bindir}/lwsm stop vmca
                echo "Removing service configuration..."
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmca'
                echo "Restarting service control manager..."
                /bin/systemctl restart lwsmd
                sleep 2
                echo "Autostart services..."
                %{_likewise_open_bindir}/lwsm autostart
            fi

            %{_likewise_open_bindir}/lwsm info vmdir > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop vmdir
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmdir'
                /bin/systemctl restart lwsmd
                %{_likewise_open_bindir}/lwsm autostart
            fi

# dns also?

            /bin/systemctl >/dev/null 2>&1
            if [ $? -eq 0 ]; then
                 if [ -f /etc/systemd/system/firewall.service ]; then
                     /bin/systemctl stop firewall.service
                     /bin/systemctl disable firewall.service
                     /bin/rm -f /etc/systemd/system/firewall.service
                     /bin/systemctl daemon-reload
                 fi
            fi

            if [ -h %{_logconfdir}/vmdird-syslog-ng.conf ]; then
                /bin/rm -f %{_logconfdir}/vmdird-syslog-ng.conf
            fi
            if [ -h %{_logconfdir}/vmcad-syslog-ng.conf ]; then
                /bin/rm -f %{_logconfdir}/vmcad-syslog-ng.conf
            fi
            if [ -h %{_logconfdir}/vmdnsd-syslog-ng.conf ]; then
                /bin/rm -f %{_logconfdir}/vmdnsd-syslog-ng.conf
            fi
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

%preun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            #
            # Uninstall
            #
            %{_likewise_open_bindir}/lwsm info vmafd > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                echo "Stopping the AFD Service..."
                %{_likewise_open_bindir}/lwsm stop vmafd
                echo "Removing service configuration..."
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmafd'
                echo "Restarting service control manager..."
                /bin/systemctl restart lwsmd
                sleep 2
                echo "Autostart services..."
                %{_likewise_open_bindir}/lwsm autostart
            fi

            # Cleanup GSSAPI SRP symlink
            if [ -h %{_krb5_lib_dir}/gss/libgssapi_srp.so ]; then
                /bin/rm -f %{_krb5_lib_dir}/gss/libgssapi_srp.so
            fi

            # Remove GSSAPI SRP Plugin configuration from GSS mech file
            if [ -f %{_krb5_gss_conf_dir}/mech ]; then
                if [ `grep -c "1.2.840.113554.1.2.10" %{_krb5_gss_conf_dir}/mech` -gt 0 ]; then
                    /bin/cat %{_krb5_gss_conf_dir}/mech | sed '/1.2.840.113554.1.2.10/d' > "/tmp/mech-$$"
                    if [ -s /tmp/mech-$$ ]; then
                        /bin/mv "/tmp/mech-$$" %{_krb5_gss_conf_dir}/mech
                    fi
                fi
            fi

            if [ -h %{_logconfdir}/vmafdd-syslog-ng.conf ]; then
                /bin/rm -f %{_logconfdir}/vmafdd-syslog-ng.conf
            fi
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

%preun raft

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            #
            # Uninstall
            #
            %{_likewise_open_bindir}/lwsm info lwraft > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop lwraft
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\lwraft'
                /bin/systemctl restart lwsmd
                %{_likewise_open_bindir}/lwsm autostart
            fi
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

%postun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig

    case "$1" in
        0)
            #
            # Uninstall
            #
            echo "Existing database files kept at [%{_vmdir_dbdir}]."

            if [ -x "%{_lwisbindir}/lwregshell" ]
            then
                %{_lwisbindir}/lwregshell list_keys "[HKEY_THIS_MACHINE\Software\VMware\Identity]" > /dev/null 2>&1
                if [ $? -eq 0 ]; then
                    # delete key if exist
                    %{_lwisbindir}/lwregshell delete_tree "[HKEY_THIS_MACHINE\Software\VMware\Identity]"
                fi
            fi
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

    if [ -a %{_sasl2dir}/vmdird.conf ]; then
        /bin/rm %{_sasl2dir}/vmdird.conf
    fi

%postun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig

    case "$1" in
        0)
            #
            # Uninstall
            #
            echo "Existing VECS files kept under [%{_vmafd_dbdir}]"
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

%postun raft

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig

    case "$1" in
        0)
            #
            # Uninstall
            #
            echo "Existing database files kept at [%{_lwraft_dbdir}]."
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

    if [ -a %{_sasl2dir}/lwraftd.conf ]; then
        /bin/rm %{_sasl2dir}/lwraftd.conf
    fi

%files

%defattr(-,root,root,0755)

%{_bindir}/ic-promote
%{_bindir}/ic-join
%{_bindir}/configure-lightwave-server
%{_bindir}/configure-sts
%{_bindir}/configure-identity-server
%{_bindir}/test-ldapbind
%{_bindir}/test-logon
%{_bindir}/test-svr
%{_bindir}/vdcadmintool
%{_bindir}/vdcbackup
%{_bindir}/vdcleavefed
%{_bindir}/vdcpass
%{_bindir}/vdcrepadmin
%{_bindir}/vdcsetupldu
%{_bindir}/vdcsrp
%{_bindir}/unix_srp
%{_bindir}/vdcupgrade
%{_bindir}/vmkdc_admin
%{_bindir}/vdcmetric
%{_bindir}/vdcschema
%{_bindir}/vmdir_upgrade.sh
%{_bindir}/vdcresetMachineActCred

%{_sbindir}/vmcad
%{_sbindir}/vmdird
%{_sbindir}/vmdnsd
%{_sbindir}/vmware-stsd.sh
%{_sbindir}/configure-build.sh
%{_sbindir}/sso-config.sh

%{_lib64dir}/sasl2/libsaslvmdirdb.so*

%{_datadir}/config/vmca.reg
%{_datadir}/config/vmcad-syslog-ng.conf
%{_datadir}/config/saslvmdird.conf
%{_datadir}/config/vmdir.reg
%{_datadir}/config/vmdirschema.ldif
%{_datadir}/config/vmdird-syslog-ng.conf
%{_datadir}/config/vmdir-rest.json
%{_datadir}/config/vmdns.reg
%{_datadir}/config/vmdnsd-syslog-ng.conf
%{_datadir}/config/idm/*

%{_jarsdir}/openidconnect-client-lib.jar
%{_jarsdir}/openidconnect-common.jar
%{_jarsdir}/openidconnect-protocol.jar
%{_jarsdir}/samlauthority.jar
%{_jarsdir}/vmware-identity-diagnostics.jar
%{_jarsdir}/vmware-identity-idm-server.jar
%{_jarsdir}/vmware-identity-rest-afd-server.jar
%{_jarsdir}/vmware-identity-rest-core-server.jar
%{_jarsdir}/vmware-identity-rest-idm-server.jar
%{_jarsdir}/vmware-directory-rest-server.jar
%{_jarsdir}/vmware-identity-install.jar
%{_jarsdir}/vmware-identity-sso-config.jar
%{_jarsdir}/websso.jar
%{_jarsdir}/sts.jar
%{_jarsdir}/openidconnect-server.jar
%{_jarsdir}/commons-lang-2.6.jar
%{_jarsdir}/commons-logging-1.1.1.jar
%{_jarsdir}/jna-4.2.1.jar
%{_jarsdir}/httpclient-4.5.1.jar
%{_jarsdir}/slf4j-api-1.7.10.jar
%{_jarsdir}/log4j-api-2.2.jar
%{_jarsdir}/log4j-slf4j-impl-2.2.jar
%{_jarsdir}/log4j-core-2.2.jar

%{_webappsdir}/lightwaveui.war
%{_webappsdir}/ROOT.war

%{_configdir}/firewall.json
%{_configdir}/setfirewallrules.py

%{_servicedir}/firewall.service
%{_servicedir}/vmware-stsd.service

%config %attr(600, root, root) %{_prefix}/vmware-sts/bin/setenv.sh
%config %attr(600, root, root) %{_prefix}/vmware-sts/bin/vmware-identity-tomcat-extensions.jar
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/catalina.policy
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/catalina.properties
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/context.xml
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/logging.properties
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/server.xml
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/web.xml
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/tomcat-users.xml

%files client

%defattr(-,root,root)

%{_bindir}/cdc-cli
%{_bindir}/certool
%{_bindir}/dir-cli
%{_bindir}/domainjoin
%{_bindir}/domainjoin.sh
%{_bindir}/lw-support-bundle.sh
%{_bindir}/sl-cli
%{_bindir}/vmafd-cli
%{_bindir}/vmdns-cli
%{_bindir}/vdcaclmgr
%{_bindir}/vdcpromo
%{_bindir}/vecs-cli

%{_sbindir}/vmafdd

%{_lib64dir}/libvecsjni.so*
%{_lib64dir}/libcdcjni.so*
%{_lib64dir}/libheartbeatjni.so*
%{_lib64dir}/libvmafcfgapi.so*
%{_lib64dir}/libvmafdclient.so*
%{_lib64dir}/libvmeventclient.so*
%{_lib64dir}/libvmcaclient.so*
%{_lib64dir}/libvmdirclient.so*
%{_lib64dir}/libkrb5crypto.so*
%{_lib64dir}/libvmkdcserv.so*
%{_lib64dir}/libcsrp.so*
%{_lib64dir}/libgssapi_ntlm.so*
%{_lib64dir}/libgssapi_srp.so*
%{_lib64dir}/libgssapi_unix.so*
%{_lib64dir}/libvmdnsclient.so*
%{_lib64dir}/libcfgutils.so*
%{_lib64dir}/libidm.so*
%{_lib64dir}/liblwraftclient.so*
%{_lib64dir}/libssoafdclient.so*
%{_lib64dir}/libssocommon.so*
%{_lib64dir}/libssocoreclient.so*
%{_lib64dir}/libssoidmclient.so*
%{_lib64dir}/libssooidc.so*
%{_lib64dir}/libssovmdirclient.so*
%{_lib64dir}/libvmdirauth.so*

%{_datadir}/config/java.security.linux
%{_datadir}/config/certool.cfg
%{_datadir}/config/vmafd.reg
%{_datadir}/config/vmdir-client.reg
%{_datadir}/config/vmdns-client.reg
%{_datadir}/config/vmafdd-syslog-ng.conf

%{_jreextdir}/vmware-endpoint-certificate-store.jar
%{_jreextdir}/client-domain-controller-cache.jar
%{_jreextdir}/afd-heartbeat-service.jar

%{_jarsdir}/authentication-framework.jar
%{_jarsdir}/vmware-identity-rest-idm-samples.jar
%{_jarsdir}/vmware-vmca-client.jar
%{_jarsdir}/samltoken.jar
%{_jarsdir}/vmware-identity-rest-idm-common.jar
%{_jarsdir}/vmware-directory-rest-common.jar
%{_jarsdir}/vmware-directory-rest-client.jar
%{_jarsdir}/vmware-identity-rest-core-common.jar
%{_jarsdir}/vmware-identity-websso-client.jar
%{_jarsdir}/vmware-identity-platform.jar
%{_jarsdir}/vmware-identity-wsTrustClient.jar
%{_jarsdir}/vmware-identity-rest-afd-common.jar
%{_jarsdir}/openidconnect-common.jar
%{_jarsdir}/openidconnect-client-lib.jar
%{_jarsdir}/vmware-identity-idm-client.jar
%{_jarsdir}/vmware-identity-idm-interface.jar
%{_jarsdir}/vmware-identity-rest-afd-client.jar
%{_jarsdir}/vmware-identity-rest-core-client.jar
%{_jarsdir}/vmware-identity-rest-idm-client.jar

%{_sysconfdir}/vmware/java/vmware-override-java.security

%files raft

%defattr(-,root,root)

%{_sbindir}/lwraftd

%{_bindir}/lwraft_upgrade.sh
%{_bindir}/lwraftadmintool
%{_bindir}/lwraftleavefed
%{_bindir}/lwraftpromo
%{_bindir}/lwraftschema

%{_lib64dir}/sasl2/libsasllwraftdb.so*

%{_datadir}/config/sasllwraftd.conf
%{_datadir}/config/lwraftschema.ldif
%{_datadir}/config/lwraft-rest.json
%{_datadir}/config/lwraft.reg
%{_datadir}/config/lwraftd-syslog-ng.conf
%{_datadir}/config/lwraft-client.reg

%files devel

%defattr(-,root,root)

%{_includedir}/vmafd.h
%{_includedir}/vmafdtypes.h
%{_includedir}/vmafdclient.h
%{_includedir}/vecsclient.h
%{_includedir}/cdcclient.h
%{_includedir}/vmsuperlogging.h
%{_includedir}/vmca.h
%{_includedir}/vmcatypes.h
%{_includedir}/vmdir.h
%{_includedir}/vmdirauth.h
%{_includedir}/vmdirclient.h
%{_includedir}/vmdirerrors.h
%{_includedir}/vmdirtypes.h
%{_includedir}/vmdns.h
%{_includedir}/vmdnstypes.h

%{_lib64dir}/libcdcjni.a
%{_lib64dir}/libcdcjni.la
%{_lib64dir}/libvecsjni.a
%{_lib64dir}/libvecsjni.la
%{_lib64dir}/libheartbeatjni.a
%{_lib64dir}/libheartbeatjni.la
%{_lib64dir}/libvmafdclient.a
%{_lib64dir}/libvmafdclient.la
%{_lib64dir}/libvmafcfgapi.a
%{_lib64dir}/libvmafcfgapi.la
%{_lib64dir}/libvmeventclient.a
%{_lib64dir}/libvmeventclient.la
%{_lib64dir}/libvmcaclient.a
%{_lib64dir}/libvmcaclient.la
%{_lib64dir}/libvmdirclient.a
%{_lib64dir}/libvmdirclient.la
%{_lib64dir}/libcsrp.a
%{_lib64dir}/libcsrp.la
%{_lib64dir}/libgssapi_ntlm.a
%{_lib64dir}/libgssapi_ntlm.la
%{_lib64dir}/libgssapi_srp.a
%{_lib64dir}/libgssapi_srp.la
%{_lib64dir}/libgssapi_unix.a
%{_lib64dir}/libgssapi_unix.la
%{_lib64dir}/libvmdnsclient.a
%{_lib64dir}/libvmdnsclient.la

#
# TBD - not sure if these should be included or excluded
#
%{_includedir}/oidc.h
%{_includedir}/oidc_types.h
%{_includedir}/ssoafdclient.h
%{_includedir}/ssocoreclient.h
%{_includedir}/ssoerrors.h
%{_includedir}/ssoidmclient.h
%{_includedir}/ssotypes.h
%{_includedir}/ssocommon.h
%{_includedir}/ssovmdirclient.h
%{_includedir}/vmevent.h

%exclude %{_bindir}/vdcvmdirpromo
%exclude %{_bindir}/vmdirclienttest
%exclude %{_bindir}/*test

%exclude %{_lib64dir}/*.la
%exclude %{_lib64dir}/*.a
%exclude %{_lib64dir}/sasl2/*.a
%exclude %{_lib64dir}/sasl2/*.la
%exclude %{_lib64dir}/libcommonunittests.*
%exclude %{_lib64dir}/libmisctests.*
%exclude %{_lib64dir}/libmultitenancytests.*
%exclude %{_lib64dir}/libpasswordapistests.*
%exclude %{_lib64dir}/libsearchtests.*
%exclude %{_lib64dir}/libsecuritydescriptortests.*

%exclude %{_prefix}/site-packages/identity/*
%exclude %{_webappsdir}/openidconnect-sample-rp.war

# %doc ChangeLog README COPYING

%changelog
