# Message-Slot-Kernel-Module-OS-Assignment
OS Course Assignment -Character device file (driver) that provides a new IPC mechanism
A message slot device has multiple message channels active concurrently, which can be used by
multiple processes for communication.
message channel preserves a message until it is overwritten, so the same message can be read multiple times.

There can be several message slot files, which correspond to different message channels.
All of these files are managed by the driver, i.e., they all have the same major number, which
is hard-coded to 240. However, different message slot files will have different minor numbers,
allowing thr driver to distinguish between them.

In order to create device files,







