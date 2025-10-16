# g1

## Summary

g1 is a minimal virtual machine that runs basic graphical applications.

The virtual machine is designed around ease-of-implementation--that is, the process of creating an implementation from the ground up should be as frictionless as possible.

To this end, the g1 ISA consists of 18 instructions which operate directly on a single statically allocated block of memory. There are no registers or data types; the only type that exists is the signed 32 bit integer.

g1 programs will always include an arbitrarily sized, 24-bit color window as well as 8 configurable audio channels.

## Table of Contents
- [g1](#g1)
  - [Summary](#summary)
  - [Table of Contents](#table-of-contents)
  - [Program Metadata](#program-metadata)
  - [Program Memory](#program-memory)
    - [Reserved Memory](#reserved-memory)
  - [Instructions and Labels](#instructions-and-labels)
    - [Instruction Arguments](#instruction-arguments)
    - [Instruction List](#instruction-list)
      - [Memory](#memory)
      - [Arithmetic](#arithmetic)
      - [Logic](#logic)
      - [Control Flow](#control-flow)
      - [Graphics](#graphics)
      - [Audio](#audio)
  - [Code Entrypoints](#code-entrypoints)
  - [I/O](#io)
    - [Input](#input)
    - [Video Output](#video-output)
      - [Color](#color)
      - [Point](#point)
      - [Line](#line)
      - [Rect](#rect)
    - [Audio Output](#audio-output)
  - [Data Format](#data-format)
    - [Data Type](#data-type)
    - [Operation](#operation)



## Program Metadata

Program metadata values may be set at the top of the file with the following syntax:

```g1
#[name] [value]
```

Here's the list of metadata names:
- `width` - The width of the output window in pixels. (default: `100`)
- `height` - The height of the output window in pixels. (default: `100`)
- `memory` - The number of memory slots to allocate for the program. (default: `128`, min: `32`)
- `tickrate` - The rate at which the `tick` label will be called. (default: `60`)


## Program Memory

When a g1 program is run, the virtual machine allocates an array of `n` signed 32 bit integers, where `n` is the value of the `#memory` meta variable.

Memory addresses are written in base 10 and are denoted with dollar signs (`$`).

### Reserved Memory

- `$0-8`: Input button states (`0`=not pressed, `1`=pressed)
  - `$0`: CONTROL1
  - `$1`: CONTROL2
  - `$2`: A
  - `$3`: B
  - `$4`: UP
  - `$5`: DOWN
  - `$6`: LEFT
  - `$7`: RIGHT
- `$8`: Program memory size (`#memory`) (constant)
- `$9`: Window width (`#width`) (constant)
- `$10`: Window height (`#height`) (constant)
- `$11`: Program tick rate (`#tickrate`) (constant)
- `$12`: Frame delta time (in milliseconds)

Each of these values are updated every tick.

> The VM implementer may decide at their own descretion which of their platform's inputs should be mapped to each of the input buttons.


## Instructions and Labels

Following any metadata declarations, the remainder of the program consists of a list of instructions and labels. An instruction consists of a name and a list of arguments:

```g1
[instruction name] [arg1] [arg2] ...
```

And a label consists of an identifier followed by a colon:
```g1
my_label:
    ; More instructions here
```

### Instruction Arguments

An argument can be an integer literal, a memory address, or a label name.

If an argument is a literal, then the instruction will operate on that literal value:

```g1
mov 20 42  ; Move the value 42 into address 20
```

If an argument is an address, then the instruction will operate on the value at that address.
```
mov 20 42   ; Move the value 42 into address 20
mov 21 $20  ; Move the value at 20 into address 21
            ; Now $20 and $21 both contain 42.
```

If an argument is a label, then the instruction will operate on the index where the label is located:
```
log 42          ; Print out 42
my_label:       ; Declare a label at index 1
log 43          ; Print out 43
jmp my_label 1  ; Jump to the index of `my_label`

```

### Instruction List

#### Memory
- `mov` `[dest address]` `[src]`
  - Copies value from `src` to `dest address`.
- `movp` `[dest address]` `[src address]`
  - Gets the value at address `src address` and copies it to `dest address`.

#### Arithmetic
- `add` `[dest address]` `[a]` `[b]`
- `sub` `[dest address]` `[a]` `[b]`
- `mul` `[dest address]` `[a]` `[b]`
- `div` `[dest address]` `[a]` `[b]`
- `mod` `[dest address]` `[a]` `[b]`

#### Logic
- `less` `[dest address]` `[a]` `[b]`
- `equal` `[dest address]` `[a]` `[b]`
- `not` `[dest address]` `[value]`

#### Control Flow
- `jmp` `[label name | instruction index]` `[value]`
  - If `value` is nonzero, jump to the specified label or index.

#### Graphics
- `color` `[r]` `[g]` `[b]`
- `point` `[x]` `[y]`
- `line` `[x1]` `[y1]` `[x2]` `[y2]`
- `rect` `[x]` `[y]` `[width]` `[height]`
- `getp` `[dest address]` `[x]` `[y]`
  - Returns the color of the pixel at (`x`, `y`) as a packed integer.

#### Audio
- `setch` `[channel number]` `[waveform]` `[frequency]` `[volume]`

- Waveform IDs:
  - `0`: Square
  - `1`: Triangle
  - `2`: Sawtooth
  - `3`: Noise


## Code Entrypoints

g1 has two code entrypoints: `start` and `tick`. Each of these positions are denoted with a label with one of those names.

`start` is executed once when the program is first run.

`tick` is executed `tickrate` times per second.

Execution stops when the program counter reaches the end of the instruction list.


## I/O

### Input

User input can be received by querying the input memory addresses. (`$0` to `$7`):

```g1
; This program continuously prints 42 while the user is holding the 'A' button.
tick:
    not 16 $2           ; Store !'A' at address 16
    jmp skip_print $16  ; Skip print if 'A' is not pressed
        log 42          ; Print 42
    skip_print:         ; Skip label
```

### Video Output

The program may write to the output window using any of these 4 graphical instructions:
- `color` `[r]` `[g]` `[b]`
- `point` `[x]` `[y]`
- `line` `[x1]` `[y1]` `[x2]` `[y2]`
- `rect` `[x]` `[y]` `[width]` `[height]`

#### Color

The `color` instruction modifies the current draw color. Any subsequent calls to `point`, `line`, or `rect` will draw the corresponding shape using the new color.

This color will persist until it is changed again.

#### Point

Draws a single pixel at (`x`, `y`).

#### Line

Draws a one-pixel thick line from (`x1`, `y1`) to (`x2`, `y2`).

#### Rect

Draws a solid rectangle with a top left corner at (`x1`, `y1`) and a specified `width` and `height` in pixels.


### Audio Output

The program may output audio using the `setch` (set channel) instruction:

`setch` `[channel number]` `[waveform]` `[frequency]` `[volume]`
- Waveform IDs:
  - `0`: Square
  - `1`: Triangle
  - `2`: Sawtooth
  - `3`: Noise

`frequency` and `volume` accept values in the range `[-32768, 32767]`.

The following program plays an A note (440Hz) for 2 seconds:
```
#tickrate 60             ; Set tickrate to 60 ticks per second

start:
    mov 16 0             ; Set timer to 0
    setch 0 0 440 10000  ; Initialize channel 0 (Square wave, 440Hz, 10,000 Volume)
    jmp end 1            ; Exit

tick:
    add 16 $16 1         ; Increment timer

    equal 17 $16 120     ; Check if timer is equal to 120
    not 17 $17
    jmp skip $17         ; Skip if timer is not equal to 120
        setch 0 0 0 0    ; Reset channel 0
    skip:
end:
```

## Data Format

To include additional data alongside program code, a program can be assembled with a g1 data file, which contains a list of data entries and where each entry should be placed in memory:

Here is the format for a single entry:
```g1d
[memory address]: [data type] [operation] [data value]
```

### Data Type

`data type` determines how `data value` should be parsed into bytes.

Options:
- `file`
  - `data value` = A file path
- `bytes`
  - `data value` = A string of hex bytes
- `string`
  - `data value` = A utf-8 string literal

### Operation

`operation` determines how the bytes processed by `data type` should be converted into a list of signed 32 bit integers.

Options:
- `raw`
  - Converts each byte into an integer in the range [0, 255].
- `pack`
  - Packs every group of 4 bytes into a single integer.
- `img`
  - Tries to parse bytes as an image. If successful, the width and height of the image will be added to the list, followed by `width * height` integers consisting of each pixel's packed rgb values.