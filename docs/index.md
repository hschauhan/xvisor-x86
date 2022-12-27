---
title: xvisor-x86
---
## Welcome to xvisor-x86

### Running Xvisor
#### Inside KVM
The instructions to run Xvisor inside can be found [here](running_on_nested_kvm.md).

#### Disassembling 16-bit SeaBIOS Code
```
objdump -D -mi386 -Maddr16,data16 out/rom16.o | less
```

