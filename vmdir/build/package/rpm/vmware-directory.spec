Name:    vmware-directory
Summary: Directory Service
Version: %{_version}
Release: %{_patch}
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open >= 6.2.11, vmware-directory-client = %{version}
BuildRequires:  coreutils >= 8.22, openssl-devel >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open-devel >= 6.2.11, vmware-event-devel >= %{_vmevent_ver}

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

%if 0%{?_vmevent_prefix:1} == 0
%define _vmevent_prefix /opt/vmware
%endif

%if 0%{?_trident_prefix:1} == 0
%define _trident_prefix /opt/vmware
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

%if 0%{?_ssocommon_prefix:1} == 0
%define _ssocommon_prefix /opt/vmware
%endif

%define _dbdir %{_localstatedir}/lib/vmware/vmdir
%define _sasl2dir %{_sasl_prefix}/lib64/sasl2
%define _krb5_lib_dir %{_krb5_prefix}/lib64
%define _krb5_gss_conf_dir /etc/gss
%define _logconfdir /etc/syslog-ng/lightwave.conf.d

%description
VMware Directory Service

%debug_package

%package client
Summary: VMware Directory Client
Requires:  coreutils >= 8.22, openssl >= 1.0.2, krb5 >= 1.14, cyrus-sasl >= 2.1, likewise-open >= 6.2.10
%description client
Client libraries to communicate with Directory Service

%package client-devel
Summary: VMware Directory Client Development Library
Requires: vmware-directory-client = %{version}
%description client-devel
Development Libraries to communicate with Directory Service

%build
export CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wimplicit-function-declaration -Wno-address -Wno-enum-compare"
cd build
autoreconf -mif ..
../configure \
    --prefix=%{_prefix} \
    --libdir=%{_lib64dir} \
    --localstatedir=%{_localstatedir}/lib/vmware/vmdir \
    --with-likewise=%{_likewise_open_prefix} \
    --with-ssl=/usr \
    --with-sasl=%{_sasl_prefix} \
    --with-datastore=mdb \
    --with-vmevent=%{_vmevent_prefix} \
    --with-trident=%{_trident_prefix} \
    --with-jansson=%{_jansson_prefix} \
    --with-copenapi=%{_copenapi_prefix} \
    --with-oidc=%{_oidc_prefix} \
    --with-ssocommon=%{_ssocommon_prefix} \
    --enable-server=yes \
    --with-logdir=%{_logdir} \
    --with-version=%{_version} \
    --enable-lightwave-build=yes

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
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir.reg
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
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir.reg
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
                if [ $started_lwregd = true ]; then
                    kill -TERM `pidof lwregd`
                    wait
                fi
            fi
            ;;
    esac

%post client
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
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir-client.reg
            else
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir-client.reg
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
                %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir-client.reg
                started_lwregd=false
                if [ -z "`pidof lwregd`" ]; then
                    echo "Starting lwregd"
                    %{_likewise_open_sbindir}/lwregd &
                    started_lwregd=true
                    sleep 5
                fi
                %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir-client.reg
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
            %{_likewise_open_bindir}/lwsm info vmdir > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                %{_likewise_open_bindir}/lwsm stop vmdir
                %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmdir'
                /bin/systemctl restart lwsmd
                %{_likewise_open_bindir}/lwsm autostart
            fi

            ;;
    esac

%preun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    case "$1" in
        0)
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

            ;;
    esac

%postun

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade

    /sbin/ldconfig

    if [ -a %{_sasl2dir}/vmdird.conf ]; then
        /bin/rm %{_sasl2dir}/vmdird.conf
    fi

    if [ "$1" = "0" ]; then
        echo "Existing database files kept at [%{_dbdir}]."
    fi

%postun client

    # First argument is 0 => Uninstall
    # First argument is 1 => Upgrade
#    case "$1" in
#        0)
#            %{_likewise_open_bindir}/lwregshell delete_tree 'HKEY_THIS_MACHINE\Services\vmdir'
#            ;;
#    esac

%files
%defattr(-,root,root)
%{_sbindir}/*
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
%{_lib64dir}/libkrb5crypto.so*
%{_lib64dir}/sasl2/libsaslvmdirdb.so*
%{_lib64dir}/libvmkdcserv.so*
%{_datadir}/config/saslvmdird.conf
%{_datadir}/config/vmdir.reg
%{_datadir}/config/vmdirschema.ldif
%{_datadir}/config/vmdird-syslog-ng.conf
%{_datadir}/config/vmdir-rest.json

%files client
%defattr(-,root,root)
%{_bindir}/vdcaclmgr
%{_datadir}/config/vmdir-client.reg
%{_lib64dir}/libvmdirclient.so*
%{_lib64dir}/libcsrp.so*
%{_lib64dir}/libgssapi_ntlm.so*
%{_lib64dir}/libgssapi_srp.so*
%{_lib64dir}/libgssapi_unix.so*

%files client-devel
%defattr(-,root,root)
%{_includedir}/vmdir.h
%{_includedir}/vmdirauth.h
%{_includedir}/vmdirclient.h
%{_includedir}/vmdirerrors.h
%{_includedir}/vmdirtypes.h
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

%exclude %{_bindir}/vdcvmdirpromo
%exclude %{_bindir}/vmdirclienttest
%exclude %{_lib64dir}/libcommonunittests.a
%exclude %{_lib64dir}/libcommonunittests.la
%exclude %{_lib64dir}/libcommonunittests.so
%exclude %{_lib64dir}/libcommonunittests.so.0
%exclude %{_lib64dir}/libcommonunittests.so.0.0.0
%exclude %{_lib64dir}/libmisctests.a
%exclude %{_lib64dir}/libmisctests.la
%exclude %{_lib64dir}/libmisctests.so
%exclude %{_lib64dir}/libmisctests.so.0
%exclude %{_lib64dir}/libmisctests.so.0.0.0
%exclude %{_lib64dir}/libmultitenancytests.a
%exclude %{_lib64dir}/libmultitenancytests.la
%exclude %{_lib64dir}/libmultitenancytests.so
%exclude %{_lib64dir}/libmultitenancytests.so.0
%exclude %{_lib64dir}/libmultitenancytests.so.0.0.0
%exclude %{_lib64dir}/libpasswordapistests.a
%exclude %{_lib64dir}/libpasswordapistests.la
%exclude %{_lib64dir}/libpasswordapistests.so
%exclude %{_lib64dir}/libpasswordapistests.so.0
%exclude %{_lib64dir}/libpasswordapistests.so.0.0.0
%exclude %{_lib64dir}/libsearchtests.a
%exclude %{_lib64dir}/libsearchtests.la
%exclude %{_lib64dir}/libsearchtests.so
%exclude %{_lib64dir}/libsearchtests.so.0
%exclude %{_lib64dir}/libsearchtests.so.0.0.0
%exclude %{_lib64dir}/libsecuritydescriptortests.a
%exclude %{_lib64dir}/libsecuritydescriptortests.la
%exclude %{_lib64dir}/libsecuritydescriptortests.so
%exclude %{_lib64dir}/libsecuritydescriptortests.so.0
%exclude %{_lib64dir}/libsecuritydescriptortests.so.0.0.0

%exclude %{_lib64dir}/libkrb5crypto.a
%exclude %{_lib64dir}/libkrb5crypto.la
%exclude %{_lib64dir}/sasl2/libsaslvmdirdb.a
%exclude %{_lib64dir}/sasl2/libsaslvmdirdb.la
%exclude %{_lib64dir}/libvmkdcserv.a
%exclude %{_lib64dir}/libvmkdcserv.la

# %doc ChangeLog README COPYING

%changelog

