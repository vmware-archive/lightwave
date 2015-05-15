import os
import sys
import subprocess
import re
import time
import fileinput
import getopt

def fixExportedData(exportedFile):
   newFile = "altered-" + exportedFile

   with open(newFile, "wt") as out:
      actualLine = ''
      for line in open(exportedFile):
         if not line.startswith(" "):
            # write previous (wrapped) line, except the nTSecurityDescriptor attribute line
            if (len(actualLine) > 0) and (not actualLine.startswith("nTSecurityDescriptor:")):
                  out.write(actualLine)

            if line.startswith("userPassword:"):
               out.write('passwordHashScheme: vmdird-digest\n')

            actualLine = line
         else:
            actualLine = actualLine.rstrip() + line.lstrip()

      # write last line
      if (len(actualLine) > 0):
         out.write(actualLine)

def main(argv):
   hostname= 'localhost'
   outputfile = ''
   try:
      opts, args = getopt.getopt(argv,"hH:o:",["hostname=","ofile="])
   except getopt.GetoptError:
      print 'data-export.py [-H <hostname>] -o <outputfile>'
      sys.exit(2)

   for opt, arg in opts:
      if opt == '-h':
         print 'data-export.py -H <hostname> -o <outputfile>'
         sys.exit()
      elif opt in ("-H", "--hostname"):
         hostname = arg
      elif opt in ("-o", "--ofile"):
         outputfile = arg

   if outputfile == '':
      print 'data-export.py [-H <hostname>] -o <outputfile>'
      sys.exit()

   print 'Host name is: ', hostname
   print 'Output file is: ', outputfile

# empty the output file.
   open(outputfile, 'w').close()

   try:
      onelevelSearch = '/opt/likewise/bin/ldapsearch -h %s -p 389 -x -D "cn=Administrator,cn=Users,dc=vsphere,dc=local" -w "vmware" -b "" -s onelevel "objectclass=*" dn > first-level-objects.ldif' % (hostname)
      print 'running %s' % onelevelSearch
      os.system(onelevelSearch)

      for line in fileinput.input(['first-level-objects.ldif']):
         tokens = re.split(': ', line)
         if (tokens[0] == "dn"):
            base = tokens[1][:-1]
            if base not in ("cn=DSE Root", "cn=schemacontext", "cn=config"):
               subtreeSearch = '/opt/likewise/bin/ldapsearch -h %s -p 389 -x -D "cn=Administrator,cn=Users,dc=vsphere,dc=local" -w "vmware" -b "%s" -s subtree "objectclass=*" "-" "*" >> %s' % (hostname, base, outputfile)
               print 'running %s' % subtreeSearch
               os.system(subtreeSearch)

      fixExportedData(outputfile)

   except Exception, e:
     print e
     print "Data export tool failed."
     sys.exit(1)

if __name__ == "__main__":
   main(sys.argv[1:])
