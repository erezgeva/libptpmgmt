# The diagram of clockmanager(CM)  usage in system : 

Test app <------> CM runtime lib (jclklib.so) <----------> jclklib_proxy <-----> libptpmgmt.so <-----> ptp4l

# How to install linux-ptp (with Intel suggested changes) :

	- Ptp4l version based on 4.2 .
	- ptp4l patches -> (servo_state  subscription changes, etc)
	
	Repo :  https://github.com/intel-staging/linux-ptp_iaclocklib
	
	Need to clone and install this version with our patches to extend ptp4l subscription options to be used later by libptpmgmt.
	
	**How to install linuxptp:** 
        make & make install                         


# How to compile clock manager:

Clock manager is basically divided to 2 parts: jclklib.so + jclklib_proxy . 

Test application is using CM runtime library : jclklib.so provided API to connect, subscribe and receive ptp4l events. Jclkib.so is communicating with jclklib_proxy using messagequeue. jclklib_proxy uses  libptpmgmt.so API (dynamically linked) to subscribe to ptp4l events.

First, we need to compile the libptpmgmt, giving us the libptpmgmt.so .
Then we compile the clock manager inside the jclklib folder. This will give us : jclklib.so, jclklib_proxy binary and  test binary. 

	1. Clone the github : https://github.com/intel-staging/libptpmgmt_iaclocklib
	2. Package dependencies to be installed before compiling libptpmgmt :

	Swig
	Cppcheck
	Doxygen
	Ctags
	astyle
	Dot
	Epstopdf
	Valgrind
	Libtool-bin
	
	3. Run this at the github root directory to compile libptpmgmt :
		autoconf
		./configure
		make
		
	4. Change directory to jclklib and run make clean && make
	
NOTE :  make will traverse the client, common and proxy folder one by one. You need to type "q" to continue from one folder to another. This is a temporary situation. 

        5. Outcome : you must have 3 binaries created - 
		client/test
		client/jclklib.so
		proxy/jclklib_proxy 

# How to test :

We are currently using a "test.cpp" file to act as an application. This test.cpp is using the API provided in the init.hpp.

We are dividing the test plan according to message type :
	A. Connect message
	B. Subscription message
	C. Notification message
		
		
##	A. CONNECT message 

** Prerequisite ** 
Jclklib_proxy needs to be running before running the test. 
Ptp4l shd be running before as well :  proxy/run_ptp4l.sh

### Test setup : 

	1. On a terminal, we run : ./proxy/jclklib_proxy 
	2. In the same machine, on a second terminal , e run the test app, using client/run_test.sh on one terminal. -> this will execute the test with LD_CONFIG_PATH set using the current libptpmgmt.so 
	Please change the LD_CONFIG_PATH accordingly. 
	
### Test scenario for connect :

	1. Proxy is not responding (i.e proxy is hang, message queue not working properly) -> in this case, by default timeout for connect is 5 sec . 

	2. Session ID is unique and incremental for a lifetime of proxy. When the proxy is restarted, the sessionID restarts at 0.  
	
	sessionID : std::uint16_t
	
##	B. Subscription message 




