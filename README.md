xinu
====

Xinu ("Xinu Is Not Unix", a recursive acronym) is a Unix-like operating system originally developed by Douglas Comer for instructional purposes at Purdue University in the 1980s. The name is both recursive, and is "Unix" spelled backwards.

Overview
========
This repository is a download of the original x86 archive from The XINU Page at http://www.xinu.cs.purdue.edu.  The plan is to

1. Enhance this codebase with a more standard build system
2. Provide instructions on setting up a cross compiler
3. Provide instructions on how to run XINU under a system like Bochs or QEMU

Description
===========
This directory contains a modifiction of the Linksys code that has
been ported to the x86 and a 3com Ethernet driver added.  In this
version, the interrupt mechanism has been replaced by one that uses
the Interrupt Flag in the processor FLAGS register (i.e., the bit
set with cli/sti).  It still maintains a global interrupt mask for
the controller that specifies which devices are allowed to interrupt,
but it only sets the interrupt mask when a new interrupt handler is
added (e.g., at device initialization time).

The Ethernet driver includes multicast, but it is not yet integrated
with ethControl (i.e., one has to call eth_mcadd separately.
