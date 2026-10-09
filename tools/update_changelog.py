#!/usr/bin/python3
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# update_changelog:
#   Copy changelog from debian to other distributions changelog
# update_doxygen:
#   Update doxygen configuration file
# update_goarch:
#   Create converting scripts from configure/GNU target architecture
#    to GOARCH using Debian dpkg-architecture tool.
#   See: https://pkg.go.dev/internal/goarch
#        https://gist.github.com/asukakenji/f15ba7e588ac42795f421b48b8aede63
#        https://wiki.debian.org/SupportedArchitectures
# format:
#   extra formatter, run after astyle
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2021 Erez Geva
#
###############################################################################

import re
import os
import sys
import fileinput

###############################################################################
# update_changelog.py

def do_update_changelog():
  debfile='debian/changelog'
  rpmfile='rpm/libptpmgmt.spec'
  arcfile='archlinux/changelog'
  if not os.path.isfile(debfile) or not os.path.isfile(rpmfile) or not os.path.isfile(arcfile):
    return
  os.system('./tools/new_version.sh -u > /dev/null')
  key = r'([A-Z][a-z][a-z])'
  empty_line = re.compile(r'^$')
  ver_match = re.compile(r'^libptpmgmt \(([^)]+)\)')
  last_line = re.compile(r'^ -- [^<]+<([^>]+)>  ' + key + r', (\d\d) '+key+r' (\d\d\d\d)')
  spaces2 = re.compile(r'^  ')
  star = re.compile(r'^\*')
  rpm_last = re.compile(r'^%changelog')
  rpm_lines = []
  for line in open(rpmfile, 'r'):
    rpm_lines.append(line)
    if rpm_last.search(line):
      break;
  rpm_out = open(rpmfile, 'w')
  for line in rpm_lines:
    rpm_out.write(line)
  arc_out = open(arcfile, 'w')
  keep = []
  first = False
  ver = ''
  for l in open(debfile, 'r'):
    line = l.rstrip()
    if empty_line.match(line):
      continue
    if s := ver_match.match(line):
      ver = s.group(1)
    elif s := last_line.match(line):
      if first:
        rpm_out.write('\n')
        arc_out.write('\n')
      a = '* {} {} {} {} {} {}-1\n'.format(s.group(2), s.group(4), s.group(3), s.group(5), s.group(1), ver)
      rpm_out.write(a)
      arc_out.write(a)
      for l1 in keep:
        rpm_out.write(l1 + '\n')
        arc_out.write(l1 + '\n')
      first=True
      keep.clear()
    else:
      line = spaces2.sub('', line)
      line = star.sub('-', line)
      keep.append(line)
  arc_out.close()
  rpm_out.close()

###############################################################################
# update_doxygen.py

def readOptions(key_reg, reg_key_match, cfg, options):
  with os.popen('doxygen -x ' + cfg) as f:
    for l in f:
      s = reg_key_match.search(l)
      if s:
        key, val = s.groups()
        options[key] = val
  return options

