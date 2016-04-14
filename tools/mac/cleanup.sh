# Purpose: Cleanups up files and libs installed with LightwaveUITools package.
# Rum following command on Mac after moving Application -> LightwaveTools to Trash.
# > sudo sh cleanup.sh

echo 'cleanup started'

# Remove the Lightwave UI pkg links:

if [ -d "/usr/lib" ]; then
	echo 'cleaning up libs under /usr/lib'
	cd /usr/lib
	echo 'unlinking libcrypto.1.0.1.dylib'
	sudo unlink libcrypto.1.0.1.dylib
	echo 'unlinking libssl.1.0.1.dylib'
	sudo unlink libssl.1.0.1.dylib
fi

# Remove vmdir client pkg links:
if [ -d "/opt/likewise/lib/gss" ]; then
	echo 'cleaning up libs under /opt/likewise/lib/gss'
	cd /opt/likewise/lib/gss
	echo 'unlinking libgssapi_srp.dylib'
	sudo unlink libgssapi_srp.dylib
	echo 'unlinking libgssapi_srp.so'
	sudo unlink libgssapi_srp.so
fi

# Trash following folders:
echo 'cleaning up /opt/likewise'
sudo rm -rf /opt/likewise

echo 'cleaning up /opt/vmware'
sudo rm -rf /opt/vmware

echo 'cleanup complete'