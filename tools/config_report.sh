#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Show configuration summary
###############################################################################
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 [[ -f defs.mk ]] || retrurn
 local n var val
 mapfile < defs.mk
 for n in "${MAPFILE[@]}"; do
  if [[ $n =~ ^# ]]; then
    continue
  fi
  if [[ $n =~ (.*):=(.*) ]]; then
    var="${BASH_REMATCH[1]}"
    val="${BASH_REMATCH[2]/$'\n'/}"
    # remove leading whitespace characters
    val="${val#"${val%%[![:space:]]*}"}"
    # remove trailing whitespace characters
    val="${val%"${val##*[![:space:]]}"}"
    if [[ -n "$val" ]]; then
      local $var="$val"
    fi
  fi
 done
 local langs='tcl perl python ruby php lua go'
 local $langs
 local -A setLang
 setLang[tcl]="@'$TCLVER'"
 local -r p2=';@"${perl#v}"'
 setLang[perl]="@'$($PERL -e 'print "$^V"')'$p2"
 setLang[python]="@'${PY3VERSION#python}'"
 setLang[ruby]="@'$RUBYVER'"
 setLang[php]="@'$PHPVER'"
 setLang[lua]="[[ -n '$LUAVERSIONS' ]] && @'$LUAVERSIONS' || @'$LUA_VERSION'"
 setLang[go]="@'$GOVER'"
 for n in $langs; do
   local -n s="SKIP_${n^^}"
   [[ -n "$s" ]] && local $n='x' || eval "${setLang[$n]//@/local $n=}"
 done
 if [[ -n "$DOTTOOL" ]]; then
   local dver="$($DOTTOOL -V 2>&1)"
   dver="${dver#*version }"
   dver="${dver% (0)}"
 else
   local -r dver='x'
 fi
 local -r gccver=$(g++ -v 2>&1 | tail -1 | sed 's/.* version //;s/ .*//')
 [[ "$build" = "$host" ]] && local -r bon='native' || local -r bon='cross'
 [[ -n "$ASTYLEMINVER" ]] && local -r astyle="$ASTYLEMINVER" || local -r astyle='x'
 [[ -n "$HAVE_GTEST_HEADER" ]] && local -r gtest='v' || local -r gtest='x'
 [[ -n "$HAVE_JSONC_LIB" ]] && local -r jsonc='v' || local -r jsonc='x'
 [[ -n "$HAVE_FJSON_LIB" ]] && local -r fjson='v' || local -r fjson='x'
 [[ -n "$SWIGMINVER" ]] && local -r swig="$SWIGMINVER" || local -r swig='x'
 [[ -n "$DOXYGENMINVER" ]] && local -r doxy="$DOXYGENMINVER" || local -r doxy='x'
 cat << EOF
========================== Config ==========================
Version '$PACKAGE_VERSION' build $bon gcc '$gccver' astyle '$astyle'
Jsonc '$jsonc' Fjson '$fjson'
Doxygen '$doxy' dot '$dver' Google test '$gtest'
swig '$swig' Python '$python' Ruby '$ruby' PHP '$php'
Perl '$perl' go '$go' tcl '$tcl' Lua '$lua'
============================================================
EOF
}
main "$@"
