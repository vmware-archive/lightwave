Summary:        A web based UI for browsing the post node
Name:           lwraft-ui
Version:        0.1
Release:        1%{?dist}
License:        VMware
URL:            http://vmware.github.io/lightwave
Group:          Development/Tools
Vendor:         VMware, Inc.
BuildRequires:  nodejs
Requires:	jq
Requires:	nginx

%description
A web based UI for browsing the post node that interacts with the REST interface of post daemon.

%build
cd  ../../lwraft-ui
npm install
npm run prod

%install
cd ../../lwraft-ui
install -dm755 %{buildroot}
install -dm755 %{buildroot}/opt/vmware/lwraftui
install -dm755 %{buildroot}/opt/vmware/config
install -dm755 %{buildroot}/opt/vmware/tools
install -m644 build/nginx_lwraftui.conf %{buildroot}/opt/vmware/config/
install -m755 build/oidc-client-utils-lwraftui %{buildroot}/opt/vmware/tools/
cp -r config dist/
cp -a dist/* %{buildroot}/opt/vmware/lwraftui/
find %{buildroot}/opt/vmware/lwraftui -type d -exec chmod 755 {} \;
find %{buildroot}/opt/vmware/lwraftui -type f -exec chmod 644 {} \;
%post
lightwaveui_cfg="/opt/vmware/config/nginx_lightwaveui.conf"
lwraftui_cfg="/opt/vmware/config/nginx_lwraftui.conf"
if [ -f "$lightwaveui_cfg" ]
then
    cp $lightwaveui_cfg $lwraftui_cfg
    sed -i '$ d' $lwraftui_cfg
    sed -i '$ d' $lwraftui_cfg
    echo "
        location /lwraftui {
            root /opt/vmware;
            index  index.html index.htm;
        }
    }
}" >> $lwraftui_cfg
    rm /etc/nginx/nginx.conf
else
    mv /etc/nginx/nginx.conf /etc/nginx/nginx_orig.conf
fi
ln -s /opt/vmware/config/nginx_lwraftui.conf /etc/nginx/nginx.conf
systemctl restart nginx
echo "Please register this POST node as OIDC client on lightwave server as shown below:"
echo "/opt/vmware/tools/oidc-client-utils-lwraftui"
/opt/vmware/tools/oidc-client-utils-lwraftui register -e

%postun
rm /etc/nginx/nginx.conf
lightwaveui_cfg="/opt/vmware/config/nginx_lightwaveui.conf"
if [ -f "$lightwaveui_cfg" ]
then
    ln -s $lightwaveui_cfg /etc/nginx/nginx.conf
else
    mv /etc/nginx/nginx_orig.conf /etc/nginx/nginx.conf
fi
systemctl restart nginx

%files
%defattr(-,root,root)
%dir /opt/vmware/lwraftui
/opt/vmware/lwraftui
/opt/vmware/tools
/opt/vmware/config

%changelog
*    Mon Jan 08 2018 Harish Udaiya Kumar <hudaiyakumar@vmware.com> 0.1-1
-    Initial version of lwraft-ui.
