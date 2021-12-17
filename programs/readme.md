# Compiling programs for 6502-burken using vasm

Build vasm like this:

```
make CPU=6502 SYNTAX=oldstyle
```

To compile a 6502 assembly file (say `program.s`) into a binary format that can be loaded into 6502-burken, do:

```
./vasm -Fbin -dotdir program.s
```

This generates the file `a.out`. To check the contents, do:

```
hexdump -C  a.out
```
