# Building Virtual Box ready PhotonOS OVA

To create a PhotonOS ova that is compatible with virtual box you should be able to simply Requirement

    ./build.sh

The resulting ova is ``build/photon-ova-virtualbox.ova``.

If you would like to base your ova on a specific version of photon os you need to provide the url to the respective PhotonOS:

    export PHOTON_ISO_URL=http://photon.os/version.iso

## Prerequisites
The following tools must be installed and available on the ``$PATH``:
- ``packer``
- ``sha1sum``
- ``virtualbox``
- ``GNU tar``
