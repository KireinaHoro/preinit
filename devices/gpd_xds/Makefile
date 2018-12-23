all:
	mkdir -p initramfs/{,s}bin
	cp -L /bin/busybox initramfs/sbin/
	for a in $$(initramfs/sbin/busybox --list-full); do \
		ln -sf /sbin/busybox initramfs/$$a; \
		done
	chmod 755 initramfs/init

clean:
	rm -rf initramfs/{,s}bin
