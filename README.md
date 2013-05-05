PTFS: Partition Table as a File System
======================================

Some forensic utilities focus on analyzing an individual file system.  However, these utilities do not always include a parser for partition tables.  Thus, their use requires first parsing the partition table of a disk image, and then copying partitions to individual files.  (Or, the tool would need to be invoked on the block device file under `/dev`.  To my knowledge, this is not an option with disk image files.)

This utility's goal is to present the partitions of a disk image as (very big, virtual) files, using a userspace file system.  Ultimately, it saves an expensive copy step.


Project status
--------------

This is a weekend hacking project.  Help/pointers/noticed-impending-gotcha's welcome. 

The plan is to modify a simple FUSE example file system to use The Sleuth Kit's volume-management code to present files.
