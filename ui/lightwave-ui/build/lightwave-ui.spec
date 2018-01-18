Summary:        A web based UI for lightwave server
Name:           lightwave-ui
Version:        0.1
Release:        1%{?dist}
License:        VMware
URL:            http://vmware.github.io/lightwave
Group:          Development/Tools
Vendor:         VMware, Inc.
BuildRequires:  apache-maven
Requires:       lightwave

%description
A web based UI for lightwave server that interacts with the REST interface.

%install
cd ../..
mkdir -p %{buildroot}
mkdir -p %{buildroot}/opt/vmware/vmware-sts/webapps/
install -m644 target/webui-1.3.0-SNAPSHOT.war %{buildroot}/opt/vmware/vmware-sts/webapps/webui.war

%postun
rm -r /opt/vmware/vmware-sts/webapps/webui

%files
%defattr(-,root,root)
/opt/vmware/vmware-sts/webapps/webui.war

%changelog
*    Mon Jan 08 2018 Harish Udaiya Kumar <hudaiyakumar@vmware.com> 0.1-1
-    Initial version of lightwave-webui.
