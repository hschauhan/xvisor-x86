## Running Xvisor on nesting KVM

Qemu/KVM don't have emulation of VT-x instructions. So, Xvisor can only be run on machines with nested KVM enabled.

The following command can be used to run Xvisor inside nesting KVM:
```
qemu-system-x86_64 --enable-kvm \
                   -cpu host,+vmx,+vmx-activity-hlt,+tsc-deadline,+vme,\
+vmx-activity-wait-sipi,+vmx-apicv-xapic,+vmx-apicv-x2apic,+vmx-apicv-vid,\
+vmx-apicv-register,+vmx-cr3-load-noexit,+vmx-cr3-store-noexit,\
+vmx-cr8-load-exit,+vmx-cr8-store-exit,+vmx-entry-ia32e-mode,\
+vmx-encls-exit,+vmx-desc-exit,+vmx-entry-load-efer,+vmx-entry-load-pat,\
+vmx-entry-load-perf-global-ctrl,+vmx-ept-1gb,+vmx-ept,\
+vmx-entry-noload-debugctl,+vmx-ept-2mb,+vmx-ept-execonly,+vmx-eptad,\
+vmx-eptp-switching,+vmx-exit-ack-intr,+vmx-exit-load-efer,+vmx-exit-load-pat,\
+vmx-exit-load-perf-global-ctrl,+vmx-exit-save-preemption-timer,\
+vmx-exit-save-pat,+vmx-exit-save-efer,+vmx-exit-nosave-debugctl,\
+vmx-flexpriority,+vmx-invept,+vmx-hlt-exit,+vmx-invept-all-context,\
+vmx-ins-outs,+vmx-invept-single-context,+vmx-intr-exit,\
+vmx-invept-single-context-noglobals,+vmx-invvpid-all-context,+vmx-invvpid,\
+vmx-invlpg-exit,+vmx-invvpid-single-addr,+vmx-io-bitmap,+vmx-io-exit,\
+vmx-monitor-exit,+vmx-mwait-exit,+vmx-mtf,+vmx-msr-bitmap,+vmx-movdr-exit,\
+vmx-nmi-exit,+vmx-page-walk-4,+vmx-pause-exit,+vmx-preemption-timer,\
+vmx-posted-intr,+vmx-pml,+vmx-rdpmc-exit,+vmx-rdtsc-exit,+vmx-store-lma,\
+vmx-shadow-vmcs,+vmx-secondary-ctls,+vmx-rdtscp-exit,+vmx-true-ctls,\
+vmx-tsc-offset,+vmx-unrestricted-guest,+vmx-vnmi,+vmx-vmwrite-vmexit-fields,\
+vmx-vmfunc,+vmx-vintr-pending,+vmx-vnmi-pending,+vmx-vpid,+vmx-wbinvd-exit,\
+x2apic,+tsc,+rdtscp,+msr,+apic \
                   -smp cpus=4,sockets=1,cores=4,threads=1 \
                   -machine q35 \
                   -serial stdio \
                   -boot c -m 2048 \
                   -hda mydisk.img \
                   -device virtio-net,netdev=net0 -netdev \
                   user,id=net0,net=192.168.1.0/24,hostfwd=tcp:127.0.0.1:1122-192.168.1.15:22 \
                   -nographic -monitor unix:qemu-monitor-socket,server,nowait
```

### Mounting QCow2 guest image on Linux host
sudo guestmount -a <path/to/qcow/disk> -i <mountpoint>

