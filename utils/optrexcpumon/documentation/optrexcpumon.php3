<?
include( "common.php3" );


function screenshot( $image, $title )
{
	return "<p>
		<table cellspacing=0 border=2>
			<tr><td><img border=4 src=$image></td></tr>
			<tr><td align=center>$title</td></tr>
		</table>
		</p>";
}


makepage( "Optrex CPU Monitor", "
<!-- ----------------------------------------------------------------------- -->

<p>
A program that binds the
<a href=w8378x.".$ext.">Winbond w8378x kernel driver</a>
and the
<a href=optrex.".$ext.">Optrex kernel driver</a>
together.

</p>

<hr width=95%>

<p>
<center>
Here are the screens it cycles through:<br>
<font size=-1>(captured with my
<a href=https://sourceforge.net/project/?form_grp=554>WebCamII</a> driver)</font><br><br>

" . screenshot( "datetime.jpg", "Top: load of the two cpu's.<br>Bottom: Current date and time." ) . "
" . screenshot( "uptime.jpg", "Shows how long the machine has<br>managed to run without crashing ;)" ) . "
" . screenshot( "idle.jpg", "How much time has the machine spend doing nothing." ) . "
" . screenshot( "cputemp.jpg", "Temperature of the cpu's.<br>(and no, it's <i>not</i> overclocked)." ) . "
" . screenshot( "mobotemp.jpg", "HOT!HOT!HOT!<br>Motherboard temperature" ) . "
" . screenshot( "hdfree.jpg", "Free space on my boot volume." ) . "
			
</center>
</p>

<!-- ----------------------------------------------------------------------- -->
" );

?>
