# Steps

1. Execute `make` first. This will result in a `netlinkKernel.ko` and `netlinkUser` output among many others.
2. Insert kernel module by :$ `sudo insmod netlinkKernel.ko`
3. Run `sudo ./netlinkUser` to communicate with kernel module `netlinkKernel` and run `dmesg --follow` in another terminal to see  the debug messages
4. Remove module by : $ `sudo rmmod netlinkKernel`
5. Finally `make clean` to remove output files.