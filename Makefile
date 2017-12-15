#==============
#    srv
#==============

SUB_DIRS = srv_net srv

all:
	set -e; for i in $(SUB_DIRS); do $(MAKE) -C $$i; done
	echo -e "\a"

clean:
	set -e; for i in $(SUB_DIRS); do $(MAKE) -C $$i clean; done
	echo -e "\a"
