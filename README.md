# Message-Slot-Kernel-Module-OS-Assignment
### OS Course Assignment - Character device file (driver) that provides a new IPC mechanism
The driver will be able to maintain multiple message channels active concurrently, which can be used by
multiple processes for communication.
message channel preserves a message until it is overwritten, so the same message can be read multiple times.

The driver will hold up to 256 message slot files which serve as communication channels, each corresponding to different message channels where each one preserves some message.
All of these files are managed by the driver, i.e., they all have the same major number, which
is hard coded to 240. However, different message slot files will have different minor numbers,
allowing thr driver to distinguish between them.

To create device files, we use the ```mknod``` Linux command, which takes the message slot file name, file’s major and minor numbers as arguments, as seen in the example below:
```bash
mknod /dev/slot0 c 240 0
```

### The Project contains the following programs:
1. **message_slot.c**: A kernel module implementing the message slot IPC mechanism.
2. **message_sender.c**: A user space program to send a message.
>The programs recieves the following arguments:

>• argv[1]: message slot file path.

>• argv[2]: the target message channel id. Assume a non-negative integer.

>• argv[3]: the message to pass

3. **message_reader.c**: A user space program to read a message
>The programs receives the following arguments:

>• argv[1]: message slot file path.

>• argv[2]: the target message channel id. Assume a non-negative integer.








