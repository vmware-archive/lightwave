import xml.etree.ElementTree as ET
import sys
import shutil

def main(argv):
    if argv == None or len(argv) < 1:
        print ('Invalid ovf path')
        return -1

    ova_path = argv[0]
    ns = {'ovf': 'http://schemas.dmtf.org/ovf/envelope/1',
          'rasd': 'http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData'}
    hardware_removed = False
    tree = ET.parse(ova_path)
    root = tree.getroot()

    # Remove sound card item if exists
    for atype in root.findall('ovf:VirtualSystem', ns):
        for sect in atype.findall('ovf:VirtualHardwareSection', ns):
            for item in sect:
                for value in item.findall('rasd:Description', ns):
                    if value.text.lower() == 'sound card':
                        hardware_removed = True
                        sect.remove(item)

    if hardware_removed:
        tree.write(ova_path, encoding='utf-8', xml_declaration=True)

if __name__ == '__main__':
    main(sys.argv[1:])
