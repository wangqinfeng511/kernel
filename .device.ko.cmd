cmd_/home/wangqinfeng/wendu_device/device.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000 -T ./scripts/module-common.lds  --build-id  -o /home/wangqinfeng/wendu_device/device.ko /home/wangqinfeng/wendu_device/device.o /home/wangqinfeng/wendu_device/device.mod.o ;  true