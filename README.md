## MI100 Firmware
This software is written in an Arduino sketch. It runs on an ATTiny1634 based simple two motors robot MI100.

## Motivation
MI100 is a simple wireless robot. It has a PIEZO, a full color LED, light sensor (optional) and two motors. 
MI100 is designed for helping children to enjoy studying programming language running on a laptop, Raspberry pi et cetra.

## Installation with Arduino IDE 1.5.7 (or later possibly)
Please refer to https://github.com/monoxit/monoxit-hardspec


## Installation with Arduino IDE 1.0.5

### Prerequisite:
* Arduino IDE 1.0.5 with the below environment
* Tiny core library from https://github.com/rambo/arduino-tiny/tree/attiny1634 in sketch book folder
* AVR_8_bit_GNU_Toolchain_3.4.3_1072 in arduino hardware>tools>avr folder
* An AVR writer

#### boards.txt section for ATTiny1634 on MI100

```
##########################################################################
#Original Idea From: https://github.com/rambo/arduino-tiny/blob/attiny1634/hardware/tiny/boards.txt
#

attiny1634at8bod2d7.name=ATTiny1634 @ 8 MHz (internal oscillator; BOD 2.7V)
attiny1634at8bod2d7.upload.using=arduino

attiny1634at8bod2d7.upload.maximum_size=16384
attiny1634at8bod2d7.upload.maximum_ram_size=1024

# Default clock (slowly rising power; long delay to clock; 8 MHz internal)
# Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms; [CKSEL=0010 SUT=0];
# Brown-out detection 2.7V; [BODLEVEL=101]
# Preserve EEPROM memory through the Chip Erase cycle; [EESAVE=0]
# BOD sample mode [BODACT=01; BODPD=01;]

attiny1634at8bod2d7.bootloader.low_fuses=0xE2
attiny1634at8bod2d7.bootloader.high_fuses=0xD5
attiny1634at8bod2d7.bootloader.extended_fuses=0xEA
attiny1634at8bod2d7.bootloader.path=empty
attiny1634at8bod2d7.bootloader.file=empty_all.hex

attiny1634at8bod2d7.build.mcu=attiny1634
attiny1634at8bod2d7.build.f_cpu=8000000L
attiny1634at8bod2d7.build.core=tiny
```

#### ATTiny1634 section in avrdude.conf in arduino hardware>tools>etc

