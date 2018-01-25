Name:    lightwave
Summary: VMware Lightwave
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64

Requires: openssl >= 1.0.2, coreutils >= 8.22, cyrus-sasl >= 2.1, c-rest-engine = 1.1, likewise-open >= 6.2.11, gawk >= 4.1.3, boost = 1.60.0, lightwave-server = %{_version}, lightwave-client = %{_version}
BuildRequires: openssl-devel >= 1.0.2, coreutils >= 8.22, likewise-open-devel >= 6.2.11, python2-devel >= 2.7.8, boost-devel = 1.60.0, c-rest-engine-devel = 1.1

%if 0%{?fedora} >= 21
Requires: java-1.8.0-openjdk >= 1.8.0.131, krb5-libs >= 1.14, sqlite >= 3.14, tomcat >= 8.5.16, apache-commons-daemon >= 1.0.15, apache-commons-daemon-jsvc >= 1.0.15
BuildRequires: java-1.8.0-openjdk >= 1.8.0.131, ant >= 1.9.4, maven >= 3.3.9
%else
Requires: openjre >= 1.8.0.131, krb5 >= 1.14, sqlite-autoconf >= 3.14, apache-tomcat >= 8.5.16, commons-daemon >= 1.0.15
BuildRequires: openjdk >= 1.8.0.131, apache-ant >= 1.9.4, apache-maven >= 3.3.9
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

%define _post_dbdir   %{_localstatedir}/post
%define _vmca_dbdir   %{_localstatedir}/vmca
%define _vmdir_dbdir  %{_localstatedir}/vmdir
%define _vmafd_dbdir  %{_localstatedir}/vmafd
%define _vmafd_rundir /var/run/vmafd
%define _vmsts_dbdir  %{_localstatedir}/vmsts

%define _vecsdir %{_vmafd_dbdir}/vecs
%define _crlsdir %{_vmafd_dbdir}/crl

%package client
Summary: Lightwave Client
Requires: openssl >= 1.0.2, coreutils >= 8.22, cyrus-sasl >= 2.1, likewise-open >= 6.2.11, gawk >= 4.1.3, boost = 1.60.0
%if 0%{?fedora} >= 21
Requires: krb5-libs >= 1.14, sqlite >= 3.14
%else
Requires: krb5 >= 1.14, sqlite-autoconf >= 3.14
%endif
%description client
Client libraries to communicate with Lightwave services

%package server
Summary: Lightwave Server
Requires: lightwave-client = %{_version}
%description server
Lightwave services

%package devel
Summary: Lightwave Client Development Library
Requires: lightwave-client = %{_version}
%description devel
Development libraries to communicate with Lightwave services

%package post
Summary: Lightwave POST Service
Requires: lightwave-client >= %{_version}
%description post
Lightwave POST service

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            #
            # New Installation
            #
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


%pre server

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

%pre post

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

    case "$1" in
        1)
            #
            # New Installation
            #
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

%post server

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /sbin/ldconfig

    # start the firewall service
    /bin/systemctl restart firewall.service
    if [ $? -ne 0 ]; then
        echo "Firewall service not restarted"
    fi
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
                sleep 5
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
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdns.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmca.reg
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 5
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdns.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmca.reg
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi
            ;;
    esac

