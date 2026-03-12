#pragma once

#include <stdint.h>

struct _IDTR
{
    uint16_t  limit;      /* Size of IDT array - 1 */
    uint64_t  base;       /* Pointer to IDT array  */
} __attribute__((packed)) ;

typedef struct _IDTR IDTR;
typedef IDTR idtr;

struct _idt_64
{
    uint16_t offset_low;
    uint16_t selector;
    struct
    {
      uint16_t ist_index : 3; /* bit position: 0 */
      uint16_t reserved_0 : 5; /* bit position: 3 */
      uint16_t type : 5; /* bit position: 8 */
      uint16_t dpl : 2; /* bit position: 13 */
      uint16_t present : 1; /* bit position: 15 */
    }; /* bitfield */
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserverd_1;
} __attribute__((packed));

typedef struct _idt_64 idt_64;

#define UNPACK_HANDLER_ADDR(idt)  ( ((uint64_t) idt->offset_high << 32) | (idt->offset_middle << 16) | idt->offset_low)
