<?
include( "common.php3" );

function makesubheading( $title, $body )
{
	return "<h2>$title</h2>
		$body";
}


makepage( "SuperSmartCard kernel driver", "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Description", "

Kernel driver for the SuperSmartCard (SSC) Gameboy flash card writer.<br>
The SSC was made by SSL, but I have head that they went out of buisness around '95.

") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Installation / configuration", "

To install the driver, just run the <i>install.sh</i> file in the archive.<br>
<br>
The driver presumes that you have hooked the write to a parallel port that uses
io port 0x378, if this is not the case, you must edit/create the file
/boot/home/config/settings/kernel/drivers/ssc and set the ioport option to
the right port (on intel you can use the Preferences/Devices app to find the
right port).
<br>
The scc file should look something like this:
<table border=0 bgcolor=#ffffff cellpadding=8> 
	<tr><td>
		<font color=#000000><pre>ioport 0x378</pre></font>
	</td></tr>
</table>
<br>
<br>

If you switch between diffrent devices on you parallel port, you have to
use the <i>rescan</i> program to let the ssc driver know that you have
changed hardware, you run it from a Terminal, like this:
<br>
<table border=0 bgcolor=#ffffff cellpadding=8> 
	<tr><td>
		<font color=#000000><pre>rescan ssc</pre></font>
	</td></tr>
</table>
<br>

") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Usage", "

The driver exports these devices in the /dev/misc/ssc directory:
<table border=1>
	<tr>
		<td><b>Device</b></td><td><b>Description</b></td>
	</tr><tr>
		<td>flash/all</td><td>all banks (boot+0+1+2+3) for a complete 512Kbytes card</td>
	</tr><tr>
		<td>flash/boot</td><td>The 64Kbytes boot area, used internaly for the small boot menu.</td>
	</tr><tr>
		<td>flash/0</td><td>a 64Kbytes bank.</td>
	</tr><tr>
		<td>flash/1</td><td>a 128Kbytes bank.</td>
	</tr><tr>
		<td>flash/2</td><td>another 128Kbytes bank.</td>
	</tr><tr>
		<td>flash/3</td><td>and yet a 128Kbytes bank.</td>
	</tr><tr>
		<td>flash/23</td><td>bank 2 and 3 joined to a 256Kbytes bank.</td>
	</tr><tr>
		<td>ram/0</td><td>8Kbytes savegame bank for flash/0</td>
	</tr><tr>
		<td>ram/1</td><td>8Kbytes savegame bank for flash/1</td>
	</tr><tr>
		<td>ram/2</td><td>8Kbytes savegame bank for flash/2 or flash/23</td>
	</tr><tr>
		<td>ram/3</td><td>8Kbytes savegame bank for flash/3</td>
	</tr>
</table>
<br>
<br>

So if you want to install MegaMan X's T2 demo (512K), you just copy it like this:<be>
<table border=0 bgcolor=#ffffff cellpadding=8> 
	<tr><td>
		<font color=#000000><pre>cp t2.gb /dev/misc/ssc/flash/all</pre></font>
	</td></tr>
</table>
<br>
<br>
You can also install more that one romimage at a time, this is how you install
Anders Granlund's Big Scroll demo, Ian Jame's Bond, Harry Mulder's Kirby XXL and
Jeff Frohwein's Basic into one cardridge:
<table border=0 bgcolor=#ffffff cellpadding=8> 
	<tr><td>
		<font color=#000000><pre>cp Bc.gb /dev/misc/ssc/flash/0
cp bond.gb /dev/misc/ssc/flash/1
cp kirbyxxl.gb /dev/misc/ssc/flash/2
cp basic.gb /dev/misc/ssc/flash/3</pre></font>
	</td></tr>
</table>
The kerneldriver will append a small boot selector to the card, so that you may
choose which program to run when you power up the gameboy.
<br>
<br>
To backup or restore the savegame ram (does not work with the black/transparent
color gameboy card's):
<table border=0 bgcolor=#ffffff cellpadding=8> 
	<tr><td>
		<font color=#000000><pre>cp /dev/misc/ssc/ram/0 savegame.backup
or
cp savegame.backup /dev/misc/ssc/ram/0 </pre></font>
	</td></tr>
</table>


") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Bugs / limitations", "

The boot selecter does not work with color programs, so if you want to see
color demos, you have to upload them to either the <i>boot</i> or the <i>all</i>
bank.<br><br>

The memory controller in the flash card that comes with the SSC seems to have
problems with dma's when the cpu is runnning in dobbelt speed, this means
that a lot of demos just won't work :(<br>
") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "License", "

This software is released under the BSD License:
<p>
<pre>
Copyright (c) 1999, Jesper Hansen. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither name of the company nor the names of its contributors may
   be used to endorse or promote products derived from this software
   without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
</pre>

") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Author", "

Jesper Hansen<br>
<a href=\"mailto:jesper@funcom.com\">jesper@funcom.com</a><br>
The source and more informantion can be found <a href=http://sourceforge.net/project/?group_id=724>here</a>.<br>

") . "
<!-- ----------------------------------------------------------------------- -->
" );

?>