%post client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    # config firewall service for server/post

    /bin/systemctl enable firewall.service >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        /bin/ln -s %{_servicedir}/firewall.service /etc/systemd/system/multi-user.target.wants/firewall.service
    fi

    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        /bin/systemctl daemon-reload
    fi
    /bin/systemctl restart firewall.service

    /bin/mkdir -m 755 -p %{_logdir}

    SRP_MECH_OID="1.2.840.113554.1.2.10"
    UNIX_MECH_OID="1.3.6.1.4.1.6876.11711.2.1.2"

    # add libgssapi_srp.so to GSSAPI plugin directory
    if [ ! -h %{_krb5_lib_dir}/gss/libgssapi_srp.so ]; then
        /bin/ln -s %{_lib64dir}/libgssapi_srp.so %{_krb5_lib_dir}/gss/libgssapi_srp.so
    fi

    # Add GSSAPI SRP plugin configuration to GSS mech file
    if [ -f %{_krb5_gss_conf_dir}/mech ]; then
        if [ `grep -c  "$SRP_MECH_OID" %{_krb5_gss_conf_dir}/mech` -lt 1 ]; then
            echo "srp $SRP_MECH_OID libgssapi_srp.so" >> %{_krb5_gss_conf_dir}/mech
        fi
    fi

    # Add GSSAPI UNIX plugin configuration to GSS mech file
    if [ -f %{_krb5_gss_conf_dir}/mech ]; then
        if [ `grep -c  "$UNIX_MECH_OID" %{_krb5_gss_conf_dir}/mech` -lt 1 ]; then
            echo "#unix  $UNIX_MECH_OID libgssapi_unix.so" >> %{_krb5_gss_conf_dir}/mech
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
    /bin/mkdir -m 777 -p %{_vmafd_rundir}
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
                %{_likewise_open_bindir}/lwsm -q refresh
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
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 5
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmafd.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir-client.reg
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdns-client.reg
                if [ $started_lwregd = true ]; then
                    kill `pidof lwregd`
                    wait
                fi
            fi
            ;;
    esac

%post post

    # start the firewall service
    /bin/systemctl restart firewall.service
    if [ $? -ne 0 ]; then
        echo "Firewall service not restarted"
    fi

    # make post db directory
    /bin/mkdir -m 700 -p %{_post_dbdir}

    if [ -a %{_sasl2dir}/postd.conf ]; then
        /bin/rm %{_sasl2dir}/postd.conf
    fi

    # add postd.conf to sasl2 directory
    /bin/ln -s %{_datadir}/config/saslpostd.conf %{_sasl2dir}/postd.conf

    /bin/mkdir -m 755 -p %{_logdir}
    /bin/mkdir -m 755 -p %{_logconfdir}
    if [ -a %{_logconfdir}/postd-syslog-ng.conf ]; then
        /bin/rm %{_logconfdir}/postd-syslog-ng.conf
    fi
    /bin/ln -s %{_datadir}/config/postd-syslog-ng.conf %{_logconfdir}/postd-syslog-ng.conf

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
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/post.reg
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 5
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/post.reg
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
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/post.reg
                %{_likewise_open_bindir}/lwsm -q refresh
                sleep 5
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/post.reg
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
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

%preun server

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            #
            # Uninstall
            #

            %{_likewise_open_bindir}/lwsm info vmca > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop vmca
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmca'
            fi

            %{_likewise_open_bindir}/lwsm info vmdir > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop vmdir
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmdir'
            fi

            %{_likewise_open_bindir}/lwsm info vmdns > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop vmdns
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmdns'
            fi

            /bin/systemctl restart lwsmd
            sleep 5

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
                %{_likewise_open_bindir}/lwsm stop vmafd
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmafd'
                /bin/systemctl restart lwsmd
                sleep 5
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -eq 0 ]; then
                 if [ -f /etc/systemd/system/firewall.service ]; then
                     /bin/systemctl stop firewall.service
                     /bin/systemctl disable firewall.service
                     /bin/rm -f /etc/systemd/system/multi-user.target.wants/firewall.service
                     /bin/systemctl daemon-reload
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

%preun post

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
            #
            # Uninstall
            #
            %{_likewise_open_bindir}/lwsm info post > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop post
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\post'
                /bin/systemctl restart lwsmd
                sleep 5
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

