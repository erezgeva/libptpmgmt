<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> -->
# C++ std::vector<> map in Scripts Languages

We use C++ `std::vector<T>` when we need an array or a list.  
The benefits of the `std::vector<T>`, are:

 1. C++ manages the memory for us. We don't need to allocate or free.
 1. The vector can grow as we do not need to allocate on advance.


In the following discussion we shall call:

  * ***'container'*** to the mapped class in scripts.
  * ***'element'*** to the C++ class, we use in the container.


SWIG does a special translation of `std::vector<T>` in a way which differs from one script language to another.  
In addition, SWIG requires explicit announcement for the mapping.

Currently the follow `std::vector<T>` maps are announced in `libptpmgmt.i`:

|Class name in scripts  |C++ `std::vector<T>`             |header   |
|-----------------------|---------------------------------|---------|
|FaultRecord_v          |FaultRecord_t                    |types.h  |
|ClockIdentity_v        |ClockIdentity_t                  |types.h  |
|PortAddress_v          |PortAddress_t                    |types.h  |
|AcceptableMaster_v     |AcceptableMaster_t               |types.h  |
|LinuxptpUnicastMaster_v|LinuxptpUnicastMaster_t          |types.h  |
|PtpEvent_v             |PtpEvent_t                       |ptp.h    |
|PtpSample_v            |PtpSample_t                      |ptp.h    |
|PtpSampleExt_v         |PtpSampleExt_t                   |ptp.h    |
|SigTime                |SLAVE_RX_SYNC_TIMING_DATA_rec_t  |sigTlvs.h|
|SigComp                |SLAVE_RX_SYNC_COMPUTED_DATA_rec_t|sigTlvs.h|
|SigEvent               |SLAVE_TX_EVENT_TIMESTAMPS_rec_t  |sigTlvs.h|
|SigDelay               |SLAVE_DELAY_TIMING_DATA_NP_rec_t |sigTlvs.h|


The container can be part of another class or structure.  
For example `std::vector<FaultRecord_t>` is part of `FAULT_LOG_t`, the structure used for FAULT_LOG Management TLV.

It could be an argument of a function.  
For example `std::vector<PtpEvent_t>` is an argument of `readEvents()`.

# Container Constructors

The allocation and release of both container and element are done in the same way we allocate other SWIG classes.  
The container provides the following constructors, following `std::vector<>` constructors.

|#  |result                 |argument 1|argument 2|
|---|-----------------------|----------|----------|
|0  |empty                  |          |          |
|1  |copy constructor       |container |          |
|1  |insert default elements|number    |          |
|2  |copies of element      |number    |element   |

* PHP and Go do not support the 'copies of element' constructor!
* Java does not support the 'insert default elements' constructor!
* Java does provides two additional constructors with array of elements and iterable object of elements.

# Comparison of C++ Class Methods with Scripts  

C++ are the methods of [`std::vector<>`](https://en.cppreference.com/w/cpp/container/vector).
-NA- means method is not available.
Python and Ruby support more methods.

|C++          |Perl |PHP     |Tcl  |Go      |Lua      |Python   |Ruby    |Java    |
|-------------|-----|--------|-----|--------|---------|---------|--------|--------|
|size         |size |size    |size |Size    |size     |size     |size    |size    |
|empty        |empty|is_empty|empty|IsEmpty |empty    |empty    |empty?  |isEmpty |
|clear        |clear|clear   |clear|Clear   |clear    |clear    |clear   |clear   |
|push_back    |push |push    |push |Add     |push_back|push_back|push    |add     |
|back+pop_back|pop  |pop     |pop  |-NA-    |-NA-     |pop      |pop     |-NA-    |
|back         |-NA- |-NA-    |-NA- |-NA-    |back     |back     |back    |-NA-    |
|pop_back     |-NA- |-NA-    |-NA- |-NA-    |pop_back |pop_back |-NA-    |-NA-    |
|front        |-NA- |-NA-    |-NA- |-NA-    |front    |front    |front   |-NA-    |
|[]           |get  |get     |get  |Get     |[]       |[]       |[]      |get     |
|[]=          |set  |set     |set  |Set     |[]=      |[]=      |[]=     |set     |
|capacity     |-NA- |capacity|-NA- |Capacity|-NA-     |capacity |capacity|capacity|
|reserve      |-NA- |reserve |-NA- |Reserve |-NA-     |reserve  |reserve |reserve |
|max_size     |-NA- |-NA-    |-NA- |-NA-    |max_size |-NA-     |-NA-    |-NA-    |

# More Documentation

The container in Python acts as list, in Ruby as array and in Java as AbstractList.  

  * [Python list](https://docs.python.org/3/tutorial/datastructures.html)
  * [Ruby array](https://ruby-doc.org/core/Array.html)
  * [Java AbstractList](https://docs.oracle.com/javase/9/docs/api/java/util/AbstractList.html)

Perl, Lua, PHP, Tcl, and Go use only the methods declared in `vecDef.h`.  
See doxygen documentation of [vecDef.h](https://erezgeva.github.io/libptpmgmt/vecDef_8h.html) for more details on the container methods.