```
#------------------------------------------------------------ 
# ATtiny1634. 
# From https://github.com/rambo/arduino-tiny/blob/attiny1634/avrdude/avrdude_tiny1634.conf
# MODIFICATION NOTICE:
# 2013.12.13 Masami Yamakawa Modify loadpage_lo, loadpage_hi and writepage
#            Original Idea for the modification: http://savannah.nongnu.org/bugs/?40144 submitted by Tobias Diedrich
#------------------------------------------------------------ 

part 
    id              = "t1634"; 
    desc            = "ATtiny1634"; 
     has_debugwire = yes; 
     flash_instr   = 0xB6, 0x01, 0x11; 
     eeprom_instr  = 0xBD, 0xF2, 0xBD, 0xE1, 0xBB, 0xCF, 0xB4, 0x00, 
                0xBE, 0x01, 0xB6, 0x01, 0xBC, 0x00, 0xBB, 0xBF, 
                0x99, 0xF9, 0xBB, 0xAF; 
    stk500_devcode  = 0x86; 
    # avr910_devcode = 0x; 
    signature       = 0x1e 0x94 0x12; 
    pagel           = 0xd7; 
    bs2             = 0xc2; 
    chip_erase_delay = 9000; 
    pgm_enable       = "1 0 1 0 1 1 0 0 0 1 0 1 0 0 1 1", 
                       "x x x x x x x x x x x x x x x x"; 

    chip_erase       = "1 0 1 0 1 1 0 0 1 0 0 x x x x x", 
                       "x x x x x x x x x x x x x x x x"; 

    timeout         = 200; 
    stabdelay       = 100; 
    cmdexedelay     = 25; 
    synchloops      = 32; 
    bytedelay       = 0; 
    pollindex       = 3; 
    pollvalue       = 0x53; 
    predelay        = 1; 
    postdelay       = 1; 
    pollmethod      = 1; 

    pp_controlstack     = 
        0x0E, 0x1E, 0x0E, 0x1E, 0x2E, 0x3E, 0x2E, 0x3E,
        0x4E, 0x5E, 0x4E, 0x5E, 0x6E, 0x7E, 0x6E, 0x7E,
        0x26, 0x36, 0x66, 0x76, 0x2A, 0x3A, 0x6A, 0x7A,
        0x2E, 0xFD, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00;
    hventerstabdelay    = 100; 
    progmodedelay       = 0; 
    latchcycles         = 5; 
    togglevtg           = 1; 
    poweroffdelay       = 15; 
    resetdelayms        = 1; 
    resetdelayus        = 0; 
    hvleavestabdelay    = 15; 
    resetdelay          = 15; 
    chiperasepulsewidth = 0; 
    chiperasepolltimeout = 10; 
    programfusepulsewidth = 0; 
    programfusepolltimeout = 5; 
    programlockpulsewidth = 0; 
    programlockpolltimeout = 5; 

    memory "eeprom" 
        paged           = no; 
        page_size       = 4; 
        size            = 256; 
        min_write_delay = 3600; 
        max_write_delay = 3600; 
        readback_p1     = 0xff; 
        readback_p2     = 0xff; 
        read            = " 1 0 1 0 0 0 0 0", 
                          " 0 0 0 x x x x a8", 
                          " a7 a6 a5 a4 a3 a2 a1 a0", 
                          " o o o o o o o o"; 
    
        write           = " 1 1 0 0 0 0 0 0", 
                          " 0 0 0 x x x x a8", 
                          " a7 a6 a5 a4 a3 a2 a1 a0", 
                          " i i i i i i i i"; 

   loadpage_lo   = "  1   1   0   0      0   0   0   1", 
           "  0   0   0   0      0   0   0   0", 
           "  0   0   0   0      0   0  a1  a0", 
           "  i   i   i   i      i   i   i   i"; 

   writepage   = "  1   1   0   0      0   0   1   0", 
           "  0   0   x   x      x   x   x  a8", 
           " a7  a6  a5  a4     a3  a2   0   0", 
           "  x   x   x   x      x   x   x   x"; 

   mode      = 0x41; 
   delay      = 5; 
   blocksize   = 4; 
   readsize   = 256; 
        ; 

    memory "flash" 
        paged           = yes; 
        size            = 16384; 
        page_size       = 32; 
        num_pages       = 512; 
        min_write_delay = 4500; 
        max_write_delay = 4500; 
        readback_p1     = 0xff; 
        readback_p2     = 0xff; 
        read_lo         = " 0 0 1 0 0 0 0 0", 
                          " 0 0 0 a12 a11 a10 a9 a8", 
                          " a7 a6 a5 a4 a3 a2 a1 a0", 
                          " o o o o o o o o"; 
        
        read_hi          = " 0 0 1 0 1 0 0 0", 
                           " 0 0 0 a12 a11 a10 a9 a8", 
                           " a7 a6 a5 a4 a3 a2 a1 a0", 
                           " o o o o o o o o"; 
        
        loadpage_lo     = " 0 1 0 0 0 0 0 0", 
                          " 0 0 0 x x x x x", 
                          " x x x x a3 a2 a1 a0", 
                          " i i i i i i i i"; 
        
        loadpage_hi     = " 0 1 0 0 1 0 0 0", 
                          " 0 0 0 x x x x x", 
                          " x x x x a3 a2 a1 a0", 
                          " i i i i i i i i"; 
        
        writepage       = " 0 1 0 0 1 1 0 0", 
                          " 0 0 0 a12 a11 a10 a9 a8", 
                          " a7 a6 a5 a4 x x x x", 
                          " x x x x x x x x"; 

        mode        = 0x41; 
        delay       = 6; 
        blocksize   = 128; 
        readsize    = 256; 

        ; 
        
    memory "lfuse" 
        size            = 1; 
        min_write_delay = 4500; 
        max_write_delay = 4500; 
        read            = "0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0", 
                          "x x x x x x x x o o o o o o o o"; 
        
        write           = "1 0 1 0 1 1 0 0 1 0 1 0 0 0 0 0", 
                          "x x x x x x x x i i i i i i i i"; 
        ; 
    
    memory "hfuse" 
        size            = 1; 
        min_write_delay = 4500; 
        max_write_delay = 4500; 
        read            = "0 1 0 1 1 0 0 0 0 0 0 0 1 0 0 0", 
                          "x x x x x x x x o o o o o o o o"; 
        
        write           = "1 0 1 0 1 1 0 0 1 0 1 0 1 0 0 0", 
                          "x x x x x x x x i i i i i i i i"; 
        ; 
    
    memory "efuse" 
        size            = 1; 
        min_write_delay = 4500; 
        max_write_delay = 4500; 
        read            = "0 1 0 1 0 0 0 0 0 0 0 0 1 0 0 0", 
                          "x x x x x x x x o o o o o o o o"; 
        
        write           = "1 0 1 0 1 1 0 0 1 0 1 0 0 1 0 0", 
                          "x x x x x x x x i i i i i i i i"; 
        ; 
    
    memory "lock" 
        size            = 1; 
        min_write_delay = 4500; 
        max_write_delay = 4500; 
        read            = "0 1 0 1 1 0 0 0 0 0 0 0 0 0 0 0", 
                          "x x x x x x x x x x o o o o o o"; 
        
        write           = "1 0 1 0 1 1 0 0 1 1 1 x x x x x", 
                          "x x x x x x x x 1 1 i i i i i i"; 
        ; 
    
    memory "calibration" 
        size            = 1; 
        read            = "0 0 1 1 1 0 0 0 0 0 0 x x x x x", 
                          "0 0 0 0 0 0 0 0 o o o o o o o o"; 
        ; 
    
    memory "signature" 
        size            = 3; 
        read            = "0 0 1 1 0 0 0 0 0 0 0 x x x x x", 
                          "x x x x x x a1 a0 o o o o o o o o"; 
        ; 
;
```
### Write the sketch
1. Connect an AVR Writer (ISP) to MI100 PCB 6 pin ISP connector.
2. (First time only) From Arduino IDE, write bootloader. It actually writes fuse bits. The bootloader is not written. emply_all.hex defined as a bootloader in boards.txt does nothing.
3. From Arduino IDE, write the sketch.

