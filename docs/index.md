---
title: Xvisor-X86
---
## Welcome to Xvisor-X86

### Running Xvisor
#### Inside KVM
The instructions to run Xvisor inside can be found [here](running_on_nested_kvm.md).

#### Disassembling 16-bit SeaBIOS Code
```
objdump -D -mi386 -Maddr16,data16 out/rom16.o | less
```

