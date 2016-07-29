#!/bin/bash
#  build.sh
#
#
#  Created by sumalathaa on 6/24/15.
#

# This script generates the pkg build and product archives for Lightwave Tools installers. 

# Generate required directories
mkdir x64/Installers
mkdir x64/Installers/Debug
mkdir x64/Installers/Release

#create lotus client libraries package. Remove this once VMCA has a GoBuild pkg component.
pkgbuild --identifier LightwaveLibraries --root MacClientLibraries --scripts scripts --install-location "/usr/lib" "x64/Installers/Debug/LikewiseClientLibraries.pkg"

# DEBUG BUILD
pkgbuild --component "x64/Debug/Lightwave Directory.app" --install-location "/Applications/LightwaveTools/" "x64/Installers/Debug/LightwaveDirectory.pkg"

pkgbuild --component "x64/Debug/LightwaveCA.app" --install-location "/Applications/LightwaveTools/" "x64/Installers/Debug/LightwaveCA.pkg"

pkgbuild --component "x64/Debug/LightwaveCertStore.app" --install-location "/Applications/LightwaveTools/" "x64/Installers/Debug/LightwaveCertStore.pkg"

pkgbuild --component "x64/Debug/tdnfv 2.app" --install-location "/Applications/LightwaveTools/" "x64/Installers/Debug/tdnfv2.pkg"

pkgbuild --component "x64/Debug/Lightwave SSO.app" --scripts scripts --install-location "/Applications/LightwaveTools/" "x64/Installers/Debug/Lightwave SSO-1.0.0.pkg"

#generate distribution xml
productbuild --synthesize --package "x64/Installers/Debug/LikewiseClientLibraries.pkg" --package "x64/Installers/Debug/LightwaveDirectory.pkg" --package "x64/Installers/Debug/LightwaveCA.pkg" --package "x64/Installers/Debug/LightwaveCertStore.pkg" --package "x64/Installers/Debug/tdnfv2.pkg" --package "x64/Installers/Debug/Lightwave SSO-1.0.0.pkg" "x64/Installers/Debug/Distribution.xml"

#generate installer
productbuild --distribution "x64/Installers/Debug/Distribution.xml" --package-path "x64/Installers/Debug/"  "x64/Installers/Debug/LightwaveToolsInstaller.pkg"

# RELEASE BUILD
pkgbuild --component "x64/Release/Lightwave Directory.app" --install-location "/Applications/LightwaveTools" "x64/Installers/Release/LightwaveDirectory.pkg"

pkgbuild --component "x64/Release/LightwaveCA.app" --install-location "/Applications/LightwaveTools" "x64/Installers/Release/LightwaveCA.pkg"

pkgbuild --component "x64/Release/LightwaveCertStore.app" --install-location "/Applications/LightwaveTools" "x64/Installers/Release/LightwaveCertStore.pkg"

pkgbuild --component "x64/Release/Lightwave SSO.app" --install-location "/Applications/LightwaveTools" "x64/Installers/Release/Lightwave SSO-1.0.0.pkg"

#generate distribution xml
productbuild --synthesize --package "x64/Installers/Release/LightwaveDirectory.pkg" --package "x64/Installers/Release/LightwaveCA.pkg" --package "x64/Installers/Release/LightwaveCertStore.pkg" --package "x64/Installers/Release/Lightwave SSO-1.0.0.pkg" "x64/Installers/Release/Distribution.xml"

#generate installer
productbuild --distribution "x64/Installers/Release/Distribution.xml" --package-path "x64/Installers/Release/"  "x64/Installers/Release/LightwaveToolsInstaller.pkg"

