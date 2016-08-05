#!/bin/bash

#makes a MacOSX installer with all UI apps. 
#Please install pre-built binaries,then custom built installer
#to satisfy dependencies.

if [ -z "$1" ]; then
  CONFIG="Debug"
else
  case $1 in
    "Debug"|"debug")
      CONFIG="Debug"
      ;;
    "Release"|"release")
      CONFIG="Release"
      ;;
    *)
      echo "Invalid config value: $1. Valid values are Debug or Release."
      exit 1
      ;;
  esac
fi

makePkg() {
  pkgbuild \
    --component "x64/$1/$2" \
    --install-location "/Applications/LightwaveTools/" \
	--identifier "$4" \
    "x64/$1/$3"
  if [ $? -ne 0 ];then
     exit 1
  fi
}

makePkgWithScript() {
  pkgbuild \
    --component "x64/$1/$2" \
    --install-location "/Applications/LightwaveTools/" \
	--identifier "$4" \
	--scripts "$5" \
    "x64/$1/$3"
  if [ $? -ne 0 ];then
     exit 1
  fi
}


makePkg $CONFIG 'Lightwave CA.app' 'LightwaveCA.pkg' 'com.vmware.LightwaveCA'
makePkg $CONFIG 'Lightwave Certificate Store.app' 'LightwaveCertStore.pkg' 'com.vmware.LightwaveCertStore'
makePkg $CONFIG 'Lightwave Directory.app' 'LightwaveDirectory.pkg' 'com.vmware.LightwaveDirectory'
makePkg $CONFIG 'Lightwave PSC Site Management.app' 'LightwavePSCSiteManagement.pkg' 'com.vmware.LightwavePSCSiteManagement'
makePkgWithScript $CONFIG 'Lightwave SSO.app' 'LightwaveSSO.pkg' 'com.vmware.LightwaveSSO' 'scripts'
makePkg $CONFIG 'Lightwave Directory Schema.app' 'LightwaveDirectorySchema.pkg' 'com.vmware.LightwaveDirectorySchema'

#generate distribution xml
productbuild --synthesize \
  --package "x64/$CONFIG/LightwaveCA.pkg"  \
  --package "x64/$CONFIG/LightwaveCertStore.pkg" \
  --package "x64/$CONFIG/LightwaveDirectory.pkg" \
  --package "x64/$CONFIG/LightwavePSCSiteManagement.pkg" \
  --package "x64/$CONFIG/LightwaveSSO.pkg" \
  --package "x64/$CONFIG/LightwaveDirectorySchema.pkg" \
  "x64/$CONFIG/Distribution.xml"

#generate installer
productbuild \
  --distribution "x64/$CONFIG/Distribution.xml" \
  --package-path "x64/$CONFIG/"  \
  "x64/$CONFIG/LightwaveToolsInstaller.pkg"
