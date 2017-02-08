certool --version
certool --initcsr --privkey=cerTest.priv --pubkey=cerTest.pub --csrfile=cerTest.csr
certool --selfca
certool --getrootca
certool --genkey  --privkey=NewKey.priv --pubkey=NewKey.pub
certool --gencert --Name="TestRootCA" --privkey=NewKey.priv --cert=TestRootCA.cert
certool --viewcert --cert=TestRootCA.cert
certool --revokecert --cert=TestRootCA.cert
certool --viewcert --cert=TestRootCA.cert

rem generate 1000 certs
FOR /L %%G IN (1,1,1000) DO certool --gencert --Name="TestRootCA_%%G" --privkey=NewKey.priv --cert=TestRootCA_%%G.cert

rem revoke first 100 certs
FOR /L %%G IN (1,1,100) DO certool --revokecert --cert=TestRootCA_%%G.cert

rem enum all types of certs
certool --enumcert --filter=revoked > revoked.txt
certool --enumcert --filter=active > active.txt
certool --enumcert --filter=expired > expired.txt
certool --enumcert --filter=all > all.txt

rem get the count
findstr /i Serial revoked.txt | find /C "Serial"
findstr /i Serial active.txt | find /C "Serial"
findstr /i Serial expired.txt | find /C "Serial"
findstr /i Serial all.txt | find /C "Serial"
