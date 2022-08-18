# libptpmgmt library to communicate with IEEE 1558 PTP clocks

# <u>Preface</u>

The library for PTP management client uses IEEE 1558 management messages to communicate with PTP entities over the network.  
We support get, set and command of all management TLVs that are described in IEEE 1588-2019.
In addition, we support LinuxPTP specific implementation management TLVs.  
The library supports the three PTP network layers

 1. UDP over IP version 4
 1. UDP over IP version 6
 1. PTP over Ethernet &mdash; *we do not support VLAN tags*

In addition, we support using the Unix network and communicate with LinuxPTP ptp4l.

We support also parsing incoming signalling messages.

# <u>libptpmgmt Library</u>
The library is written in C++ using a data-oriented model.  
It uses the namespace 'ptpmgmt'.  
Each class represent a data object with methods acting on the data.  
The socket classes are the exception, as they represent a system object.  
Management TLV that have data, have a structure and a process function that
build and parse from structure to message and via versa.  
The caller needs to fill the structure and use it with a message object.  
When receiving a reply message, the user can parse the message and
get a pointer to a structure from the message object.

The configuration file uses the [ptp4l](https://manpages.debian.org/unstable/linuxptp/ptp4l.8.en.html)
parameters that are relevant to the pmc tool.

# <u>Using C</u>
This project does **NOT** support C directly.  
Users that want to use the library with their C application,
need to write a wrapping in C++ and combine it with their application.

# <u>Scripting</u>
This project uses [SWIG](http://www.swig.org/) to generate wrapper to script languages.
For now, we support:

  * Perl version 5
  * Python 2.7 and Python 3
  * Lua 5.1, 5.2, 5.3, 5.4
  * Ruby version 2
  * PHP version 7
  * TCL version 8

Though the C++ library uses the namespace 'ptpmgmt'.  
SWIG for PHP do not support namespace.  
Ruby uses 'Ptpmgmt' as namespace follow Ruby convention.  
Perl uses the name 'PtpMgmtLib' follow Perl convention.  
Tcl, Lua, and Python do use the namespace 'ptpmgmt'.

Some C++ syntax is ignored or renamed in various scripts,  
for example in PHP: Binary::empty is renamed to Binary::c_empty  
See comments in libptpmgmt.i for the relevant changes

Some C++ structure and functions use C++ standard vector "`std::vector<>`"  
SWIG map C++ standard vector to a class.  
See libptpmgmt.i for the full list of the mapping classes.  
All languages create the vector as a class object.  
In Python, Ruby and TCL the vector have properties of a native list.  
Lua uses subst of C++ standard vector methods.  
Perl and PHP use class methods, see PtpMgmtLib.pm and ptpmgmt.php for these methods.

# <u>Library content</u>
  * Binary in bin.h - class that hold a binary
  * Buf in buf.h - Allocate buffer for scripting
  * ConfigFile in cfg.h - Read, parse, and provide configuration from configuration file
  * Message in msg.h - Create and parse Management and Signalling PTP messages
  * IfInfo in ptp.h - Provide information on a network interface
  * PtpClock in ptp.h - Provide a PTP dynamic clock ID
  * sockets classes in sock.h - Provide access to UPD IPv4, IPv6, and L2 PTP networks
  * SockUnix in sock.h - socket to communicate with local LinuxPTP daemon
  * Management TLVs in proc.h - structures that hold a PTP Management TLV data
  * Signalling TLVs in sig.h - structures that hold a PTP Signalling TLV data
  * Library version in ver.h
  * Managment TLVs mngIds.h - enumerator for PTP Management TLVs
  * PTP managment types types.h - enumerators and structure to use with PTP Management messages
  * Dispatcher and builder in msgCall.h - classes which provice call-backs for specific Management TLVs
  * Dispatcher and builder base in callDef.h - provide all call-backs which may be impleamented
  * Time convertion in timeCvrt.h - constants to convert time to different units.
  * Json2msg in json.h - Convert json text to a message, require linking with a JSON library
  * msg2json in json.h - Convert message to json text
  * Options in opt.h - parse a pmc tool command line parameters
  * Init in init.h - Initialize objects for a pmc tool

# <u>pmc tool</u>
The project provides a clone of [LinuxPTP](http://linuxptp.sf.net)
pmc tool using the libptpmgmt library.  
The project does not provide a manual page to the tool.
As the tool mimic the original tool in command line, input and in output.  
Please refer to the [pmc man page](https://manpages.debian.org/unstable/linuxptp/pmc.8.en.html)

# <u>Inspiration</u>
The library provide functionality that is provided by the pmc tool of the LinuxPTP project.  
We wish to thank Richard Cochran and the LinuxPTP contributors for their excellent works.

# <u>Packaging</u>
The project comes with packaging:

  * Debian packages for Debian and Debian based Linux systems.
  * RPM packages for Red Hat based Linux systems.
  * Arch Linux packages.

# <u>Licence</u>
The Licence of this project is

  * The library is under LGPL 3 or later
  * The pmc tool and the testing scripts are under GPL 3 or later
