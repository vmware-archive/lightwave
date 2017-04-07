if [ $# -lt 1 ]
then
  DEST_DIR=$(pwd)
else
  DEST_DIR=$1
fi

timestamp=$(date +"%Y%m%d%H%M")

LW_REG_EXPORT_LOG="lwreg-export-$timestamp.reg"
BUILD_INFO_LOG="buildInfo.log"
NETSTAT_LOG="netstat.log"
SERVICE_STATUS_LOG="serviceStatus.log"
SUPPORT_BUNDLE_TAR_FILE=$DEST_DIR/lw-support-bundle-$timestamp.tar

# Create an archive for the support bundle, add logs from /var/log/lightwave and /var/log/vmware/
tar -cf $SUPPORT_BUNDLE_TAR_FILE /var/log/lightwave/* /var/log/vmware/* /var/log/journal/*

# export registry keys and add it to the archive, filter passwords out
/opt/likewise/bin/lwregshell export | pcregrep -vM 'Password" = {\n.*\n}' > $LW_REG_EXPORT_LOG
tar -rf $SUPPORT_BUNDLE_TAR_FILE $LW_REG_EXPORT_LOG

# capture build information and add it to archive
echo $'RPM Versions :\n' > $BUILD_INFO_LOG
rpm -qa | grep vmware >> $BUILD_INFO_LOG
echo $'\nlikewise-open versions :\n' >> $BUILD_INFO_LOG
tdnf list | grep likewise-open >> $BUILD_INFO_LOG
echo $'\napache-tomcat verions :\n' >> $BUILD_INFO_LOG
tdnf list | grep apache-tomcat >> $BUILD_INFO_LOG
echo $'\ncommons-daemon versions :\n' >> $BUILD_INFO_LOG
tdnf list | grep commons-daemon >> $BUILD_INFO_LOG
echo $'\nopenjre versions :\n' >> $BUILD_INFO_LOG
tdnf list | grep openjre >> $BUILD_INFO_LOG
tar -rf $SUPPORT_BUNDLE_TAR_FILE $BUILD_INFO_LOG

# capture netstat output and add it to archive
netstat -ano > $NETSTAT_LOG
tar -rf $SUPPORT_BUNDLE_TAR_FILE $NETSTAT_LOG

# capture lwsm list output and add it to archive
/opt/likewise/bin/lwsm list > $SERVICE_STATUS_LOG
tar -rf $SUPPORT_BUNDLE_TAR_FILE $SERVICE_STATUS_LOG

# compress the tar file
gzip $SUPPORT_BUNDLE_TAR_FILE

# clean up temp files
rm $LW_REG_EXPORT_LOG
rm $BUILD_INFO_LOG
rm $NETSTAT_LOG
rm $SERVICE_STATUS_LOG

echo "Lightwave support bundle saved to : $SUPPORT_BUNDLE_TAR_FILE.gz"

