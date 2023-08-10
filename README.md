# Thoth

## Provenance Collection Tool for Experimental Workflow System


## Development Setup

## Dependencies

Build dependencies: 
`llvm bpftool linux-tools-common`

Runtime dependencies:
`libbpf`

### Build

To build project run:

`make all`

### Install

To install the Thoth daemon and cli:

`make install`

## Installation from packages

### Fedora

Tested on Fedora 37.

```
curl -1sLf 'https://dl.cloudsmith.io/public/camflow/camflow/cfg/setup/bash.rpm.sh' | sudo -E bash
sudo dnf -y install thoth
```


### Ubuntu

eBPF must be enabled in order to use the Thoth tool. We tested on Ubuntu 22.04.

```
curl -1sLf 'https://dl.cloudsmith.io/public/camflow/camflow/setup.deb.sh' | sudo -E bash
sudo apt-get install thoth
```

Note: double check that eBPF kernel flags are set and eBPF is listed in Linux Security Module.
To check the LSM modules, read the list from `/sys/kernel/security/lsm`.
If `bpf` does not appear in this list, you can add it at boot time using GRUB. Enter the GRUB menu and press `e` to edit the GRUB config.
Add `security=bpf` to the line that starts with `linux ...`.

### Usage

Start the Thoth daemon:

`sudo systemctl start thothd`

Note: the daemon uses syslog. You can check syslog to make sure the daemon has started correctly.

Once the daemon has started, add directories to track using: 

`thoth --track-dir <directory_name>`

The provenance log will be stored in the `/tmp/` directory.

To stop the Thoth daemon:

`sudo systemctl stop thothd`

Currently there is no mechanism to remove a directory from tracking. Tracked directories are not persisted after the daemon stops, so to remove and add a new directory just stop and restart the daemon and add the new directory :)
