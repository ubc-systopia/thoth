# bpf-track

## Provenance Collection Tool for Experimental Workflow System


## Set-up

## Build

To build project:

`make all`

## Run

To run the provenance collection:

`sudo ./user`

## Installation from packages


### Fedora

```
curl -1sLf 'https://dl.cloudsmith.io/public/camflow/camflow/cfg/setup/bash.rpm.sh' | sudo -E bash
sudo dnf -y install thoth
```


### Ubuntu

```
curl -1sLf 'https://dl.cloudsmith.io/public/camflow/camflow/setup.deb.sh' | sudo -E bash
sudo apt-get install thoth
```

TODO instruction to update grub.
