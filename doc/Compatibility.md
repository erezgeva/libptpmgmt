<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> -->
# Compatibility with Linuxptp

In addition to IEEE 1588-2019,
libptpmgmt implement LinuxPTP specific management TLVs.  
The following table specify the libptpmgmt version that
 support each LinuxPTP version.  
Note that libptpmgmt version 1.3 support AUTHENTICATION TLV.
<br>

|libptpmgmt|LinuxPTP|
|----------|--------|
|1.0       |4.0     |
|1.1       |4.1     |
|1.2       |4.2     |
|1.3       |4.4     |

<br>
## Note
The above table focus on **full** compatible with LinuxPTP.  
Other combinations would work, lacking newer features!