def update_doxygen_file(cfg):
  if not os.path.isfile(cfg):
    return
  # These options are always used
  options = {}
  options['CASE_SENSE_NAMES'] = 'YES' # Default is system dependent.
  # Regular expressions
  key_reg=r'[A-Z][A-Z0-9_]+'
  reg_key_match = re.compile(r'^(' + key_reg + r')\s*=\s*(.*)')
  reg_ckey_match = re.compile(r'^#' + key_reg + r'\s*=.*')
  cont_line_match = re.compile(r'\\$')
  comm_match = re.compile(r'^#')
  dcomm_match = re.compile(r'^##')
  space_match = re.compile(r'^ ')
  start_match = re.compile(r'^')
  empty_line = re.compile(r'^$')
  # Store current used options
  cur_options = readOptions(key_reg, reg_key_match, cfg, options)
  # Unmark all options, so the double comments retain thier place
  # Also catch any option that default value is changed on new version
  cont_comm = False # Continue strip comments on multiple lines option
  for l0 in fileinput.input(cfg, inplace=True):
    l = l0.rstrip()
    if comm_match.search(l):
      if cont_comm or reg_ckey_match.search(l):
        l = comm_match.sub('', l)
        cont_comm = bool(cont_line_match.search(l))
    else:
      cont_comm = False
    print(l)

  # Store current used options on new version
  options = readOptions(key_reg, reg_key_match, cfg, options)
 # DEBUG: See options
 #for k in options:
 #  print(k + '=' + options[k])
  # Update doxygen to new version
  os.system('doxygen -u ' + cfg + ' > /dev/null')
  # Move double comments before first option, to begin of file
  first_2comments = []
  for line in open(cfg, 'r'):
    if dcomm_match.search(line):
        first_2comments.append(line.rstrip())
    s = reg_key_match.match(line)
    if s:
        first_option = s.group(1)
        break
  escape_dcmt = True # Remove double comments before first option
  last_empty = False # Prevent sequance of empty lines
  cont_comm = False # Continue comment a multiple lines option
  for l0 in fileinput.input(cfg, inplace=True):
    l = l0.rstrip()
    # print double comments at the begin of file
    if len(first_2comments) > 0:
      for l1 in first_2comments:
        print(l1)
      first_2comments.clear()
    if dcomm_match.search(l):
      continue
    if empty_line.search(l):
      cont_comm = False
      if last_empty:
        continue
      last_empty = True
    else:
      last_empty = False
    #sys.stderr.write(l0) # DEBUG: print file
    s = reg_key_match.match(l)
    if s:
      cont_comm = False
      key = s.group(1)
      val = s.group(2)
      if key == first_option:
        escape_dcmt = False
      if not key in options:
        l = start_match.sub('#', l)
        cont_comm = bool(cont_line_match.search(l))
    elif cont_comm and space_match.search(l):
      l = start_match.sub('#', l)
      cont_comm = bool(cont_line_match.search(l))
    else:
      cont_comm = False
    print(l)
  # New options added due to default value changed on new version!
  for key in options:
    if not key in cur_options:
      print('New option: ' + key)
  if os.path.isfile(cfg + '.bak'):
    os.unlink(cfg + '.bak')

def do_update_doxygen():
  update_doxygen_file('tools/doxygen.cfg.in')
  update_doxygen_file('tools/doxygen.clkmgr.cfg.in')

###############################################################################
# update_goarch.py

def is_exe(file):
  return os.path.isfile(file) and os.access(file, os.X_OK)

def gnu(dpkg_arc, deb_arch):
  f = os.popen(dpkg_arc + ' -qDEB_TARGET_GNU_CPU -a' + deb_arch)
  return f.read().strip()

def case(goarch, _gnu):
  return "    {}) GOARCH='{}';;".format(_gnu, goarch)

def do_update_goarch():
  f=os.popen('which dpkg-architecture')
  dpkg_arc = f.read().strip()
  # Make sure we have the tool
  if not is_exe(dpkg_arc):
    return
  fname='tools/goarch.sh'
  f=open(fname,'w')
  # Header
  print('''\
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Convert GNU target to GOARCH
###############################################################################
main()
{
  [[ -n "$1" ]] || return
  local GOARCH
  case "$1" in''',file=f)
  # GO arch and Debian different
  print(case('386', gnu(dpkg_arc, 'i386')),file=f)
  archs=['amd64', 'arm', 'arm64', 'ppc64', 'mips', 'mips64', 'riscv64',
         's390', 's390x', 'sparc', 'sparc64']
  same=''
  s=''
  for a in archs:
    g = gnu(dpkg_arc, a)
    if g == a:
      same+=s + a
      s = '|'
    else:
      print(case(a, g),file=f)
  # For unkown architecture, return the GNU architecture, they might be equal.
  print('    ' + same + ') GOARCH="$1";;',file=f)
  print('''\
    *)
      if [[ -n "`which dpkg-architecture 2> /dev/null`" ]]; then
        GOARCH="`dpkg-architecture -qDEB_TARGET_GNU_CPU -a$1 2> /dev/null`"
      else
        GOARCH="$1"
      fi
      ;;
  esac
  printf "$GOARCH"
}
main "$@"''',file=f)
  f.close()
  os.chmod(fname, 0o755)

