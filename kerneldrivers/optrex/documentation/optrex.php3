<?
include( "common.php3" );

function makesubheading( $title, $body )
{
	return "<h2>$title</h2>
		$body";
}

makepage( "Optrex kernel driver", "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Description", "

Kernel driver for the Optrex DMC16202 LCD display.<br>
The display is a 16x2 character, 5x8 dots matrix display.<br>

") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "ioctl opcodes", "

<ul>
<li><b>ioctl( handle, OPTREX_UPLOADLINE0, char[16] )</b> Display the 16 characters in the top line of the display.
<li><b>ioctl( handle, OPTREX_UPLOADLINE1, char[16] )</b> Display the 16 characters in the bottom line of the display.
<li><b>ioctl( handle, OPTREX_UPLOADALLLINES, char[32] )</b> Display the 16 first characters in the top line, and the last 16 in the bottom line.
<li><b>ioctl( handle, OPTREX_UPLOADGLYP[0..7], uint8[8] )</b> Upload a custon 5x8 pixels character to index n.
</ul>

") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Pictures", "

Here are some pictures of the old Win95 version:<br><br>
<img border=4 src=optrex1.jpg>
<img border=4 src=optrex2.jpg>
<img border=4 src=optrex3.jpg>
<img border=4 src=optrex4.jpg>

") . "
<!-- ----------------------------------------------------------------------- -->
" );

?>
