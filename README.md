<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> -->
# libptpmgmt library to communicate with IEEE 1558 PTP clocks

# <u>Preface</u>

The library for PTP management client uses IEEE 1558 management messages to communicate with PTP entities over the network.  
We support get, set and command of all management TLVs that are described in IEEE 1588-2019.  
In addition, we support LinuxPTP specific implementation management TLVs.  
The library supports the three PTP network layers:

 1. UDP over IP version 4
 1. UDP over IP version 6
 1. PTP over Ethernet &mdash; *we do not support VLAN tags*

In addition, we support using the Unix network and communicate with LinuxPTP ptp4l.  
We also support parsing incoming signalling messages.  
Users can use Linux VLAN with all sockets.

# <u>libptpmgmt Library</u>
The library is written in C++ using a data-oriented model.  
It uses the namespace 'ptpmgmt'.  
Each class represents a data object with methods acting on the data.  
The socket classes are the exception, as they represent a system object.  
Management TLVs that have data, have a structure and a process function that
builds and parses from structure to message and via versa.  
The caller needs to fill the structure and use it with a message object.  
When receiving a reply message, the user can parse the message and
get a pointer to a structure from the message object.

The configuration file uses the [ptp4l](https://linuxptp.nwtime.org/documentation/ptp4l/)
parameters that are relevant to the pmc tool.

# <u>Using C</u>
The library is written in C++ and provides a C wrapper.  
A notice regarding memory:  
The classes wrappers, provide a `free` callback which free any memory allocated by the wrapper itself.  
With one exception: the functions `ptpmgmt_json_msg2json` and `ptpmgmt_json_tlv2json` allocate string,
 which you need to free, in your application!  
You need to release any memory allocated on your application,
 as the library and the wrappers do **not** free them!  
As C do not provides namespaces, all global functions and global structures are prefix with `ptpmgmt_` or `PTPMGMT_`.

# <u>C++ and C standards</u>
The libptpmgmt Library uses `C++11` with [POSIX](https://posix.opengroup.org/), [GNU](https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html) extensions and [Linux kernel headers](https://kernel.org/).  
The C wrapper use `C99`.  
We try our best to avoid conflicts with newer versions of C++ and C. But if you find a conflict, please notify us.

# <u>Scripting</u>
This project uses [SWIG](http://www.swig.org/) to generate wrapper to script languages.  
For now, we support:

  * Perl version 5
  * Python version 3
  * Lua versions 5.1, 5.2, 5.3, 5.4
  * Ruby version 2, 3
  * PHP version 7, 8
  * Tcl version 8
  * Go version 1.14 and above

Though the C++ library uses the namespace 'ptpmgmt'.  
SWIG for PHP does not support namespace.  
Ruby uses 'Ptpmgmt' as namespace, following Ruby convention.  
Perl uses the name 'PtpMgmtLib', following Perl convention.  
Tcl, Lua, Go, and Python use the namespace 'ptpmgmt'.

Some C++ syntax is ignored or renamed in various scripts.  
For example in PHP: Binary::empty is renamed to Binary::c_empty.
See comments in libptpmgmt.i and warn.i of each language for the relevant changes.

Some C++ structure and functions use C++ standard vector "`std::vector<>`"  
SWIG maps C++ standard vector to a class.  
See libptpmgmt.i for the full list of the mapping classes.  
All languages create the vector as a class object.  
In Python, Ruby, and Tcl the vector has the properties of a native list.  
Lua uses subset of C++ standard vector methods.  
Perl, PHP, and Go use class methods. See PtpMgmtLib.pm for Perl, ptpmgmt.php for php,
 and ptpmgmt.go for Go, for these methods.

[std_vectors.md](./std_vectors.md) provides more information on vectors mapping and the Doxygen documentation provides information per class.

# <u>Go wrapper</u>

Since Go is a compile language,  
and not a "pure" script, the wrapper is used a bit differently.  
The wrapper is only required during development,  
 as the resulting application is a binary that does not require the Go wrapper, only the C++ library.  
During development, you need the Go wrapper and the C++ library development headers,  
 as the Go build compiles the Go wrapper.  
In addition add the '-lm -lptpmgmt' flags to the linking using the `CGO_LDFLAGS` environment,  
 so Go will link your application with the C++ library.  
Pay attention that although Go uses static typing and checks the types in compilation,
some C++ methods uses variable arguments or share name for different methods. In this case, swig will use the `... interface{}` parameter
and perform the type check in runtime; if types are wrong, the Go swig wrapper will issue an exception to your application in run-time.  
For example, `Binary.SetBin(position, value)` requires `position` to be `int64` and `value` to be `byte`.  
Go uses methods for interface which use Pascal notation, so all class functions in the C++ library are converted to use first letter capital.  
Go does not use constructors and destructors.  
You need to use the New'Class' functions and release with Delete'Class'.  
You can use the `defer` statment for the releasing, if the release is due in the same function.  
As Go does not provide destructors, MessageBuilder is not a class and it does not call `message.clearData()` once it is removed.  
You are advised to call `message.clearData()` once you build the message, and do not plan any further use with the send TLV.  
Note: as Go syntax is stricter, you may need to update your code with small fixes, when you build with a newer version of the Go wrapper of the library.  
The C++ library does sustain backward compatible, yet Go wrapper does not and may break!

# <u>Library content</u>
  * Binary in bin.h - Class that holds a binary
  * Buf in buf.h - Allocate buffer for scripting
  * ConfigFile in cfg.h - Read, parse, and provide configuration from configuration file
  * Message in msg.h - Create and parse Management and Signalling PTP messages
  * IfInfo in ptp.h - Provide information on a network interface
  * PtpClock in ptp.h - Provide a PTP dynamic clock ID
  * sockets classes in sock.h - Provide access to UPD IPv4, IPv6, and L2 PTP networks
  * SockUnix in sock.h - Socket to communicate with local LinuxPTP daemon
  * Management TLVs in proc.h - Structures that hold a PTP Management TLV data
  * Signalling TLVs in sig.h - Structures that hold a PTP Signalling TLV data
  * Library version in ver.h
  * Managment TLVs mngIds.h - Enumerator for PTP Management TLVs
  * PTP managment types types.h - Enumerators and structure to use with PTP Management messages
  * Dispatcher and builder in msgCall.h - Classes which provide call-backs for specific Management TLVs
  * Dispatcher and builder base in callDef.h - Provide all call-backs which may be implemented
  * Time convertion in timeCvrt.h - Constants to convert time to different units
  * Json2msg in json.h - Convert json text to a message, require linking with a JSON library
  * msg2json in json.h - Convert message to json text
  * Options in opt.h - Parse pmc tool command line parameters
  * Init in init.h - Initialize objects for a pmc tool

# <u>pmc and phc_ctl tools</u>
The project provides a clone of [LinuxPTP](https://linuxptp.nwtime.org)
pmc tool using the libptpmgmt library  
and phc_ctl using the libptpmgmt library and python wrapper.

# <u>Inspiration</u>
The library provides functionality that is provided by the pmc tool of the LinuxPTP project.  
We wish to thank Richard Cochran and the LinuxPTP contributors for their excellent work.

# <u>Packaging</u>
The project comes with packaging:

  * Debian packages for Debian and Debian based Linux systems.
  * RPM packages for Red Hat based Linux systems.
  * Arch Linux packages.
  * Gentoo build recipe using ebuild.

# <u>More Documentation</u>
The project more documents

  * [Build project documentation](./Build.md)
  * [C++ std::vector<> map in Scripts Languages](./std_vectors.md)
  * [Frequently asked questions](./FAQs.md)
  * [Developer Guide for using Doxygen in our project](./HOWTO_doc_code.md)
  * Library API doxygen, can be generated using the make file. [Or on github](https://erezgeva.github.io/libptpmgmt/)

# <u>Licence</u>
The Licence of this project is

  * The library is under LGPL v3 or later.
  * The pmc and phc_ctl tools and the testing scripts are under GPL v3 or later.
  * Documentation is under under GFDL v1.3 or later without invariants sections.
