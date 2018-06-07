#!/bin/bash -xe

echo "Installing codedeploy agent"

source /root/cloud-init/variables

aws s3 cp ${CODEDEPLOY_INSTALLER_LOCATION} /tmp/codedeploy-install
chmod +x /tmp/codedeploy-install
/tmp/codedeploy-install rpm

echo "Installing /usr/bin/service"

cat > /usr/bin/service <<EOF
#!/bin/bash
systemctl $2 $1
EOF

chmod +x /usr/bin/service
