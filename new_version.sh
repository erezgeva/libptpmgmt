#!/bin/bash
main()
{
 local -r email=$(git config --get user.email)
 local -r cur_ver=$(dpkg-parsechangelog --show-field Version)
 local -r maj_ver=${cur_ver%.*} min_ver=${cur_ver##*.}
 local -ri next_min_ver=min_ver+1
 local -r next_ver=$maj_ver.$next_min_ver
 local -r hash=$(git blame debian/changelog  | head -n1 | sed 's/ .*//')
 local -a log
 mapfile -t log < <(git log $hash..HEAD | grep -v '^Author:.*' |\
  grep -v '^\s*$')
 sed -i "s/ver_min=.*/ver_min=$next_min_ver/" version
 local l add_line=0
 for l in "${log[@]}"; do
  if [[ $l =~ ^Date|^commit ]]; then
   if [ $add_line -eq 1 ]; then
    DEBEMAIL="$email" debchange -v $next_ver "==============="
   fi
   DEBEMAIL="$email" debchange -v $next_ver "** $l"
   add_line=0
  else
   DEBEMAIL="$email" debchange -v $next_ver "$l"
   add_line=1
  fi
 done
 DEBEMAIL="$email" debchange -r buster
 gvim debian/changelog
 gitk &
}
main
