/* The default linker script: the board's memory map, then the standard sections. An application
 * needing a section of its own copies this file and inserts it between the INCLUDEs, e.g.
 *
 *     INCLUDE memory.ld
 *     SECTIONS {
 *         INCLUDE sections_head.ld
 *         .my_special_data : { *(.my_special_data) } >RAM
 *         INCLUDE sections_data.ld
 *         INCLUDE sections_tail.ld
 *     }
 *
 * The fragments are found through the linker search path: link with -L<this directory>. */

INCLUDE memory.ld

SECTIONS
{
  INCLUDE sections_head.ld
  INCLUDE sections_data.ld
  INCLUDE sections_tail.ld
}
