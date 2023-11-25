# Build project documentation
How do we build the library, and what can we do with the make file.

# <u>GNU build</u>
We use GNU build

```
autoconf
./configure
make
```

# <u>configure</u>
The configure script support options.  
Some basic options comes from autoconf and some are specific to our project.  
Use help to see more information:

```
./configure --help
```

# <u>Make file</u>
The make file have many targets and parameters.  
Use help to see more information:

```
make help
```

# <u>Linux distribution</u>
We provide libraries and wrapper libraries for scripting.  
You can generate packages for installation.  
For Debain packages (and Ubuntu):

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

To install on Gentoo

```
make gentoo
```
