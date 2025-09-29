#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2025 Erez Geva <ErezGeva2@gmail.com>
#
# TODO:
#  workaround of to enable 'ruby2_keywords' 'Multi-Arch'
#  so we can install 'ruby-dev' on multiple architectura
main()
{
 # Store folder
 local st_dir=ruby2_keywords
 # Package name
 local -r b_nm=ruby-ruby2-keywords
 # Full debian name
 local -r nm=${b_nm}_0.0.5-1_all.deb
 # Target package
 local -r tgt=pkgs/trixie/$nm

 if ! [[ -f $nm ]]; then
   wget -c http://ftp.de.debian.org/debian/pool/main/r/$b_nm/$nm
   rm -fR $st_dir
 fi
 if ! [[ -d $st_dir ]]; then
   echo "Unpack ..,"
   dpkg-deb -R $nm $st_dir
 fi
 local -r ctrl=$st_dir/DEBIAN/control
 local do_repack=false
 if ! [[ `grep Multi-Arch: $ctrl` ]]; then
   echo "Add Multi-Arch flag ..."
   sed -i '/Architecture:/ a Multi-Arch: foreign' $ctrl
   do_repack=true
 fi
 if $do_repack || ! [[ -f $tgt ]]; then
   dpkg-deb -b $st_dir $tgt
 fi
}
main "$@"
