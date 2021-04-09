# libpmc library to communicate with IEEE 1558 PTP clocks

# <u>Preface</u>

The library for PTP management client uses IEEE 1558 management messages to communicate with PTP entities over the network.
We support get, set and command of all management TLVs that are described in IEEE 1588-2019.
In addition, we support linuxptp specific implementation management TLVs.
The library supports the three PTP network layers

 1. UDP over IP version 4
 1. UDP over IP version 6
 1. PTP over Ethernet &mdash; *we do not support VLAN tags*

In addition, we support using the Unix network and communicate with linuxptp ptp4l.

We support also parsing incoming signaling messages.

# <u>libpmc Library</u>
The library is written in C++ using a data-oriented model.
Each class represent a data object with methods acting on the data.
The socket classes are the exception, as they represent a system object.
Management TLV that have data, have a structure and a process function that
build and parse from structure to message and via versa.
The caller needs to fill the structure and use it with a message object.
When receiving a reply message, the user can parse the message and
get a pointer to a structure from the message object.

The configuration file uses the [ptp4l](https://manpages.debian.org/unstable/linuxptp/ptp4l.8.en.html)
parameters that are relevant to pmc tool.

# <u>Using C</u>
This project does **NOT** support C directly.
Users that want to use the library with their C application,
need to write a wrapping in C++ and combine it with their application.

# <u>Scripting</u>
This project uses [SWIG](http://www.swig.org/) to generate wrapper to script languages.
For now, we support:

  * Perl version 5
  * Python 2.7 and Python 3
  * Lua 5.1, 5.2, 5.3
  * Ruby version 2

# <u>pmc tool</u>
The project provides a clone to [linuxptp](http://linuxptp.sf.net/)
pmc tool that uses the libpmc library.
The project does not provide a manual page to the tool.
As the tool mimic the original tool in command line, input and in output.
Please refer to the [pmc man page](https://manpages.debian.org/unstable/linuxptp/pmc.8.en.html)

# <u>Inspiration</u>
The library provide functionality that is provided by the pmc tool of the linuxptp project.
We wish to thank Richard Cochran and the linuxptp contributors for their excellent works.

# <u>Packaging</u>
The project comes with packaging of Debian packages for Debian and Debian based Linux systems.

# <u>Licence</u>
The Licence of this project is

  * The library is under LGPL 3 or later
  * The pmc tool is under GPL 3 or later
