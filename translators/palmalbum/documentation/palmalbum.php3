<?
include( "common.php3" );

function makesubheading( $title, $body )
{
	return "<h2>$title</h2>
		$body";
}

makepage( "HSIRaw Translator", "
<!-- ======================================================================= -->
" . makesubheading( "Description", "

A Translator that can read and write the AlbumToGo color image format.<br>
AlbumToGo is a smple image viewer that comes with the Palm IIIc.

") . "
<!-- ======================================================================= -->
" . makesubheading( "Installation", "

The translator has a buildin installer, so just dobbelt-click on the
<i>PalmAlbumTranslator</i> file.<br>

If you are running BeOS R5 or newer, you should also run <i>RegisterMime.sh</i>
to install the sniffer rule.

") . "
<!-- ======================================================================= -->
" . makesubheading( "Configuration", "

Ths current version is not configurable...

") . "
<!-- ======================================================================= -->
" . makesubheading( "Changes", "

<table border=0>
<tr valign=top><td>0.1.0 :</td><td>First version.</td></tr>
</table>

") . "
<!-- ======================================================================= -->
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
<!-- ======================================================================= -->
" . makesubheading( "Author", "

Jesper Hansen<br>
<a href=\"mailto:jesper@funcom.com\">jesper@funcom.com</a><br>
The source and more informantion can be found <a href=http://sourceforge.net/project/?group_id=724>here</a>.<br>

") . "
<!-- ======================================================================= -->
" );

?>
