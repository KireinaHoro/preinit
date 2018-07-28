# Preinit

Preinit handles the early process to load Gentoo init.  The following elements make up preinit:

  * `bootimg.cfg` specifies structure of the device's boot.img and commandline options.
  * `initramfs` sets up mount points for the Gentoo root and loads the Gentoo init.

Ports to other devices are strongly welcomed!  Read the Preinit section in the [Porter's Guide](https://jsteward.moe/sharkbait-porters-guide.html) for more information.
