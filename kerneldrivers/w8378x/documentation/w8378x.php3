<?
include( "common.php3" );

makepage( "Winbond w8378x kernel driver", "
<!-- ----------------------------------------------------------------------- -->

<p>
Kernel driver for the Winbond w8378x motherboard monitor chips.<br><br>

These chips are capable of mesuring tons of diffrent things, the most intresting
is cpu temperature and fan speeds.<br>
These chips are located on many newer motherbards these days.
</p>

<hr width=95%>

<p>
ioctl opcodes:
<ul>
<li><b>ioctl( handle, W8378x_READ_TEMP[1..3], int &temperature )</b> Returns the temperature in celcius*256 of the nth probe.
<li><b>ioctl( handle, W8378x_READ_FAN[1..3], int &temperature )</b> Returns the speed in rpm of the nth fan.
</p>

<!-- ----------------------------------------------------------------------- -->
" );

?>
