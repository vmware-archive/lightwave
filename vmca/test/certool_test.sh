clear
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspaces/lotus/main/gobuild/compcache/boost1470_lin64_gcc412/ob-764487/linux64/lib
export PATH=$PATH:../build/certool
pwd
#certool = '../build/certool/certool'
echo "Starting Certool Test"
echo "Testing Help Commands"
certool --help || echo "certool --help FAILED"
certool --help init || echo "certool --help init FAILED"
certool --help functions || echo "certool --help functions FAILED"
certool --help config || echo "certool --help config FAILED"
certool --help files || echo "certool --help files FAILED"

#echo "Testing Init Functions"
certool --initcsr --privkey=cerTest.priv --pubkey=cerTest.pub || echo "certool --initcsr --privkey=cerTest.priv --pubkey=cerTest.pub FAILED"
certool --selfca || echo "certool --selfca FAILED"
certool --getrootca || echo "certool --rootca FAILED"
certool --genkey --privkey test.priv --pubkey test.pub || echo "--genkey failed"
echo "enter a value "
read $k

# now that we know that CA is setup to do that
#cert generation, let us create some.
echo $BASH_VERSION

for i in $(seq 1 1 1000)
do
	name="testcert$i"

	certool --gencert --Name $name --cert $name --privkey test.priv --config VMCAUnitTest.cfg
done


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
