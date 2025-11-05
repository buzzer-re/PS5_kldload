# Hello World Kernel Module for PS5 Firmware 8.60

This is an updated version of the hello_world kernel module with multi-firmware support, specifically including PS5 firmware version 8.60.

## What's New

- **Multi-firmware support**: The `offsets.h` file now supports multiple PS5 firmware versions from 3.00 to 9.00
- **Firmware 8.60 support**: Added calculated kprintf offset for firmware 8.60
- **Compile-time firmware selection**: Use the `FIRMWARE` macro to build for specific firmware versions

## Building

### For Firmware 8.60

```bash
make clean
make FIRMWARE=860
```

### For Other Firmware Versions

```bash
# For firmware 4.03 (default)
make clean
make FIRMWARE=403

# For firmware 6.50
make clean
make FIRMWARE=650

# For firmware 7.00
make clean
make FIRMWARE=700

# For firmware 9.00
make clean
make FIRMWARE=900
```

## Usage

1. Ensure PS5_kldload is running on your PS5 (listening on port 9022)
2. Send the compiled binary to your PS5:

```bash
socat -t 99999999 - TCP:PS5_IP:9022 < hello_world.bin
```

3. Check the kernel logs (klogs) to see the output

## Expected Output

```
Hello from kernel land, let's check the MSR_LSTAR value!
MSR_LSTAR => 0x[address]
```

## Firmware 8.60 Notes

**IMPORTANT**: The kprintf offset for firmware 8.60 (`-0x97F000`) is an estimated value based on pattern analysis of offset changes between firmware versions.

If the module crashes or produces incorrect output:

1. The offset may need adjustment
2. You can find the correct offset by:
   - Analyzing the PS5 kernel binary for your firmware version
   - Checking updated offset databases from the PS5 homebrew community
   - Using kernel debugging tools like prosper0gdb

To update the offset, edit `include/firmware/offsets.h` and change the value for `FIRMWARE == 860`.

## Supported Firmware Versions

- 3.00, 3.10, 3.20, 3.21
- 4.03 (verified), 4.50, 4.51
- 5.00, 5.02, 5.10, 5.50
- 6.00, 6.02, 6.50
- 7.00, 7.20
- 8.00, 8.20, 8.40
- **8.60** (estimated offset - testing needed)
- 9.00

## Technical Details

### What This Module Does

1. Receives kernel data base address from the PS5_kldload loader
2. Calculates the kprintf function address using the firmware-specific offset
3. Prints a message to the kernel log
4. Reads and displays the MSR_LSTAR register value (syscall handler address)

### How kprintf Offset Works

The kprintf function in the PS5 FreeBSD kernel is located at a specific offset from the kernel data base address. This offset changes between firmware versions due to:

- Kernel code changes and additions
- Security patches
- Feature updates

The formula is:
```c
kprintf_address = kernel_data_base + kprintf_offset
```

## Troubleshooting

### Module doesn't produce output

1. Verify PS5_kldload is running and loaded correctly
2. Check that the firmware version matches what you compiled for
3. Try adjusting the kprintf_offset value

### Module crashes the system

1. The offset is likely incorrect for your firmware version
2. Reboot and try with a verified offset from the community
3. Consider using firmware version detection at runtime (future enhancement)

## Contributing

If you find the correct kprintf offset for firmware 8.60 or other versions:

1. Update `include/firmware/offsets.h` with the correct value
2. Mark it as verified in the comments
3. Submit a pull request or create an issue

## References

- [PS5_kldload](https://github.com/buzzer-re/PS5_kldload) - Original kernel module loader
- [kstuff](https://github.com/sleirsgoevy/ps4jb-payloads) - PS5 kernel research utilities
- [PS5 Developer Wiki](https://www.psdevwiki.com/ps5/) - Technical documentation

## License

This project inherits the license from the original PS5_kldload project.

## Disclaimer

This tool is intended for educational and research purposes on systems you own. Modifying system software may void warranties and violate terms of service.
