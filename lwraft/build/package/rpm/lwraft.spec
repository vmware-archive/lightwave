Name:    vmware-post
Summary: VMware Persistent Objectstore
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open >= 6.2.10, jansson >= 2.9, copenapi >= 0.0.1, c-rest-engine >= 1.0.1, vmware-sts-c-client = %{version}, vmware-post-client = %{version} vmware-directory-client = %{version}
BuildRequires:  coreutils >= 8.22, openssl-devel >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open-devel >= 6.2.10, jansson-devel >= 2.9, copenapi-devel >= 0.0.1, c-rest-engine-devel >= 1.0.1, vmware-sts-c-client = %{version}, vmware-event-devel >= %{_vmevent_ver}

%if 0%{?_sasl_prefix:1} == 0
%define _sasl_prefix /usr
%endif

%if 0%{?_krb5_prefix:1} == 0
%define _krb5_prefix /usr
%endif

%if 0%{?_likewise_open_prefix:1} == 0
%define _likewise_open_prefix /opt/likewise
%endif

%define _likewise_open_bindir %{_likewise_open_prefix}/bin
%define _likewise_open_sbindir %{_likewise_open_prefix}/sbin

#The %_unpackaged_files_terminate_build macro,
#if set to 1, tells rpmbuild to exit if it finds files that are in the #
#$RPM_BUILD_ROOT directory but not listed as part of the package. #
#Set this macro to 0 to turn off the Fascist build policy
%define _unpackaged_files_terminate_build 0

%if 0%{?_vmevent_prefix:1} == 0
%define _vmevent_prefix /opt/vmware
%endif

%if 0%{?_jansson_prefix:1} == 0
%define _jansson_prefix /usr
%endif

%if 0%{?_copenapi_prefix:1} == 0
%define _copenapi_prefix /usr
%endif

%if 0%{?_c_rest_engine_prefix:1} == 0
%define _c_rest_engine_prefix /usr
%endif

%if 0%{?_oidc_prefix:1} == 0
%define _oidc_prefix /opt/vmware
%endif

%define _dbdir %{_localstatedir}/lib/vmware/post
%define _sasl2dir %{_sasl_prefix}/lib64/sasl2
%define _krb5_lib_dir %{_krb5_prefix}/lib64
%define _krb5_gss_conf_dir /etc/gss
%define _logdir /var/log/lightwave
%define _logconfdir /etc/syslog-ng/lightwave.conf.d

%description
VMware Persistent Objectstore Service

%debug_package

%package client
Summary: VMWare Persistent Objectstore Client
Requires:  coreutils >= 8.22, openssl >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open >= 6.2.9
%description client
Client libraries to communicate with Ligthwave Raft Service

%package client-devel
Summary: VMware Persistent Objectstore Client Development Library
Requires: vmware-post-client = %{version}
%description client-devel
Development Libraries to communicate with Ligthwave Raft Service

%build
export CFLAGS="-Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare"
cd build
autoreconf -mif ..
../configure \
    --prefix=%{_prefix} \
    --libdir=%{_lib64dir} \
    --localstatedir=%{_localstatedir}/lib/vmware \
    --with-likewise=%{_likewise_open_prefix} \
    --with-ssl=/usr \
    --with-sasl=%{_sasl_prefix} \
    --with-datastore=mdb \
    --with-vmevent=%{_vmevent_prefix} \
    --with-jansson=%{_jansson_prefix} \
    --with-copenapi=%{_copenapi_prefix} \
    --with-c-rest-engine=%{_c_rest_engine_prefix} \
    --with-oidc=%{_oidc_prefix} \
    --enable-server=yes \
    --with-version=%{_version} \
    --enable-lightwave-build=yes \
    --enable-rest=%{_enable-rest}

make

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build && make install DESTDIR=$RPM_BUILD_ROOT

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        if [ -z "`pidof lwsmd`" ]; then
            /bin/systemctl start lwsmd
        fi
    fi

%pre client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    /bin/systemctl >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        if [ -z "`pidof lwsmd`" ]; then
            /bin/systemctl start lwsmd
        fi
    fi

%post

    /sbin/ldconfig

    /bin/mkdir -m 700 -p %{_dbdir}

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

    # TO REMOVE, temporary link to avoid breaking existing scripts
    rm -f %{_bindir}/lwraft-cli
    /bin/ln -s %{_bindir}/post-cli %{_bindir}/lwraft-cli
    rm -f %{_bindir}/lwraftschema
    /bin/ln -s %{_bindir}/postschema %{_bindir}/lwraftschema
    rm -f %{_bindir}/lwraftadmintool
    /bin/ln -s %{_bindir}/postsadmintool %{_bindir}/lwraftadmintool

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
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
                sleep 2
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
                sleep 2
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
    esac

%post client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/post-client.reg
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/post-client.reg
                if [ $started_lwregd = true ]; then
                    kill `pidof lwregd`
                    wait
                fi
            fi
            ;;
        2)
            try_starting_lwregd_svc=true

            if [ "$(stat -c %d:%i /)" != "$(stat -c %d:%i /proc/1/root/.)" ]; then
                try_starting_lwregd_svc=false
            fi

            /bin/systemctl >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                try_starting_lwregd_svc=false
            fi

            if [ $try_starting_lwregd_svc = true ]; then
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/post-client.reg
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/post-client.reg
                if [ $started_lwregd = true ]; then
                    kill `pidof lwregd`
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
            %{_likewise_open_bindir}/lwsm info post > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop post
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\post'
                /bin/systemctl restart lwsmd
                %{_likewise_open_bindir}/lwsm autostart
            fi

            ;;
    esac

%preun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

%postun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig

    if [ -a %{_sasl2dir}/postd.conf ]; then
        /bin/rm %{_sasl2dir}/postd.conf
    fi

    if [ "$1" = "0" ]; then
        echo "Existing database files kept at [%{_dbdir}]."
    fi

%postun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade
#    case "$1" in
#        0)
#            %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\post'
#            ;;
#    esac

%files
%defattr(-,root,root)
%{_sbindir}/*
%{_bindir}/postadmintool
%{_bindir}/postschema
%{_bindir}/post-cli
%{_lib64dir}/sasl2/libsaslpostdb.so*
%{_lib64dir}/libkrb5crypto.so*
%{_lib64dir}/libvmkdcserv.so*
%{_datadir}/config/saslpostd.conf
%{_datadir}/config/post.reg
%{_datadir}/config/postschema.ldif
%{_datadir}/config/postd-syslog-ng.conf
%{_datadir}/config/post-rest.json

%files client
%defattr(-,root,root)
%{_datadir}/config/post-client.reg
%{_lib64dir}/libpostclient.so*

%files client-devel
%defattr(-,root,root)
%{_lib64dir}/libpostclient.a
%{_lib64dir}/libpostclient.la

%exclude %{_bindir}/dequetest
%exclude %{_bindir}/lwraftclienttest
%exclude %{_bindir}/circularbuffertest
%exclude %{_bindir}/parseargstest
%exclude %{_bindir}/registrytest
%exclude %{_bindir}/stringtest
%exclude %{_lib64dir}/libkrb5crypto.a
%exclude %{_lib64dir}/libkrb5crypto.la
%exclude %{_lib64dir}/sasl2/libsaslpostdb.a
%exclude %{_lib64dir}/sasl2/libsaslpostdb.la
%exclude %{_lib64dir}/libvmkdcserv.a
%exclude %{_lib64dir}/libvmkdcserv.la

# %doc ChangeLog README COPYING

%changelog

