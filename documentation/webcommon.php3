<?

//$dotimage = imagecreate( 1, 1 );

function makeheading( $width, $link, $alt, $title )
{
	global $ext;

	echo "<table cols=3 border=0 cellspacing=0 cellpadding=0 width=100%>",
		"<tr bgcolor=#636b8c>",
			"<td width=",$width," height=1></td>",
			"<td height=1>";
		
	if( $link != "" )	echo	"<a href=",$link,".",$ext,">";
	if( $title != "" )
	{
						echo	"<img border=0 src=",$title;
						if( $alt != "" ) echo " alt=",$alt;
						echo	">";
	}
	if( $link != "" )	echo	"</a>";

	echo	"</td><td width=10 height=1></td></tr><tr width=100%><td bgcolor=#000000 height=1 colspan=3></td>",
			"</tr></table>\n";
/*
	echo "<table cols=3 border=0 cellspacing=0 cellpadding=0 width=100%>",
		"<tr bgcolor=#636b8c><td width=",$width,"></td><td><font size=+1> <b>",
		"<a href=",$link,".",$ext,"><font color=#b3bbdc>",$title,"</font></a>",
		"</b></font></td><td width=10></td></tr>",
		"<tr><td bgcolor=#000000 height=1 colspan=3></td></tr>",
		"</table>\n";
*/
}

function makepage( $title, $page )
{
	global $ext;
?>
<html><head>
<title>Yasir's Junkyard - <?echo $title?></title>
</head>
		
<body bgcolor=#f0f0f0 text=#000000 background=yasirsjunkyard2.gif>

<table border=0>
	<tr>
		<td align=center>
			<font size="+2">Yasir's<br>Junkyard</font><br>
		</td>
		<td valign=bottom>
		</td>
		<td align=center><?
		if( strpos($title,".png") > 0 )
			echo "<img border=0 src=",$title,">";
		else
			echo "<b><font size=+4>",$title,"</font></b>";
			?>
		</td>
	</tr>

	<tr><td  colspan=3>
		<hr width=80%>
	</td></tr>

	<tr>
		<td align=center valign=top bgcolor=#637b9c>
			<table cols=1 cellspacing=0 border=0 cellpadding=1>

				<tr><td><?makeheading( 10, "", "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "index", "index", "Main.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "", "Translators", "Translators.png" )?></td></tr>
				<tr><td><?makeheading( 20, "amigaicon", "AmigaIcon", "AmigaIcon.png" )?></td></tr>
				<tr><td><?makeheading( 20, "hsiraw", "HsiRaw", "HsiRaw.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "", "KernelDrivers", "KernelDrivers.png" )?></td></tr>
				<tr><td><?makeheading( 20, "optrex", "Optrex", "Optrex.png" )?></td></tr>
				<tr><td><?makeheading( 20, "w8378x", "W8378x", "W8378x.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "", "Utils", "Utils.png" )?></td></tr>
				<tr><td><?makeheading( 20, "optrexcpumon", "OptrexCPUMon", "OptrexCPUMon.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "bake", "Bake", "Bake.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "", "" )?></td></tr>
				
				<tr width=100%><td>
					<center>
					<br><br><br>
					Hosted by:<br>
					<a href="http://sourceforge.net"><img src="http://sourceforge.net/sflogo.php?group_id=724&type=1" width="88" height="31" border="0"></a>
					</center>
				</td></tr>
			</table>

		</td>
		<td valign=top height=400>
		</td>
		<td valign=top width=100%>
			<?echo $page?>
		</td>
	</tr>
	
</table>

<center>
	<p>
	<p>
	<hr width=80%>
		(c) '2000 <a href="mailto:jesper@funcom.com">Jesper Hansen</a>
	</p> 
</center>

</body>
</html>
<?
}
?>

