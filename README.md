# sdb

Advanced Programming in the UNIX Environment - Homework 4

Simplified Scriptable Instruction Level Debugger

# Usage
```
./hw4 [-s script] [program]
```

# Command
```
- break/b {instruction-address}: add a break point
- cont/c: continue execution
- delete {break-point-id}: remove a break point
- disasm/d addr: disassemble instructions in a file or a memory region
- dump/x addr: dump memory content
- exit/q: terminate the debugger
- get/g reg: get a single value from a register
- getregs: show registers
- help/h: show this message
- list/l: list break points
- load {path/to/a/program}: load a program
- run/r: run the program
- vmmap/m: show memory layout
- set/s reg val: get a single value to a register
- si: step into instruction
- start: start the program and stop at the first instruction
```

# Script
Example:
```
load sample/hello64
start
b 0x4000c6
l
cont
set rip 0x4000b0
cont
delete 0
set rip 0x4000b0
cont
```
