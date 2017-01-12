#!/bin/bash -xe

show_usage=0

packerVM="$1"
outputVM="$2"
customModScript="$3"

OVA_NAME=`basename $outputVM`
TAR_DIR=`dirname $outputVM`

if [[ -z "${packerVM}" ]]; then
  echo "Error: Missing input OVA argument"
  show_usage=1
fi

if [[ -z "${outputVM}" ]]; then
  echo "Error: Missing output OVA argument"
  show_usage=1
fi

if [[ ${show_usage} -eq 1 ]]; then
  echo "Usage: toVmwareOva.sh <inputOVA> <outputOVA> [customModScript]"
  echo "Note: Just provide base names without extention (no .ova or .ovf)"
  exit 1
fi

ovftool --lax -o ${packerVM}.ova ${outputVM}.ovf

oldOvfSha=$(sha1sum ${outputVM}.ovf | awk '{ print $1 }')

# Overwrite VirtualBox related stuff with vmware related values
sed -i.bak 's/virtualbox-2.2/vmx-04 vmx-06 vmx-07 vmx-09/' ${outputVM}.ovf
sed -i.bak 's/<rasd:Caption>sataController0/<rasd:Caption>SCSIController/' ${outputVM}.ovf
sed -i.bak 's/<rasd:Description>SATA Controller/<rasd:Description>SCSI Controller/' ${outputVM}.ovf
sed -i.bak 's/<rasd:ElementName>sataController0/<rasd:ElementName>SCSIController/' ${outputVM}.ovf
sed -i.bak 's/<rasd:ResourceSubType>AHCI/<rasd:ResourceSubType>lsilogic/' ${outputVM}.ovf
sed -i.bak 's/<rasd:ResourceType>20</<rasd:ResourceType>6</' ${outputVM}.ovf

if [[ ! -z ${customModScript} ]]; then
  eval ${customModScript} ${packerVM} ${outputVM}
fi

python remove_vbox_hardware.py ${outputVM}.ovf

newOvfSha=$(sha1sum ${outputVM}.ovf | awk '{ print $1 }')
sed -i.bak "s/$oldOvfSha/$newOvfSha/" ${outputVM}.mf

pushd $TAR_DIR
tar cvf ${outputVM}.ova ${OVA_NAME}.ovf ${OVA_NAME}*.vmdk ${OVA_NAME}.mf
popd
