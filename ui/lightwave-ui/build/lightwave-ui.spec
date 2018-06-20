Summary:        A web based UI for lightwave server.
Name:           lightwave-ui
Version:        0.1
Release:        1%{?dist}
License:        VMware
URL:            http://vmware.github.io/lightwave
Group:          Development/Tools
Vendor:         VMware, Inc.
BuildRequires:  nodejs
Requires:       nginx
Requires:       jq
%description
A web based UI for lightwave server that includes identity management and lightwave directory browser.

%build
pwd
cd  ../../lightwave-ui/idm
mkdir -p ~/.npm-global
export NPM_CONFIG_PREFIX=~/.npm-global
export PATH=$NPM_CONFIG_PREFIX/bin:$PATH
npm i -g bower@1.8.4
npm i -g gulp-cli@2.0.1
bower install --allow-root
npm install
npm install
gulp

cd ../lwdir
npm install
npm run prod

%install
install -dm755 %{buildroot}
install -dm755 %{buildroot}/opt/vmware/lightwaveui
install -dm755 %{buildroot}/opt/vmware/lightwaveui/directory
mkdir -p %{buildroot}/opt/vmware/tools

cd ../../lightwave-ui/idm
cp -a dist/* %{buildroot}/opt/vmware/lightwaveui/
find %{buildroot}/opt/vmware/lightwaveui -type d -exec chmod 755 {} \;
find %{buildroot}/opt/vmware/lightwaveui -type f -exec chmod 644 {} \;

install -m644 ../build/nginx.conf %{buildroot}/opt/vmware/lightwaveui/config
install -m755 ../build/oidc-client-utils %{buildroot}/opt/vmware/tools

cd ../lwdir
install -m644 dist/* %{buildroot}/opt/vmware/lightwaveui/directory

%post
mv /etc/nginx/nginx.conf /etc/nginx/nginx_orig.conf
ln -s /opt/vmware/lightwaveui/config/nginx.conf /etc/nginx/nginx.conf
systemctl restart nginx
echo "Please register this lightwave ui instance as OIDC client on lightwave server as shown below:"
echo "/opt/vmware/tools/oidc-client-utils"
/opt/vmware/tools/oidc-client-utils register -e

%postun
rm /etc/nginx/nginx.conf
mv /etc/nginx/nginx_orig.conf /etc/nginx/nginx.conf
systemctl restart nginx

%files
%defattr(-,root,root)
%dir /opt/vmware/lightwaveui
/opt/vmware/lightwaveui
/opt/vmware/tools

%changelog
*    Mon Jan 08 2018 Harish Udaiya Kumar <hudaiyakumar@vmware.com> 0.1-1
-    Initial version of lightwave-ui
