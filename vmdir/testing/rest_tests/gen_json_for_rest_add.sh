#!/bin/sh

# Create a file which will be used to add new user through REST call
# Expects following arguments:
# ./gen_json_for_rest_add dn_prefix cn objectclass destination

if [ $# -ne 4 ]; then
   echo "./gen_json_for_rest_add <dn_prefix> <cn> <objectclass> <destination file>"
   exit 0
fi

echo "{" > $4
echo "  \"dn\": \"cn=$2,$1\"," >> $4
echo "  \"attrs\":" >> $4
echo "  {" >> $4
echo "    \"cn\": [\"$2\"]," >> $4
echo "    \"objectclass\": [\"$3\"]" >> $4
echo "  }" >> $4
echo "}" >> $4


