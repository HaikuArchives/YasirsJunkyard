<?
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
		<td align=center valign=top bgcolor=#737b9c> <--! bgcolor=#d0d0d0 -->
			<table cols=1 cellspacing=0>

				<tr><td><?makeheading( 10, "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "index", "Main.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "", "Translators.png" )?></td></tr>
				<tr><td><?makeheading( 20, "amigaicon", "AmigaIcon.png" )?></td></tr>
				<tr><td><?makeheading( 20, "hsiraw", "HsiRaw.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "", "KernelDrivers.png" )?></td></tr>
				<tr><td><?makeheading( 20, "optrex", "Optrex.png" )?></td></tr>
				<tr><td><?makeheading( 20, "w8378x", "W8378x.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "", "Utils.png" )?></td></tr>
				<tr><td><?makeheading( 20, "optrexcpumon", "OptrexCPUMon.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "" )?></td></tr>

				<tr><td><?makeheading( 10, "bake", "Bake.png" )?></td></tr>

				<tr><td><?makeheading( 10, "", "" )?></td></tr>
				
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

