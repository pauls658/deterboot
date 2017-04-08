###############################################################################
#
# make distribution tarballs for bios, efi32 & efi64
#
###############################################################################

#
# bios
#
rm -f bios.tgz
mkdir -p tmp/bios
find bios -name *.c32 -exec cp "{}" tmp/bios/ \;
find bios -name lpxelinux.0 -exec cp "{}" tmp/bios/ \;
cd tmp
tar czf bios.tgz bios;
cd ..
mv tmp/bios.tgz .
rm -rf tmp

#
# efi32
#

# ~~~ TODO ~~~

#
# efi64
#

# ~~~ TODO ~~~

