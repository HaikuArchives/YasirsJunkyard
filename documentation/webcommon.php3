<?

//$dotimage = imagecreate( 1, 1 );

$menubgcol[0]=255;
$menubgcol[1]=150;
$menubgcol[2]=100;

$menubordercol[0]=0;
$menubordercol[1]=0;
$menubordercol[2]=0;

$bgcol[0]=220;
$bgcol[1]=220;
$bgcol[2]=220;

function makecol( $col1, $col2, $alpha )
{
	$r = ($col1[0]*(255-$alpha) + $col2[0]*($alpha)) / 255;
	$g = ($col1[1]*(255-$alpha) + $col2[1]*($alpha)) / 255;
	$b = ($col1[2]*(255-$alpha) + $col2[2]*($alpha)) / 255;
	return sprintf( "\"#%02x%02x%02x\"", $r, $g, $b );
}

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
}

function addtopentry( $title, $link )
{
}

function addtophead( $title )
{
	global $menubgcol,$bgcol,$menubordercol;
	echo "
	<tr bgcolor=#000000 valign=top>
		<td align=center>";
			if( $title != "" )
			{
				echo "
				<font face=\"MS Sans Serif, Geneva, Swiss\" size=1 color=#FFFFFF>
					<b>&nbsp;$title</b>
				</font>";
			}
			else
			{
				echo "<img src=1p.gif width=1 height=4>";
			}
		echo "
		</td>
		<td width=1 bgcolor=",makecol($menubordercol,$bgcol,64)," nowrap rowspan=4><img src=1p.gif width=1 height=1></td>
		<td width=1 bgcolor=",makecol($menubordercol,$bgcol,128)," nowrap rowspan=5><img src=1p.gif width=1 height=1></td>
		<td width=1 bgcolor=",makecol($menubordercol,$bgcol,196)," nowrap rowspan=4><img src=1p.gif width=1 height=1></td>
	</tr>";
	echo "
		<tr><td bgcolor=",makecol($menubordercol,$menubgcol,255*1/3),"><img src=1p.gif width=1 height=1></td></tr>
		<tr><td bgcolor=",makecol($menubordercol,$menubgcol,255*2/3),"><img src=1p.gif width=1 height=1></td></tr>";
	echo "
	<tr bgcolor=",makecol($menubgcol,$menubgcol,0),">
		<td> 
			<table width=100% border=0 cellspacing=1> 
				<tr bgcolor=",makecol($menubgcol,$menubgcol,0),">
					<td width=100%>
						<font face=\"MS Sans Serif, Geneva, Swiss\" size=1>";
}

function addsubentry( $title, $link )
{
	global $ext;
	echo "
		<a href=\"$link.$ext\"><font color=#000000>$title</font></a>
		<br>";
}

function addtopend()
{
	global $menubgcol,$bgcol,$menubordercol;
	echo "
						</font>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr><td bgcolor=",makecol($menubordercol,$bgcol,64)," colspan=2><img src=1p.gif width=1 height=1></td></tr>
	<tr><td bgcolor=",makecol($menubordercol,$bgcol,128)," colspan=2><img src=1p.gif width=1 height=1></td></tr>
	<tr><td bgcolor=",makecol($menubordercol,$bgcol,196),"><img src=1p.gif width=1 height=1></td></tr> 
	<tr>
		<td>
			<font size=-2>&nbsp;</font>
		</td>
	</tr>";
}

/*
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

//	echo "<table cols=3 border=0 cellspacing=0 cellpadding=0 width=100%>",
//		"<tr bgcolor=#636b8c><td width=",$width,"></td><td><font size=+1> <b>",
//		"<a href=",$link,".",$ext,"><font color=#b3bbdc>",$title,"</font></a>",
//		"</b></font></td><td width=10></td></tr>",
//		"<tr><td bgcolor=#000000 height=1 colspan=3></td></tr>",
//		"</table>\n";
}
*/

function makepage( $title, $page )
{
	global $ext;
?>
<html><head>
<title>Yasir's Junkyard - <?echo $title?></title>
</head>

<!-- I totaly lack html design knowlegde, so the menulook is ripped from the BeNews site... -->
		
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
		<hr width=95%>
	</td></tr>

	<tr valign=top>
		<td width=100 nowrap rowspan=99>
			<table width=100 cellspacing=0 border=0 cellpadding=0>
			
				<?addtophead( "Junkyard" )?>
					<?addsubentry( "Index", "index" )?>
				<?addtopend()?>

				<?addtophead( "Translators" )?>
					<?addsubentry( "AmigaIcon", "amigaicon" )?>
					<?addsubentry( "HsiRaw", "hsiraw" )?>
				<?addtopend()?>

				<?addtophead( "Kernel drivers" )?>
					<?addsubentry( "Optrex", "optrex" )?>
					<?addsubentry( "W8378x", "w8378x" )?>
					<?addsubentry( "SuperSmartCard", "ssc" )?>
				<?addtopend()?>

				<?addtophead( "Utils" )?>
					<?addsubentry( "OptrexCPUMon", "optrexcpumon" )?>
				<?addtopend()?>

				<?addtophead( "Misc" )?>
					<?addsubentry( "Bake", "bake" )?>
				<?addtopend()?>
			
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
	<hr width=95%>
		(c) '2000 <a href="mailto:jesper@funcom.com">Jesper Hansen</a>
	</p> 
</center>

</body>
</html>
<?
}
?>

