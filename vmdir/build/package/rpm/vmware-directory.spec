Name:    vmware-directory
Summary: Directory Service
Version: 6.0.0
Release: 0
Group:   Applications/System
Vendor:  VMware, Inc.
License: VMware
URL:     http://www.vmware.com
BuildArch: x86_64
Requires:  coreutils >= 8.22, openssl >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open >= 6.2.0, vmware-directory-client = %{version}
BuildRequires:  coreutils >= 8.22, openssl-devel >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open-devel >= 6.2.0

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

%define _dbdir %{_localstatedir}/lib/vmware/vmdir
%define _sasl2dir %{_sasl_prefix}/lib64/sasl2
%define _krb5_lib_dir %{_krb5_prefix}/lib64
%define _krb5_gss_conf_dir /etc/gss

%description
VMware Directory Service

%package client
Summary: VMware Directory Client
Requires:  coreutils >= 8.22, openssl >= 1.0.1, krb5 >= 1.12, cyrus-sasl >= 2.1, likewise-open >= 6.2.0
%description client
Client libraries to communicate with Directory Service

%package client-devel
Summary: VMware Directory Client Development Library
Requires: vmware-directory-client = %{version}
%description client-devel
Development Libraries to communicate with Directory Service

%build
export CFLAGS="-Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare"
cd build
autoreconf -mif ..
../configure \
    --prefix=%{_prefix} \
    --libdir=%{_lib64dir} \
    --localstatedir=%{_localstatedir}/lib/vmware/vmdir \
    --with-likewise=%{_likewise_open_prefix} \
    --with-ssl=/usr \
    --with-sasl=%{_sasl_prefix} \
    --with-datastore=mdb

make

%install

[ %{buildroot} != "/" ] && rm -rf %{buildroot}/*
cd build && make install DESTDIR=$RPM_BUILD_ROOT

%pre

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    if [ -z "`pidof lwsmd`" ]; then
        /bin/systemctl start lwsmd
    fi

%pre client

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    if [ -z "`pidof lwsmd`" ]; then
        /bin/systemctl start lwsmd
    fi

%post

    /sbin/ldconfig

    /bin/mkdir -m 700 -p %{_dbdir}

    if [ -a %{_sasl2dir}/vmdird.conf ]; then
        /bin/rm %{_sasl2dir}/vmdird.conf
    fi

    # add vmdird.conf to sasl2 directory
    /bin/ln -s %{_datadir}/config/saslvmdird.conf %{_sasl2dir}/vmdird.conf

    if [ -a %{_sasl2dir}/libsaslvmdirdb.so ]; then
        /bin/rm %{_sasl2dir}/libsaslvmdirdb.so 
    fi

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir.reg
            %{_likewise_open_bindir}/lwsm -q refresh
            sleep 2
            ;;         
        2)
            %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir.reg
            %{_likewise_open_bindir}/lwsm -q refresh
            sleep 2
            ;;
    esac

%post client

    # add libgssapi_srp.so to GSSAPI plugin directory
    if [ ! -h %{_krb5_lib_dir}/gss/libgssapi_srp.so ]; then
        /bin/ln -s %{_lib64dir}/libgssapi_srp.so %{_krb5_lib_dir}/gss/libgssapi_srp.so
    fi

    # Add GSSAPI SRP plugin configuration to GSS mech file
    if [ -f %{_krb5_gss_conf_dir}/mech ]; then
        if [ `grep -c  "1.2.840.113554.1.2.10" %{_krb5_gss_conf_dir}/mech` -lt 1 ]; then
            echo "srp  1.2.840.113554.1.2.10 libgssapi_srp.so" >> %{_krb5_gss_conf_dir}/mech
        fi

        # Comment out the NTLM mech oid; interferes with SRP authentication.
        if [ `grep -c  "^ntlm " %{_krb5_gss_conf_dir}/mech` -ge 1 ]; then
            mv %{_krb5_gss_conf_dir}/mech %{_krb5_gss_conf_dir}/mech-$$
            cat %{_krb5_gss_conf_dir}/mech-$$ | sed 's|^ntlm|#ntlm|' > %{_krb5_gss_conf_dir}/mech
            if [ -s %{_krb5_gss_conf_dir}/mech ]; then
                rm %{_krb5_gss_conf_dir}/mech-$$
            fi
        fi
    fi

    # First argument is 1 => New Installation
    # First argument is 2 => Upgrade

    case "$1" in
        1)
            %{_likewise_open_bindir}/lwregshell import %{_datadir}/config/vmdir-client.reg
            ;;         
        2)
            %{_likewise_open_bindir}/lwregshell upgrade %{_datadir}/config/vmdir-client.reg
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

            # Restore commented out NTLM mech oid if found
            if [ `grep -c  "#ntlm " %{_krb5_gss_conf_dir}/mech` -ge 1 ]; then
                /bin/mv %{_krb5_gss_conf_dir}/mech %{_krb5_gss_conf_dir}/mech-$$
                /bin/cat %{_krb5_gss_conf_dir}/mech-$$ | sed 's|^#ntlm|ntlm|' > %{_krb5_gss_conf_dir}/mech
                if [ -s %{_krb5_gss_conf_dir}/mech ]; then
                    /bin/rm %{_krb5_gss_conf_dir}/mech-$$
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

    if [ -a %{_sasl2dir}/libsaslvmdirdb.so ]; then
        /bin/rm %{_sasl2dir}/libsaslvmdirdb.so
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
%{_bindir}/vdcupgrade
%{_bindir}/vmkdc_admin
%{_lib64dir}/libkrb5crypto.so*
%{_lib64dir}/libsaslvmdirdb.so*
%{_lib64dir}/libvmkdcserv.so*
%{_datadir}/config/saslvmdird.conf
%{_datadir}/config/vmdir.reg
%{_datadir}/config/vmdirschema.ldif

%files client
%defattr(-,root,root)
%{_datadir}/config/vmdir-client.reg
%{_lib64dir}/libvmdirclient.so*
%{_lib64dir}/libcsrp.so*
%{_lib64dir}/libgssapi_ntlm.so*
%{_lib64dir}/libgssapi_srp.so*

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

%exclude %{_bindir}/dequetest
%exclude %{_bindir}/vdcpromo
%exclude %{_bindir}/vmdirclienttest
%exclude %{_lib64dir}/libkrb5crypto.a
%exclude %{_lib64dir}/libkrb5crypto.la
%exclude %{_lib64dir}/libsaslvmdirdb.a
%exclude %{_lib64dir}/libsaslvmdirdb.la
%exclude %{_lib64dir}/libvmkdcserv.a
%exclude %{_lib64dir}/libvmkdcserv.la

# %doc ChangeLog README COPYING

%changelog

