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

?>

