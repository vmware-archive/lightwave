#!/bin/bash

#Allow build config from cmd line
if [ -z "$1" ]; then
  BUILD_CONFIG="Debug"
else
  BUILD_CONFIG=$1
fi

INTEROP_FOLDER="tools/interop/lib64"
DIR="$PWD"

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
  chmod -R 777 ./*
  /Applications/Xamarin\ Studio.app/Contents/MacOS/mdtool build -c:"$2" -t:Clean
  chmod -R 777 ./*
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

# build interops
buildSolution ../../vmafd/dotnet/VMAFD.Client $BUILD_CONFIG
buildSolution ../../vmdir/dotnet/VMDIR.Client $BUILD_CONFIG
xbuild /p:Configuration=$BUILD_CONFIG ../../vmdir/interop/csharp/VmDirInterop/VmDirInterop/VmDirInterop.csproj

#copy interops
# xamarin build - psc vmdir

cd ..
cd ..
pwd

chmod -R 777 tools/*

mkdir -pv tools/interop/lib64
rm -rf tools/interop/lib64/*

cp vmdir/dotnet/VMDIR.Client/bin/$BUILD_CONFIG/VMDIR.Client.dll tools/interop/lib64
cp vmafd/dotnet/VMAFD.Client/bin/$BUILD_CONFIG/VMAFD.Client.dll tools/interop/lib64
cp vmdir/interop/csharp/VmDirInterop/VmDirInterop/bin/$BUILD_CONFIG/VmDirInterop.dll tools/interop/lib64
cp vmdir/dotnet/VMDIR.Client/bin/$BUILD_CONFIG/VMDIR.Client.dll.config tools/interop/lib64
cp vmafd/dotnet/VMAFD.Client/bin/$BUILD_CONFIG/VMAFD.Client.dll.config tools/interop/lib64
cp vmdir/interop/csharp/VmDirInterop/VmDirInterop/bin/$BUILD_CONFIG/VmDirInterop.dll.config tools/interop/lib64

cp tools/common/VMIdentity.CommonUtils/Brand_lw.config tools/common/VMIdentity.CommonUtils/VMIdentity.CommonUtils.dll.config
cd tools/mac
pwd

buildSolution VMCertStoreSnapIn $BUILD_CONFIG
buildSolution VMDirSnapIn $BUILD_CONFIG
buildSolution VMPSCHighAvailabilitySnapIn $BUILD_CONFIG
buildSolution VMRestSsoSnapIn $BUILD_CONFIG
buildSolution VMCASnapIn $BUILD_CONFIG
buildSolution VMDirSchemaEditorSnapIn $BUILD_CONFIG

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
if [ ! -d /opt/vmware/vmca ]; then
  ENV_ISSUES=1
  echo 'certificate client package is not installed.'
fi

#check for vmcertstore mac interop
if [ ! -d /opt/vmware/vmafd ]; then
  ENV_ISSUES=1
  echo 'afd client package is not installed.'
fi

#check for vmdir mac interop
if [ ! -d /opt/vmware/vmdir ]; then
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

