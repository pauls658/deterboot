# DeterBoot
This repository is a customized branch of the Syslinux tree that implements the Deter PXE bootloader. This is the stage-0 bootloader for the testbed. Every testbed node boots directly into this bootloader each time it boots. It is the responsibility of deterboot to engage in the bootinfo protocol with boss to determine what actually needs to be booted.

The code that implements the bootinfo protocol and performs subsequent stage booting is encapsulated in a Syslinux module. That code is in com32/deterboot.
