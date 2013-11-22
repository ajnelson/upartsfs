UPartsFS: Partition Table as a Userspace File System
====================================================

Some forensic utilities focus on analyzing an individual file system.  However, these utilities do not always include a parser for partition tables.  Thus, their use requires first parsing the partition table of a disk image with unrelated-utility-X, and then copying partitions to individual files.  (Or, in Unix/Linux OS's, the tool would need to be invoked on the block device file under `/dev` for a plugged-in disk.  To my knowledge, this is not an option with disk image files.)

This utility's goal is to present the partitions of a disk image as (very big, virtual) read-only files, using a userspace file system.  Ultimately, it saves a copy step that is expensive in time, storage, and programmer momentum.

The plan is to modify a simple FUSE example file system to use The Sleuth Kit's volume-management code to present partitions as files.

The interface should look like this, for a disk image with a partition at 63 sectors, and another at 20GiB:

    $ upartsfs mount_point disk_image 
    $ find mount_point | sort
    mount_point/by_offset/21474836480
    mount_point/by_offset/32256
    mount_point/in_order/0
    mount_point/in_order/1
    $ umount mount_point

(The `disk_image` parameter is reversed from the normal `mount` order because some disk images are passed to TSK as multiple files.)


Related projects
----------------

From poking around on Github, I found these projects do close to the same thing, though with some underlying technical decisions I'd like to make differently:

* https://github.com/andreax79/partsfs - Basically the project I want to write, except it's a Linux kernel module, and uses its own partition table parsers.  I'd like FUSE for use in other OS's.
* http://code.google.com/p/libewf/wiki/Mounting - `ewfmount` presents a whole "raw" disk image stored in E01 format, but does no interpretation of the image's contents.
* vdfuse - VirtualBox-based partition-presenting FUSE file system. Meets some of the output goals of this project, with an included VirtualBox format bridge.  From a documentation skim, it appears to be a merge of a virtual block device and a partition table interpreter.  It is unclear what version of the tool one would want to use, though.  [Here's one](https://github.com/jonathanxavier/vdfuse) that claims heavy hacking from the upstream version, and [a fork with bug fixes](https://github.com/SophosLabs/vdfuse).  I haven't inspected what the partition parser is, though I assume it's whatever the running kernel uses, as the real heavy lifting of this code would have to be a virtual block device.  According to the SophosLabs README, it appears to balk at multi-thread access to a partition or the whole disk -- an artifact of VirtualBox's API?


Project status
--------------

This is a weekend hacking project.  If I have had a search engine oversight, and somebody finds pretty much this exact project already implemented, that would be good to hear.

Other help/pointers/noticed-impending-gotcha's welcome.
