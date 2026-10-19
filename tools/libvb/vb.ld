OUTPUT_FORMAT("elf32-v810", "elf32-v810", "elf32-v810")
OUTPUT_ARCH(v810)
ENTRY(_start)

__stack_size   = 0x1000;
PROVIDE(__min_rom_size = 0x400);

MEMORY {
    ram (!r) : ORIGIN = 0x05000000, LENGTH = 64K
    rom (rx) : ORIGIN = 0x07000000, LENGTH = 16M
}

__text_vma = ORIGIN(rom);
__data_vma = ORIGIN(ram);

__stack_end = ORIGIN(ram) + LENGTH(ram);

PROVIDE(isrGamePad       = default_handler);
PROVIDE(isrTimer         = default_handler);
PROVIDE(isrGamePak       = default_handler);
PROVIDE(isrCom           = default_handler);
PROVIDE(isrVIP           = default_handler);
PROVIDE(isrFpException   = default_handler);
PROVIDE(isrDivideByZero  = default_handler);
PROVIDE(isrIllegalOpcode = default_handler);
PROVIDE(isrLoTrap        = default_handler);
PROVIDE(isrHiTrap        = default_handler);
PROVIDE(isrAddressTrap   = default_handler);
PROVIDE(isrDuplexed      = default_handler);

SECTIONS
{
  .text __text_vma : {
    *(.text .text.*)
    *(.gnu.warning)
    *(.gnu.linkonce.t*)
  } >rom = 0xFF

  .rodata : {
    *(.rodata .rodata.*)
    *(.roda)
    *all.rodata*(*)
    *(.gnu.linkonce.r*)
  } >rom = 0xFF

  . = ALIGN(4);
  __data_lma = .;

  .data __data_vma : AT(__data_lma) {
    *(.data .data.*)
    *(.sdata .sdata.*)
    *(.gnu.linkonce.d*)
    . = ALIGN(4);
  } >ram = 0xFF

  __data_end = .;

  __gp = __data_vma + 0x8000;

  .sbss (NOLOAD) : {
    *(.sbss .sbss.*)
    *(.scommon)
  } >ram

  .bss (NOLOAD) : {
    *(.dynbss)
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4);
  } >ram

  __bss_end = .;

  /DISCARD/ : {
    *(.comment)
    *(.note .note.*)
    *(.eh_frame)
  }

  __rom_used = __data_lma + SIZEOF(.data) - ORIGIN(rom);
  __rom_size = MAX(1 << LOG2CEIL(__rom_used + 0x220), __min_rom_size);
  __rom_end  = ORIGIN(rom) + __rom_size;

  .rom_title 0x07FFFDE0: AT(__rom_end - 0x220) ALIGN(0) SUBALIGN(0) {
      KEEP (*(.rom_title))
      FILL (0x20202020) # pad with spaces
      ASSERT (. <= 0x07FFFDF4, "ROM header title must be 20 characters or fewer");
      . = 0x07FFFDF4;
  } >rom

  .rom_maker 0x07FFFDF9: AT(__rom_end - 0x207) ALIGN(0) SUBALIGN(0) {
      KEEP (*(.rom_maker))
      FILL (0x20202020) # pad with spaces
      ASSERT (. <= 0x07FFFDFB, "ROM header maker code must be 2 characters or fewer");
      . = 0x07FFFDFB;
  } >rom
  
  .rom_game 0x07FFFDFB: AT(__rom_end - 0x205) ALIGN(0) SUBALIGN(0) {
      KEEP (*(.rom_game))
      FILL (0x20202020) # pad with spaces
      ASSERT (. <= 0x07FFFDFF, "ROM header game code must be 4 characters or fewer");
      . = 0x07FFFDFF;
  } >rom
  
  .rom_version 0x07FFFDFF: AT(__rom_end - 0x201) ALIGN(0) SUBALIGN(0) {
      KEEP (*(.rom_version))
      #FILL (0x00000000) # pad with zeroes
      #ASSERT (. <= 0x07FFFE00, "ROM header version must be 1 byte");
      #. = 0x07FFFE00;
  } >rom

  .vectors 0x07FFFE00 : AT(__rom_end - 0x200) {
    KEEP (*(.handlers.isrGamePad))
    KEEP (*(.handlers.isrTimer))
    KEEP (*(.handlers.isrGamePak))
    KEEP (*(.handlers.isrCom))
    KEEP (*(.handlers.isrVIP))
    . = . + 0x110;
    KEEP (*(.handlers.isrFpException))
    . = . + 0x10;
    KEEP (*(.handlers.isrDivideByZero))
    KEEP (*(.handlers.isrIllegalOpcode))
    KEEP (*(.handlers.isrLoTrap))
    KEEP (*(.handlers.isrHiTrap))
    KEEP (*(.handlers.isrAddressTrap))
    KEEP (*(.handlers.isrDuplexed))
    . = . + 0x10;
    KEEP (*(.handlers._start))
  } >rom = 0x008a008a /* fill unset interrupts with "loop forever" */

  ASSERT(__bss_end <= __stack_end - __stack_size,
         "RAM overflow: .data/.bss leave no room for the stack")

  ASSERT(SIZEOF(.vectors) == 0x200,
         "a .handlers.* section name does not match crt0.s")
}
