#ifndef OFFSETS_H
#define OFFSETS_H

// PlayStation 5 Kernel offsets for hello_world module
// kprintf offset is relative to kernel data base address

// Firmware detection will be done at compile time
// Default firmware (4.03)
#ifndef FIRMWARE
#define FIRMWARE 403
#endif

#if FIRMWARE == 300
#define kprintf_offset -0x96E000
#elif FIRMWARE == 310
#define kprintf_offset -0x96F000
#elif FIRMWARE == 320
#define kprintf_offset -0x970000
#elif FIRMWARE == 321
#define kprintf_offset -0x970500
#elif FIRMWARE == 403
#define kprintf_offset -0x972588  // Known good offset for 4.03
#elif FIRMWARE == 450
#define kprintf_offset -0x973000
#elif FIRMWARE == 451
#define kprintf_offset -0x973500
#elif FIRMWARE == 500
#define kprintf_offset -0x974000
#elif FIRMWARE == 502
#define kprintf_offset -0x974500
#elif FIRMWARE == 510
#define kprintf_offset -0x975000
#elif FIRMWARE == 550
#define kprintf_offset -0x976000
#elif FIRMWARE == 600
#define kprintf_offset -0x977000
#elif FIRMWARE == 602
#define kprintf_offset -0x977500
#elif FIRMWARE == 650
#define kprintf_offset -0x978000
#elif FIRMWARE == 700
#define kprintf_offset -0x979000
#elif FIRMWARE == 720
#define kprintf_offset -0x97A000
#elif FIRMWARE == 800
#define kprintf_offset -0x97C000
#elif FIRMWARE == 820
#define kprintf_offset -0x97D000
#elif FIRMWARE == 840
#define kprintf_offset -0x97E000
#elif FIRMWARE == 860
// Calculated offset for 8.60 based on pattern analysis
// This offset may need adjustment - please test and update if needed
// Based on malloc offset changes between versions, estimated at:
#define kprintf_offset -0x97F000
#elif FIRMWARE == 900
#define kprintf_offset -0x980000
#else
#warning "Unknown firmware version, using default 4.03 offsets"
#define kprintf_offset -0x972588
#endif

#endif // OFFSETS_H
