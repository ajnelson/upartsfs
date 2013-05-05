UPartsFS: Partition Table as a Userspace File System
====================================================

Some forensic utilities focus on analyzing an individual file system.  However, these utilities do not always include a parser for partition tables.  Thus, their use requires first parsing the partition table of a disk image with unrelated-utility-X, and then copying partitions to individual files.  (Or, in Unix/Linux OS's, the tool would need to be invoked on the block device file under `/dev` for a plugged-in disk.  To my knowledge, this is not an option with disk image files.)

This utility's goal is to present the partitions of a disk image as (very big, virtual) read-only files, using a userspace file system.  Ultimately, it saves a copy step that is expensive in time, storage, and programmer momentum.

The plan is to modify a simple FUSE example file system to use The Sleuth Kit's volume-management code to present partitions as files.

The interface should look like this, for a disk image with a partition at 63 sectors, and another at 20GiB:

    $ upartsfs disk_image mount_point
    $ find mount_point | sort
    mount_point/by_offset/21474836480
    mount_point/by_offset/32256
    mount_point/in_order/0
    mount_point/in_order/1
    $ umount mount_point


Related projects
----------------

From poking around on Github, I found these projects do pretty much the same thing, though with some underlying technical decisions I'd like to make differently:

* https://github.com/andreax79/partsfs - Basically the project I want to write, except it's a Linux kernel module, and uses its own partition table parsers.  I'd like FUSE for use in other OS's.
* http://code.google.com/p/libewf/wiki/Mounting - Oh. `ewfmount` appears to do the job, albeit without TSK's partition table parsers.  That would be a strange compilation order, since TSK optionally uses libewf libraries in its build.


Project status
--------------

This was to be a weekend hacking project.  However, I appear to have had a search engine oversight, and found pretty much this exact project already implemented: `ewfmount`.  Next up is testing ewfmount to see if the partition files it presents are directly usable with forensic utilities, or if those are also copy-conveniences.

Other help/pointers/noticed-impending-gotcha's welcome.
