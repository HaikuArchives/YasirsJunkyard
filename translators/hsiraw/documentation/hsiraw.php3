<?
include( "common.php3" );

function makesubheading( $title, $body )
{
	return "
	<table cols=3 border=0 cellspacing=0 cellpadding=0 width=100%>
		<tr bgcolor=#e0e0e0>
			<td width=10></td>
			<td><font size=+1><b>$title</b></font></td>
			<td width=10></td>
		</tr>
		<tr>
			<td bgcolor=#c0c0c0 height=1 colspan=2>
			</td>
		</tr>
		<tr>
			<td></td>
			<td>
				$body
			</td>
			<td></td>
		</tr>
	</table>";
}

makepage( "HSIRaw Translator", "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Description", "<br><br>

Enables all Translator aware applications to read and write 8 and 24 bit, version 4 HSIRaw bitmaps.<br>
HSI Raw is a graphic format used by <b>Image Alchemy</b> made by <b>Handmade Software Inc.</b>

<br><br>") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Installation", "<br><br>

The translator has a buildin installer, so just dobbelt-click on the <i>AmigaIconTranslator</i> file.

<br><br>") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Configuration", "<br><br>

<center><img src=hsirawconfig.png></center>
<br><br>

<b>Read BW/Gray images as truecolor</b>: for some reason BeOS does not handle grayscale images well,
so when this options is enabled grayscale HSIRaw images is retured as 24 bit.

<br><br>") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Changes", "<br><br>

<table border=0>
<tr valign=top><td>0.9.1 :</td><td>Minor bugfixes.<br>License changed from GPL to BSD.</td></tr>
<tr valign=top><td>0.9.0 :</td><td>This is the first version of HSIRaw that works on R4.</td></tr>
</table>

<br><br>") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "License", "<br><br>

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

<br><br>") . "
<!-- ----------------------------------------------------------------------- -->
" . makesubheading( "Changes", "<br><br>

Jesper Hansen<br>
<a href=\"mailto:jesper@funcom.com\">jesper@funcom.com</a><br>
<a href=http://home.c2i.net/damnrednex/>http://home.c2i.net/damnrednex</a><br>

<br><br>") . "
<!-- ----------------------------------------------------------------------- -->
" );

?>
