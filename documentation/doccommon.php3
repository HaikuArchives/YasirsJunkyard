<?
function makepage( $title, $page )
{
	global $ext;
?>
<html><head>
<title><?echo $title?></title>
</head>
		
<body bgcolor=#f0f0f0 text=#000000>

<center>
	<b><font size="+4"><?echo $title?></font></b>
</center>

<?echo $page?>

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

