/*
 * libmaple linker script for "JTAG" builds.
 *
 * A "JTAG" build puts .text (and .rodata) in Flash, and
 * .data/.bss/heap (of course) in SRAM, but links starting at the
 * Flash and SRAM starting addresses (0x08000000 and 0x20000000
 * respectively). This will wipe out a Maple bootloader if there's one
 * on the board, so only use this if you know what you're doing.
 *
 * Of course, a "JTAG" build is perfectly usable for upload over SWD,
 * the system memory bootloader, etc. The name is just a historical
 * artifact.
 */

/*
 * This pulls in the appropriate MEMORY declaration from the right
 * subdirectory of stm32/mem/ (the environment must call ld with the
 * right include directory flags to make this happen). Boards can also
 * use this file to use any of libmaple's memory-related hooks (like
 * where the heap should live).
 */
MEMORY
{
  ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
  rom (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
}
/**
    we allocate 2 pages at the end of the flash for eeprom
*/
PROVIDE(eeprom_size = 4*1024);
PROVIDE(eeprom_begin = ORIGIN(rom) + LENGTH(rom)-eeprom_size);



/* Let common.inc handle the real work. */
INCLUDE common.inc
