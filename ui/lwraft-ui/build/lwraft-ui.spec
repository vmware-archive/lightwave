Summary:        A web based UI for browsing the post node
Name:           lwraft-ui
Version:        0.1
Release:        1%{?dist}
License:        VMware
URL:            http://vmware.github.io/lightwave
Group:          Development/Tools
Vendor:         VMware, Inc.
BuildRequires:  nodejs
Requires:	nginx

%description
A web based UI for browsing the post node that interacts with the REST interface of post daemon.

%build
cd  ../..
npm install
npm run prod

%install
cd ../..
mkdir -p %{buildroot}
mkdir -p %{buildroot}/opt/vmware/lwraft/ui
install -m644 build/nginx.conf %{buildroot}/opt/vmware/lwraft/ 
install -m644 dist/* %{buildroot}/opt/vmware/lwraft/ui

%post
mkdir /etc/vmware/vmware-vmafd
/opt/likewise/bin/lwsm restart vmafd
mv /etc/nginx/nginx.conf /etc/nginx/nginx_orig.conf
ln -s /opt/vmware/lwraft/nginx.conf /etc/nginx/nginx.conf
systemctl restart nginx

%postun
rm /etc/nginx/nginx.conf
mv /etc/nginx/nginx_orig.conf /etc/nginx/nginx.conf
systemctl restart nginx

%files
%defattr(-,root,root)
%dir /opt/vmware/lwraft
/opt/vmware/lwraft

%changelog
*    Mon Jan 08 2018 Harish Udaiya Kumar <hudaiyakumar@vmware.com> 0.1-1
-    Initial version of lwraft-ui.
