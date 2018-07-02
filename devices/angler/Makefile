DEVICE := angler

all:
	cp -L /bin/busybox initramfs/sbin/
	for a in $$(initramfs/sbin/busybox --list-full); do \
		ln -s /sbin/busybox initramfs/$$a; \
		done

clean:
	rm initramfs/sbin/busybox
	find initramfs/ -type l -delete

install:
	mkdir -p /usr/lib/preinit/$(DEVICE)/
	cp -Rv initramfs/ /usr/lib/preinit/$(DEVICE)/
