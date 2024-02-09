<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
  -- SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> -->
***Q:***  
I downloaded the libptpmgmt from [github](https://github.com/erezgeva/libptpmgmt/releases)
 and then tried to use the libptpmgmt with python test.  
But getting following error.

```
# python3 test.py
Traceback (most recent call last):
  File "test.py", line 14, in <module>
    import ptpmgmt
ModuleNotFoundError: No module named 'ptpmgmt'
```

***A:***  
The test script fails to find the libraries.  
Python script needs 2 libraries:  
 1. libptpmgmt.so - The ptpmgmt C++ library.  
 2. ptpmgmt.py and _ptpmgmt.so - The python wrapper library.

You can install the libraries in your system.  
Or you can use:  
 1. LD_LIBRARY_PATH to tell the shell the path to libptpmgmt.so.  
 2. PYTHONPATH to tell the shell the location of the python wrapper library files.

For example:  
`$ LD_LIBRARY_PATH=../.. PYTHONPATH=3 python3 test.py`


***Q:***  
Can you provide details how can I make use of ptpmgmt to get, query and set PTP configuration parameters?

***A:***  
The PTP daemon comes with its own configuration.  
The ptpmgmt uses the PTP IEEE management messages to monitor, query and set.

Look in the test.py, it shows reading and setting of `PRIORITY1`.  
You could also see "sample/sync_watch.py" which 
"continuously checks whether a PTP client is well synced to its master",  
using the `PORT_DATA_SET`, `PARENT_DATA_SET`, `CURRENT_DATA_SET`.

The library support [linuxptp](https://linuxptp.nwtime.org) vendor specific TLVs
like: `SUBSCRIBE_EVENTS_NP`.


***Q:***  
Where are ptpmgmt.py and _ptpmgmt.so files?
How do I build them?

***A:***  

They are built by the project.

You need [swig](https://swig.org/) to generate the wrapper code,
and [python](https://www.python.org) development package for [C API](https://docs.python.org/3/c-api/index.html).

The make file would skip the python wrapper library if python development were absent.

The project supports several languages in addition to native C++ library.  
The make does **NOT** notify the absence of languages, as the user may defer them.


***Q:***  
Where can I download the ptpmgmt.py and _ptpmgmt.so files?

***A:***  

The project does not provide binaries.  
You can build packages for you Linux distribution.  
 1. [Debian package](https://wiki.debian.org/deb)  
 2. [RPM package](https://rpm.org/)  
 3. [Arch Linux package](https://wiki.archlinux.org/title/creating_packages)  

Users can create packages for their system.  
The packages are per language, so for example, you can install the package for python only.


***Q:***  
I use Cent OS Linux.

***A:***  

You can build RPMs with  
`$ make rpm`

The RPM are tested on Fedora and should work on all [RPM-based Linux distributions](https://en.wikipedia.org/wiki/List_of_Linux_distributions#RPM-based).

The RPM files are stored under the `rpm/RPMS/` folder.
