#!/bin/sh

if alert "Are you sure you want to install the SSC driver?" "No" "Yes" ; then
  exit
fi

cwd=$(dirname $0)

######################
#install kernel driver
######################
cp --force --verbose $cwd/ssc ~/config/add-ons/kernel/drivers/bin
mkdir --parents --verbose ~/config/add-ons/kernel/drivers/dev/misc
ln --symbolic --force --verbose ../../bin/ssc ~/config/add-ons/kernel/drivers/dev/misc

###########################
#activate the kernel driver
###########################
rescan ssc

if [ -d /dev/misc/ssc ] ; then
	alert --idea "The driver found a SuperSmartCard" "Cool"
else
	alert --stop "The driver could not find a SuperSmartCard. Read the ssc_doc.html for comfiguration tips." "Bummer"
fi
