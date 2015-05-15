clear
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/zhiwen/workspaces/lotus/main/gobuild/compcache/boost1470_lin64_gcc412/ob-764487/linux64/lib
certool='../build/certool/certool --config=../config/certool.cfg'
echo "Starting Certool Test"
echo "using certool from " $certool
echo "Testing Help Commands"
$certool --help || echo "certool --help \e[0;31;47m FAILED \033[0m" 
$certool --help init || echo "certool --help init \e[0;31;47m FAILED \033[0m" 
$certool --help functions || echo "certool --help functions \e[0;31;47m FAILED \033[0m" 
$certool --help config || echo "certool --help config \e[0;31;47m FAILED \033[0m" 
$certool --help files || echo "certool --help files \e[0;31;47m FAILED \033[0m" 


echo "Testing Init Functions"
$certool --initcsr --privkey=cerTest.priv --pubkey=cerTest.pub --csrfile=cerTest.csr || echo "certool --initcsr --privkey=cerTest.priv --pubkey=cerTest.pub \e[0;31;47m FAILED \033[0m"

echo "Testing Selfca Functions"
$certool --selfca || echo "certool --selfca \e[0;31;47m FAILED \033[0m"

echo "Testing Get Root CA Functions"
$certool --getrootca || echo "certool --rootca \e[0;31;47m FAILED \033[0m"

# now that we know that CA is setup to do that
#cert generation, let us create some.
echo "Testing Gen Key Functions"
$certool --genkey  --privkey=NewKey.priv --pubkey=NewKey.pub || echo "certool --gencert --Name=TestRootCA.cert \e[0;31;47m FAILED \033[0m"

echo "Testing Gen Cert Functions"
$certool --gencert --Name="TestRootCA" --privkey=NewKey.priv --cert=TestRootCA.cert || echo "certool --gencert --Name=TestRootCA.cert \e[0;31;47m FAILED \033[0m"

echo "Testing Revoke Cert Functions"
$certool --revokecert --cert=TestRootCA.cert

echo "Testing View Cert Functions"
$certool --viewcert --cert=TestRootCA.cert

for i in $(seq 1 1 1000)
do
	name="testcert$i"
	certool --gencert --Name $name --cert $name --privkey NewKey.priv --config ../config/certool.cfg
done

for i in $(seq 1 1 100)
do
	name="testcert$i"
	certool  --revokecert --cert=$name
done

echo "Testing Enum Cert Functions"
$certool --enumcert --filter=revoked
$certool --enumcert --filter=active
$certool --enumcert --filter=expired
$certool --enumcert --filter=all
