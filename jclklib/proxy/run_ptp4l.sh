#clone the linux-ptp repo and make
git clone https://github.com/intel-staging/linux-ptp_iaclocklib.git
cd linux-ptp_iaclocklib/
make

#start ptp4l on board A (where jclklib_proxy running) as slave
./ptp4l -i enp1s0 -m -S -A -P -s -f configs/default.cfg

#start ptp4l on board B as master
#./ptp4l -i enp1s0 -m -S -A -P -f configs/default.cfg