###############################################################################
# format.py

def err(file, lnum, line, msg):
  return 'Check: {}:{}: {}: |{}|'.format(file, lnum, msg, line)

def do_format():
  errVal = False
  empty_line = re.compile(r'^$')
  cpp_file = re.compile(r'\.(cpp|h|hpp)$')
  header_file = re.compile(r'\.h$')
  shell_file = re.compile(r'\.sh$')
  c_wrong_char = re.compile(r'''[^a-zA-Z0-9{}()<>©~"'?:@&;%!.,*#_^+=| \[\]\$\/\\-]''')
  c_wrong_escape = re.compile(r'''\\[^xntrbf0"'\\]''')
  astyle_skip = re.compile(r'\*(INDENT-ON|INDENT-OFF|NOPAD)\*', re.I)
  nullptr = re.compile(r'[^"_]NULL\b')
  # Ignore protocol '://' sign
  c_commemt  = re.compile(r'(^//|[^:]//)')
  sh_wrong_char = re.compile(r'''[^a-zA-Z0-9{}()<>©~"'?:@&;%!.,*#_^+=| \[\]\$\/\\`-]''')
  sh_wrong_escape = re.compile(r'\\[^0-9nes".()\[\]\$\/\\]')
  wrong_char = re.compile(r'''[^a-zA-Z0-9{}()<>©~"'?:@&;%!.,*#_^+=| \[\]\$\/\\\t`-]''')
  wrong_tabs = re.compile(r'[^\t]\t')
  for file in sys.argv[1:]:
    if os.path.isfile(file) and not os.path.islink(file):
      # As script removes empty lines only
      # We store times are restore them once we finish
      s = os.stat(file)
      times = (s.st_atime, s.st_mtime)
      # skip empty lines at start of
      not_first_empty_lines = False
      # combine empty lines into single empty line
      # and skip empty lines at end of file
      had_empty_lines = False
      lnum=0
      for l0 in fileinput.input(file, inplace=True):
        lnum += 1
        #######################################
        # Handle empty lines
        line = l0.rstrip()
        if empty_line.match(line):
          if not_first_empty_lines:
            had_empty_lines = True
          continue
        if had_empty_lines:
          print()
        not_first_empty_lines = True
        had_empty_lines = False
        #######################################
        # Verify we use proper characters!
        if cpp_file.search(file):
          if c_wrong_char.search(line):
            errVal = err(file, lnum, line, 'for wrong char')
          elif c_wrong_escape.search(line):
            errVal = err(file, lnum, line, 'wrong escape char')
          if astyle_skip.search(line):
            errVal = err(file, lnum, line, 'for using astyle skip')
          if nullptr.search(line):
            errVal = err(file, lnum, line, 'use C++ nullptr')
          #######################################
          # proper comments in headers
          if header_file.search(file):
            if c_commemt.search(line):
              errVal = err(file, lnum, line, 'use C comments only')
        elif shell_file.search(file):
          if sh_wrong_char.search(line):
            errVal = err(file, lnum, line, 'for wrong char')
          elif sh_wrong_escape.search(line):
            errVal = err(file, lnum, line, 'wrong escape char')
        else:
          if wrong_char.search(line):
            errVal = err(file, lnum, line, 'for wrong char')
          elif wrong_tabs.search(line):
            errVal = err(file, lnum, line, 'Tabs are allowed only in begining')
        if len(line) > 85:
          errVal = err(file, lnum, line, 'line is too long')
        print(line)

      os.utime(file, times)
  if errVal:
    sys.exit(errVal)

###############################################################################
def main():
  p = os.path
  os.chdir(p.dirname(p.realpath(sys.argv[0])) + '/..')
  n = p.basename(sys.argv[0])
  eval('do_' + re.sub(r'\.py$', '', n))
main()
