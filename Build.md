<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> -->
# Build project documentation
How we build the library, and what we can do with the make file.

# <u>GNU build</u>
We use GNU build:

```
autoconf
./configure
make
```

# <u>configure</u>
The configure script support options.  
Some basic options come from autoconf and some are specific to our project.  
Use help to see more information:

```
./configure --help
```

# <u>Make file</u>
The make file has many targets and parameters.  
Use help to see more information:

```
make help
```

# <u>Linux distribution</u>
We provide libraries and wrapper libraries for scripting.  
You can generate packages for installation.  
For Debian and Ubuntu packages:

```
make deb
```

For Red Hat packages (RPM):

```
make rpm
```

For Arch Linux packages:

```
make pkg
```

To install on Gentoo:

```
make gentoo
```
