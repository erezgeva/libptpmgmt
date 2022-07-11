# C++ std::vector<> map in scripts languages

We use C++ `std::vector<T>` when we need an array or a list.  
The benefits of the `std::vector<T>`, are:

 1. C++ manage the memory for us. We don't need to allocate or free.
 1. The vector can grow, we do not need to allocate on advance.


In the following disccusuin we shall call

  * ***'container'*** to the mapped class in scripts.
  * ***'element'*** to the C++ class, we use in the container.


SWIG does a special translation of `std::vector<T>` in a way which differ script language to other.  
In addition SWIG require explicit announcement for the mapping.

Currently the follow `std::vector<T>` maps are announced in libptpmgmt.i  

|Class name in scripts  |C++ `std::vector<T>`             |header |
|-----------------------|---------------------------------|-------|
|FaultRecord_v          |FaultRecord_t                    |types.h|
|ClockIdentity_v        |ClockIdentity_t                  |types.h|
|PortAddress_v          |PortAddress_t                    |types.h|
|AcceptableMaster_v     |AcceptableMaster_t               |types.h|
|LinuxptpUnicastMaster_v|LinuxptpUnicastMaster_t          |types.h|
|PtpEvent_v             |PtpEvent_t                       |ptp.h  |
|PtpSample_v            |PtpSample_t                      |ptp.h  |
|PtpSampleExt_v         |PtpSampleExt_t                   |ptp.h  |
|SigTime                |SLAVE_RX_SYNC_TIMING_DATA_rec_t  |sig.h  |
|SigComp                |SLAVE_RX_SYNC_COMPUTED_DATA_rec_t|sig.h  |
|SigEvent               |SLAVE_TX_EVENT_TIMESTAMPS_rec_t  |sig.h  |
|SigDelay               |SLAVE_DELAY_TIMING_DATA_NP_rec_t |sig.h  |


The container can be part of another class or structure.  
For example `std::vector<FaultRecord_t>` is part of `FAULT_LOG_t`, the structure used for FAULT_LOG Managment TLV.

It could be an argument of a function.  
For example `std::vector<PtpEvent_t>` is an argument of `readEvents()`.

# Container constructors

The allocation and release of both container and element are done in the same way we allocate other SWIG classes.  
The container provids the following constructors, following `std::vector<>` constructors.

|#  |result                 |argument 1|argument 2|
|---|-----------------------|----------|----------|
|0  |empty                  |          |          |
|1  |copy constructor       |container |          |
|1  |insert defualt elements|number    |          |
|2  |copies of element      |number    |element   |

* PHP do not support the last, 2 arguments constructor!

# Compare of C++ class methods with scripts  

C++ are the methods of [`std::vector<>`](https://en.cppreference.com/w/cpp/container/vector)  
-NA- means method is not available  
Python and Ruby support more methods  

|C++          |Perl |PHP     |tcl  |Lua      |Python   |Ruby    |
|-------------|-----|--------|-----|---------|---------|--------|
|size         |size |size    |size |size     |size     |size    |
|empty        |empty|is_empty|empty|empty    |empty    |empty?  |
|clear        |clear|clear   |clear|clear    |clear    |clear   |
|push_back    |push |push    |push |push_back|push_back|push    |
|back+pop_back|pop  |pop     |pop  |-NA-     |pop      |pop     |
|back         |-NA- |-NA-    |-NA- |back     |back     |back    |
|pop_back     |-NA- |-NA-    |-NA- |pop_back |pop_back |-NA-    |
|front        |-NA- |-NA-    |-NA- |front    |front    |front   |
|[]           |get  |get     |get  |[]       |[]       |[]      |
|[]=          |set  |set     |set  |[]=      |[]=      |[]=     |
|capacity     |-NA- |capacity|-NA- |-NA-     |capacity |capacity|
|reserve      |-NA- |reserve |-NA- |-NA-     |reserve  |reserve |
|max_size     |-NA- |-NA-    |-NA- |max_size |-NA-     |-NA-    |

# more documentation

The container in Python act as list and in Ruby as array.  

  * [Python list](https://docs.python.org/3/tutorial/datastructures.html)
  * [Ruby array](https://ruby-doc.org/core/Array.html)

Perl, Lua, PHP and tcl uses only the methods declared in `vecDef.h`.  
See doxygen documentation of `vecDef.h` for more detailed on the container methods.
