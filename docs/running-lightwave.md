# Running Lightwave


## Instantiating a domain controller

Running in *Standalone* mode (this is the first replica in a new domain)

```
/opt/vmware/bin/ic-promote
```

Running in *Partner* mode (this is a new replica in an existing domain)

```
/opt/vmware/bin/ic-promote --partner <hostname or ip-address of partner instance>
```

> Notes:
> - The password specified for the domain administrator must be *at least* 8 characters, include an upper case letter, a lower case letter, a digit and a special character.
> - Make sure to assign a static ip address or a dhcp-address with a reservation to the system before promoting it to be a domain controller.

## Setting up a Lightwave Domain Client

The following packages are required to join the Photon system to the Lightwave Domain.

1. `vmware-directory-client-6.0.0-0.x86_64.rpm`
2. `vmware-afd-client-6.0.0-0.x86_64.rpm`
3. `vmware-afd-6.0.0-0.x86_64.rpm`
4. `vmware-ca-client-6.0.0-0.x86_64.rpm`
5. `vmware-ic-config-1.0.0-0.x86_64.rpm`

Alternately, you can install the `vmware-lightwave-clients-6.0.0-0.x86_64.rpm` which is a meta RPM with dependencies on all the above RPMs.

If using the YUM repositories for the pre-built binaries, install the Lightwave Domain Client using `tdnf install vmware-lightwave-clients`.

Joining a system to a Lightwave domain:
```
/opt/vmware/bin/ic-join --domain-controller <hostname or ip-address of domain controller>
```
