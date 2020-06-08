Summary:        A web based UI for lightwave server.
Name:           lightwave-ui
Version:        0.1
Release:        2%{?dist}
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
npm i -g gulp-cli@2.3.0
npm install
gulp

cd ../lwdir
npm install
npm run prod

%install
install -dm755 %{buildroot}
install -dm755 %{buildroot}/opt/vmware/lightwaveui
install -dm755 %{buildroot}/opt/vmware/config
install -dm755 %{buildroot}/opt/vmware/lightwaveui/directory
mkdir -p %{buildroot}/opt/vmware/tools
cd ../../lightwave-ui/idm
cp -a dist/* %{buildroot}/opt/vmware/lightwaveui/

find %{buildroot}/opt/vmware/lightwaveui -type d -exec chmod 755 {} \;
find %{buildroot}/opt/vmware/lightwaveui -type f -exec chmod 644 {} \;

install -m644 ../build/nginx_lightwaveui.conf %{buildroot}/opt/vmware/config
install -m755 ../build/oidc-client-utils-lightwaveui %{buildroot}/opt/vmware/tools

cd ../lwdir
install -m644 dist/* %{buildroot}/opt/vmware/lightwaveui/directory

%post
lwraftui_cfg="/opt/vmware/config/nginx_lwraftui.conf"
lightwaveuiui_cfg="/opt/vmware/config/nginx_lightwaveui.conf"
if [ -f "$lwraftui_cfg" ]
then
    cp $lwraft_cfg $lightwaveui_cfg
    sed -i '$ d' $lightwaveui_cfg
    sed -i '$ d' $lightwaveui_cfg
    echo "
        location /lightwaveui {
            root /opt/vmware;
            index  index.html index.htm;
        }
    }
}" >> $lightwaveui_cfg
    rm /etc/nginx/nginx.conf
else
    mv /etc/nginx/nginx.conf /etc/nginx/nginx_orig.conf
fi

ln -s /opt/vmware/config/nginx_lightwaveui.conf /etc/nginx/nginx.conf
systemctl restart nginx
echo "Please register this lightwave UI instance as OIDC client on lightwave server as shown below:"
echo "/opt/vmware/tools/oidc-client-utils-lightwaveui"
/opt/vmware/tools/oidc-client-utils-lightwaveui register -e

%postun
rm /etc/nginx/nginx.conf
lwraftui_cfg="/opt/vmware/config/nginx_lwraftui.conf"
if [ -f "$lwraftui_cfg" ]
then
    ln -s $lwraftui_cfg /etc/nginx/nginx.conf
else
    mv /etc/nginx/nginx_orig.conf /etc/nginx/nginx.conf
fi
systemctl restart nginx

%files
%defattr(-,root,root)
%dir /opt/vmware/lightwaveui
/opt/vmware/lightwaveui
/opt/vmware/tools
/opt/vmware/config

%changelog
*    Mon Jul 08 2020 Sriram Nambakam <snambakam@vmware.com> 0.1-2
*    Mon Jan 08 2018 Harish Udaiya Kumar <hudaiyakumar@vmware.com> 0.1-1
-    Initial version of lightwave-ui