%postun server

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig

    case "$1" in
        0)
            #
            # Uninstall
            #
            if [ -f %{_vmdir_dbdir}/data.mdb ]; then
                # backup db if exists
                mv %{_vmdir_dbdir}/data.mdb %{_vmdir_dbdir}/data.mdb.bak
            fi

            echo "Existing database files kept at [%{_vmdir_dbdir}]."

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

            # Un-configure SRP/UNIX mech authentication plugins
            SRP_MECH_OID="1.2.840.113554.1.2.10"
            UNIX_MECH_OID="1.3.6.1.4.1.6876.11711.2.1.2"

            # Cleanup GSSAPI SRP symlink
            if [ -h %{_libdir}/gss/libgssapi_srp.so ]; then
                rm -f %{_libdir}/gss/libgssapi_srp.so
            fi

            # Cleanup GSSAPI UNIX symlink
            if [ -h %{_libdir}/gss/libgssapi_unix.so ]; then
                rm -f %{_libdir}/gss/libgssapi_unix.so
            fi

            # Remove GSSAPI SRP plugin configuration from GSS mech file
            if [ -f %{_krb5_gss_conf_dir} ]; then
                if [ `grep -c  "$SRP_MECH_OID" %{_krb5_gss_conf_dir}` -gt 0 ]; then
                    cat %{_krb5_gss_conf_dir} | sed "/$SRP_MECH_OID/d" > "/tmp/mech-$$"
                    if [ -s /tmp/mech-$$ ]; then
                        mv "/tmp/mech-$$" %{_krb5_gss_conf_dir}
                    fi
                fi
            fi

            # Remove GSSAPI UNIX plugin configuration from GSS mech file
            if [ -f %{_krb5_gss_conf_dir} ]; then
                if [ `grep -c  "$UNIX_MECH_OID" %{_krb5_gss_conf_dir}` -gt 0 ]; then
                    cat %{_krb5_gss_conf_dir} | sed "/$UNIX_MECH_OID/d" > "/tmp/mech-$$"
                    if [ -s /tmp/mech-$$ ]; then
                        mv "/tmp/mech-$$" %{_krb5_gss_conf_dir}
                    fi
                fi
            fi

            # Cleanup vmafd db and files
            if [ -d %{_vmafd_dbdir} ]; then
                rm -rf %{_vmafd_dbdir}
            fi

            # Cleanup vmafd run dir
            if [ -d %{_vmafd_rundir} ]; then
                rm -rf %{_vmafd_rundir}
            fi

            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

%postun post

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig

    case "$1" in
        0)
            #
            # Uninstall
            #
            echo "Existing database files kept at [%{_post_dbdir}]."
            ;;

        1)
            #
            # Upgrade
            #
            ;;
    esac

    if [ -a %{_sasl2dir}/postd.conf ]; then
        /bin/rm %{_sasl2dir}/postd.conf
    fi

%files

%defattr(-,root,root,0755)

%{_bindir}/configure-sts
%{_bindir}/configure-identity-server

%{_sbindir}/vmware-stsd.sh
%{_sbindir}/configure-build.sh
%{_sbindir}/sso-config.sh