## Commands

Firmware gets simple commands from host PC through UART (e.g. through bluetooth SPP module). E.g. "F,500" move forwared 500ms. Error recovery is not implemented.

### Format:
`Command,Arguments,<LF>`

LF is line feed, carriage return or both.

### Command example:

`H` Return buttery power level. H,voltage `H,2.82`

`P` Return photo sensor level. (Require photo sensor option) P,raw_ADC_value `P,320`

`L,duration(ms)` Left pivot. L,500 Left pivot 500ms.

`R,duration(ms)` Right pivot. R,500 Right pivot 500ms.

`F,duration(ms)` Move forward. F,500 Move forward 500ms.

`B,duration(ms)` Move backward. B,500 Move backward 500ms.

`D,red_duty(%),green_duty(%),blue_duty(%),duration(ms)` Blink full color LED with RGB duty ratio.

`T,tone(Hz),duration(ms)` Beep PIEZO with tone Hz and duration. T,440,500 Beep 440Hz tone for 500ms.


## Firmware revision indication

MI100 has a full color LED. During startup, MI100 indicate firmware revision by LED.

* Rev 2: First release: R-G-B
* Rev 3: Photo sensor support: R-G-B-R-G-B
* Rev 4: Fix blue LED never lit after 75min from startup: R-G-B-B-G-R
* Rev 6: Add motor speed control: G-R-R-G (0110 = 6)

## License

LGPLv2.1 or later (same as a linked library license).
