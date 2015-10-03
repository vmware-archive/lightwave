## Enable SSH authentication

Users can log in to Photon with ssh by providing their Lightwave credentials.  In order to enable this capability, run the following commands on each Photon instance:

```
/opt/likewise/bin/domainjoin-cli configure --enable pam
/opt/likewise/bin/domainjoin-cli configure --enable nsswitch
/opt/likewise/bin/lwregshell set_value '[HKEY_THIS_MACHINE\Services\lsass\Parameters\Providers]' LoadOrder "ActiveDirectory" "VmDir" "Local"
/opt/likewise/bin/lwsm restart lsass
```

To connect from another system, specify a Lightwave account when logging into Photon:

```
ssh -l <Lightwave account>@<Lightwave domain name> <Photon host>
```
For example:
```
ssh -l administrator@lightwave.local 192.168.237.202
```

If Lightwave users require sudo access to run privileged commands, grant by using the following approach.  Note the double backslash.

```
echo 'lightwave.local\\Administrator ALL=(ALL) NOPASSWD: ALL' > /etc/sudoers.d/lightwave-administrator
```


## Disable SSH authentication

If necessary, SSH authentication can be un-configured by reversing the above commands:

```
/opt/likewise/bin/domainjoin-cli configure --disable pam
/opt/likewise/bin/domainjoin-cli configure --disable nsswitch
```