%{_datadir}/config/idm/*

%{_jarsdir}/samlauthority.jar
%{_jarsdir}/vmware-identity-diagnostics.jar
%{_jarsdir}/vmware-identity-install.jar
%{_jarsdir}/vmware-identity-sso-config.jar
%{_jarsdir}/websso.jar
%{_jarsdir}/sts.jar
%{_jarsdir}/openidconnect-protocol.jar
%{_jarsdir}/args4j-2.33.jar
%{_jarsdir}/commons-codec-1.9.jar
%{_jarsdir}/commons-lang-2.6.jar
%{_jarsdir}/commons-lang3-3.3.2.jar
%{_jarsdir}/commons-logging-1.2.jar
%{_jarsdir}/jersey-media-json-jackson-2.25.1.jar
%{_jarsdir}/jackson-core-2.8.4.jar
%{_jarsdir}/jackson-databind-2.8.4.jar
%{_jarsdir}/jackson-annotations-2.8.4.jar
%{_jarsdir}/jna-4.2.1.jar
%{_jarsdir}/json-smart-1.3.1.jar
%{_jarsdir}/httpclient-4.5.1.jar
%{_jarsdir}/httpcore-4.4.4.jar
%{_jarsdir}/slf4j-api-1.7.25.jar
%{_jarsdir}/log4j-api-2.8.2.jar
%{_jarsdir}/log4j-slf4j-impl-2.8.2.jar
%{_jarsdir}/log4j-core-2.8.2.jar
%{_jarsdir}/nimbus-jose-jwt-4.12.jar

%{_webappsdir}/lightwaveui.war
%{_webappsdir}/ROOT.war

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
%config %attr(600, root, root) %{_prefix}/vmware-sts/conf/vmsts-telegraf.conf

%files server

%defattr(-,root,root,0755)

%{_bindir}/ic-promote
%{_bindir}/configure-lightwave-server
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
%{_bindir}/vmdir_upgrade.sh
%{_bindir}/vdcresetMachineActCred

%{_sbindir}/vmcad
%{_sbindir}/vmdird
%{_sbindir}/vmdnsd

%{_lib64dir}/sasl2/libsaslvmdirdb.so*

%{_datadir}/config/vmca.reg
%{_datadir}/config/vmcad-syslog-ng.conf
%{_datadir}/config/vmca-telegraf.conf

%{_datadir}/config/saslvmdird.conf
%{_datadir}/config/vmdir.reg
%{_datadir}/config/vmdirschema.ldif
%{_datadir}/config/vmdird-syslog-ng.conf
%{_datadir}/config/vmdir-rest.json
%{_datadir}/config/vmdir-telegraf.conf

%{_datadir}/config/vmdns.reg
%{_datadir}/config/vmdns-rest.json
%{_datadir}/config/vmdnsd-syslog-ng.conf
%{_datadir}/config/vmdns-telegraf.conf

%{_configdir}/lw-firewall-server.json


%files client

%defattr(-,root,root)

%{_bindir}/ic-join
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
%{_bindir}/vdcschema
%{_bindir}/postschema
%{_bindir}/vecs-cli
%{_lib64dir}/libkrb5crypto.so*
%{_lib64dir}/libcsrp.so*

%{_sbindir}/vmafdd

%{_lib64dir}/libvecsjni.so*
%{_lib64dir}/libcdcjni.so*
%{_lib64dir}/libheartbeatjni.so*
%{_lib64dir}/libvmafcfgapi.so*
%{_lib64dir}/libvmafdclient.so*
%{_lib64dir}/libvmeventclient.so*
%{_lib64dir}/libvmcaclient.so*
%{_lib64dir}/libvmdirclient.so*
%{_lib64dir}/libvmkdcserv.so*
%{_lib64dir}/libgssapi_ntlm.so*
%{_lib64dir}/libgssapi_srp.so*
%{_lib64dir}/libgssapi_unix.so*
%{_lib64dir}/libvmdnsclient.so*
%{_lib64dir}/libcfgutils.so*
%{_lib64dir}/libidm.so*
%{_lib64dir}/libpostclient.so*
%{_lib64dir}/libssoafdclient.so*
%{_lib64dir}/libssocommon.so*
%{_lib64dir}/libssocoreclient.so*
%{_lib64dir}/libssoidmclient.so*
%{_lib64dir}/libssooidc.so*
%{_lib64dir}/libssovmdirclient.so*
%{_lib64dir}/libvmdirauth.so*
%{_lib64dir}/libvmmetrics.so*

%{_datadir}/config/java.security.linux
%{_datadir}/config/certool.cfg
%{_datadir}/config/vmafd.reg
%{_datadir}/config/vmdir-client.reg
%{_datadir}/config/vmdns-client.reg
%{_datadir}/config/vmafdd-syslog-ng.conf
%{_datadir}/config/telegraf.conf
%{_datadir}/config/vmafd-telegraf.conf

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

%{_configdir}/lw-firewall-client.json
%{_configdir}/setfirewallrules.py
%{_configdir}/lightwave-syslog-logrotate.conf

%{_servicedir}/firewall.service

%{_sysconfdir}/vmware/java/vmware-override-java.security

%files post

%defattr(-,root,root)

%{_sbindir}/postd

%{_bindir}/postadmintool
%{_bindir}/postaclmgr
%{_bindir}/post-cli

%{_lib64dir}/sasl2/libsaslpostdb.so*

%{_datadir}/config/saslpostd.conf
%{_datadir}/config/postschema.ldif
%{_datadir}/config/post-rest.json
%{_datadir}/config/post.reg
%{_datadir}/config/postd-syslog-ng.conf
%{_datadir}/config/post-client.reg
%{_datadir}/config/post-telegraf.conf

%{_configdir}/lw-firewall-post.json

%config %attr(750, root, root) %{_datadir}/config/refresh-resolve-conf.sh
%config %attr(750, root, root) %{_datadir}/config/post-demote-deads.sh
%config %attr(750, root, root) %{_datadir}/config/monitor-core-dump.sh

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
%{_includedir}/vmmetrics.h

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
%{_lib64dir}/libvmdnsclient.a
%{_lib64dir}/libvmdnsclient.la
%{_lib64dir}/libvmmetrics.a
%{_lib64dir}/libvmmetrics.la

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
