<?
if( getenv("PHPGENDOC") == "1" )
{
	include( "doccommon.php3" );
}
else
{
	include( "webcommon.php3" );
}

if( getenv("PHPGENHTML") == "1" )
	$ext = "html";
else
	$ext = "php3";

function makeheading( $width, $link, $title )
{
	global $ext;

	echo "<table cols=3 border=0 cellspacing=0 cellpadding=0 width=100%>",
		"<tr bgcolor=#636b8c>",
			"<td width=",$width,"></td>",
			"<td>";
		
	if( $link != "" )	echo	"<a href=",$link,".",$ext,">";
	if( $title != "" )	echo	"<img border=0 src=",$title,">";
	if( $link != "" )	echo	"</a>";

	echo	"</td><td width=10></td></tr><tr><td bgcolor=#000000 height=1 colspan=3></td>",
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
?>

