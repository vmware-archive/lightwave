#!/bin/bash

#Allow build config from cmd line
if [ -z "$1" ]; then
  BUILD_CONFIG="Debug"
else
  BUILD_CONFIG=$1
fi

#check pre-requisites (Xcode, Xamarin Studio, Mono.framework, Xamarin.Mac.framework)
#check if Xcode is installed
if [ ! -d /Applications/Xcode.app ]; then
  echo 'Xcode is required to do a build. Please install Xcode 6.4 or later.'
  exit 1
fi

#check if Xamarin Studio is installed
if [ ! -d /Applications/Xamarin\ Studio.app ]; then
  echo 'Xamarin Studio is required to do a Xamarin.Mac build. Please install Xamarin Studio 5.9.4.5 or later.'
  exit 1
fi

#check if Mono.framework is installed
if [ ! -d /Library/Frameworks/Mono.framework ]; then
  echo 'Mono.framework is required. Please install Mono.framework version 4.0.3 or later.'
  exit 1
fi

#check if Xamarin.Mac.framework is installed
if [ ! -d /Library/Frameworks/Xamarin.Mac.framework ]; then
  echo 'Xamarin.Mac.framework is required. Please install Xamarin.Mac.framework version 2.0.2 or later.'
  exit 1
fi

buildSolution() {
  pushd $1
  /Applications/Xamarin\ Studio.app/Contents/MacOS/mdtool build -c:"$2" -t:Clean
  /Applications/Xamarin\ Studio.app/Contents/MacOS/mdtool build -c:"$2"

  #check result and exit if not successful
  if [ $? -ne 0 ];then
     echo "Failed building $1"
     exit 1
  fi

  popd
}
# pre-build
nuget restore "VMRestSsoSnapIn/Lightwave SSO.sln"

# build
buildSolution VMCASnapIn $BUILD_CONFIG
buildSolution VMCertStoreSnapIn $BUILD_CONFIG
buildSolution VMDirSnapIn $BUILD_CONFIG
buildSolution VMRestSsoSnapIn $BUILD_CONFIG
buildSolution VMPSCHighAvailabilitySnapIn $BUILD_CONFIG

echo ''
echo ''
echo 'Checking for environment issues...'
echo ''

ENV_ISSUES=0
#post build checks to ensure the apps can be run
#check for likewise-open
if [ ! -d /opt/likewise/bin ]; then
  ENV_ISSUES=1
  echo 'likewise-open mac package is not installed.'
fi

#check for vmca mac interop
if [ ! -d /usr/lib/vmware-vmca ]; then
  ENV_ISSUES=1
  echo 'certificate client package is not installed.'
fi

#check for vmcertstore mac interop
if [ ! -d /usr/lib/vmware-vmafd ]; then
  ENV_ISSUES=1
  echo 'afd client package is not installed.'
fi

#check for vmdir mac interop
if [ ! -d /usr/lib/vmware-vmdir ]; then
  ENV_ISSUES=1
  echo 'directory client package is not installed.'
fi


if [ $ENV_ISSUES -eq 1 ]; then
  echo ''
  echo 'WARNING!!'
  echo 'Some pre-requisites are missing.'
  echo 'Refer README file and install the pre-built binaries before running apps.'
else
  echo 'All pre-requisites are installed. No issues detected.'
fi
