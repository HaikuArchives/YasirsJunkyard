/*
 * Copyright (c) 1999, Jesper Hansen. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither name of the company nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//-----------------------------------------------------------------------------
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//-------------------------------------
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <interface/Font.h>
#include <interface/View.h>
#include <storage/File.h>
#include <storage/Mime.h>
#include <translation/BitmapStream.h>
#include <translation/TranslatorRoster.h>
//-------------------------------------
#include "misc/MimeUtils.h"
#include "gfx/TranslatorUtils.h"
//-----------------------------------------------------------------------------

#define SPLITID( _id ) char(((_id)>>24)&0xff),char(((_id)>>16)&0xff),char(((_id)>>8)&0xff),char(((_id)>>0)&0xff)

void ListFonts()
{
	printf( "Installed fonts:\n" );

	int fontfamiliescnt = count_font_families();
	for( int i=0; i<fontfamiliescnt; i++ )
	{
		font_family family;
		if( get_font_family(i,&family) >= B_NO_ERROR )
		{
			printf( "Family: \"%s\"", family );
			int fontstylescnt = count_font_styles( family );
			if( fontstylescnt )
				printf( "   style%s:", fontstylescnt>1?"s":"" );
			for( int j=0; j<fontstylescnt; j++ )
			{
				font_style style;
				if( get_font_style(family,j,&style) >= B_NO_ERROR )
				{
					printf( " \"%s\"", style );
				}
			}
			printf( "\n" );
		}
	}
}

void ListTranslators()
{
	printf( "available imageformats:\n" );
	
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	assert( roster != NULL );

	int32 translators_cnt = 0;
	translator_id *translators = NULL;
	roster->GetAllTranslators( &translators, &translators_cnt );
	for( int i=0; i<translators_cnt; i++ )
	{
		const char *translator_name;
		const char *translator_info;
		int32 translator_version;
		roster->GetTranslatorInfo( translators[i], &translator_name, &translator_info, &translator_version );

		bool supportsbitmap = false;
		int32 informats_cnt = 0;
		const translation_format *informats = NULL;
		roster->GetInputFormats( translators[i], &informats, &informats_cnt );
		for( int iin=0; iin<informats_cnt; iin++ )
		{
			if( informats[iin].type==B_TRANSLATOR_BITMAP && informats[iin].group==B_TRANSLATOR_BITMAP )
			{
				supportsbitmap = true;
				break;
			}
		}
		
		if( supportsbitmap )
		{
			int32 outformats_cnt = 0;
			const translation_format *outformats = NULL;
			roster->GetOutputFormats( translators[i], &outformats, &outformats_cnt );
			for( int iout=0; iout<informats_cnt; iout++ )
			{
				if( outformats[iout].type!=B_TRANSLATOR_BITMAP && outformats[iout].group==B_TRANSLATOR_BITMAP )
				{
					printf( "%s : %s\n", outformats[iout].MIME, outformats[iout].name );
				}
			}
		}
	}

	delete translators;
}	
	
	


void Usage( const char *argv0 )
{
	fprintf( stderr, "Usage: %s [OPTION]... <string> <dstimage>\n", argv0 );
	fprintf( stderr, "\n" );
	fprintf( stderr, "  -h, --help                display this help and exit\n" );
	fprintf( stderr, "  -v, --verbose             explain what is being done\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "      --fontfamily=FAMILY   name of the font to draw the string with\n" );
	fprintf( stderr, "      --fontstyle=STYLE     style of font to draw the string with\n" );
	fprintf( stderr, "  -s, --fontsize=HEIGHT     height if font in pixels\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "  -x, --xsize=WIDTH         width of the generated image\n" );
	fprintf( stderr, "  -y, --ysize=HEIGHT        height of the generated image\n" );
	fprintf( stderr, "      --padleft=PIXELS      add/remove PIXELS pixels from left size\n" );
	fprintf( stderr, "      --padright=PIXELS     add/remove PIXELS pixels from right size\n" );
	fprintf( stderr, "      --padtop=PIXELS       add/remove PIXELS pixels from the top\n" );
	fprintf( stderr, "      --padbottom=PIXELS    add/remove PIXELS pixels from the bottom\n" );
	fprintf( stderr, "      --imageformat=MIME    mime type of generated image\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "      --halign=ALIGN        horizontal alignment of text\n" );
	fprintf( stderr, "                              possible values: left, center and right\n" );
	fprintf( stderr, "      --valign=ALIGN        vertical alignment of text\n" );
	fprintf( stderr, "                              possible values: top, center and bottom\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "      --backcolor=COLOR     background color of the bitmap\n" );
	fprintf( stderr, "      --textcolor=COLOR     color of the text\n" );
	fprintf( stderr, "                              colors are in html format (#rrggbb)\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "      --listformats         list all available image formats\n" );
	fprintf( stderr, "      --listfonts           list all available fonts\n" );
	fprintf( stderr, "\n" );
}

int main( int argc, char **argv )
{
	status_t status;
	BApplication app( "application/x-vnd.DamnRednex-TextImage" );
	
	static struct option long_options[] = {
		{"help",		0, NULL, 'h'},
        {"verbose",		0, NULL, 'v'},

        {"fontfamily",	1, NULL, 'ffam'},
        {"fontstyle",	1, NULL, 'fstl'},
        {"fontsize",	1, NULL, 's'},

        {"imageformat",	1, NULL, 'imgf'},

        {"xsize",		1, NULL, 'x'},
        {"ysize",		1, NULL, 'y'},

        {"padleft",		1, NULL, 'padl'},
        {"padright",	1, NULL, 'padr'},
        {"padtop",		1, NULL, 'padt'},
        {"padbottom",	1, NULL, 'padb'},

        {"halign",		1, NULL, 'hora'},
        {"valign",		1, NULL, 'vera'},

        {"backcolor",	1, NULL, 'bckc'},
        {"textcolor",	1, NULL, 'txtc'},

        {"listformats",	0, NULL, 'lfmt'},
        {"listfonts",	0, NULL, 'lfnt'},
        {NULL,			0, NULL, 0}};

	int imagewidth = -1;
	int imageheight = -1;

	int padleft = 0;
	int padright = 0;
	int padtop = 0;
	int padbottom = 0;

	const char *fontfamily = NULL;
	const char *fontstyle = NULL;
	int fontheight = -1;

	bool verbose = false;

	int horalign = 0; // <0:left, 0:center, >0:right
	int veralign = 0;
	
	rgb_color backcolor = { 255, 255, 255, 255 };
	rgb_color textcolor = { 0, 0, 0, 255 };
	
	char mimetype[B_MIME_TYPE_LENGTH] = "\0";

    int option;
    int option_index = 0;
    while( (option=getopt_long(argc, argv, "hx:y:s:v", long_options, &option_index)) != EOF )
    {
        switch( option )
        {
            case 'h':
                Usage( argv[0] );
                exit( 0 );

            case 'x':
            	imagewidth = atoi( optarg );
                if( imagewidth <= 0 )
				{
					fprintf( stderr, "Illegal image width: %d\n", imagewidth );
					Usage( argv[0] );
					exit( 1 );
				}
                break;

            case 'y':
            	imageheight = atoi( optarg );
                if( imageheight <= 0 )
				{
					fprintf( stderr, "Illegal image height: %d\n", imageheight );
					Usage( argv[0] );
					exit( 1 );
				}
                break;

			case 's':
            	fontheight = atoi( optarg );
                if( fontheight <= 0 )
				{
					fprintf( stderr, "Illegal font height: %d\n", fontheight );
					Usage( argv[0] );
					exit( 1 );
				}
                break;

			case 'v':
				verbose = true;
				break;
				
			case 'padl':
            	padleft = atoi( optarg );
				break;
			case 'padr':
            	padright = atoi( optarg );
				break;
			case 'padt':
            	padtop = atoi( optarg );
				break;
			case 'padb':
            	padbottom = atoi( optarg );
				break;
				
			case 'imgf':
				strncpy( mimetype, optarg, sizeof(mimetype) );
				break;

			case 'hora': // halign
				if( strcasecmp(optarg,"left") == 0 )		horalign = -1;
				else if( strcasecmp(optarg,"center") == 0 )	horalign = 0;
				else if( strcasecmp(optarg,"right") == 0 )	horalign = 1;
				else
				{
					fprintf( stderr, "Illegal horizontal alingment: %s\n", optarg );
					Usage( argv[0] );
					exit( 1 );
				}
				break;

			case 'vera': // valign
				if( strcasecmp(optarg,"top") == 0 )			veralign = -1;
				else if( strcasecmp(optarg,"center") == 0 )	veralign = 0;
				else if( strcasecmp(optarg,"bottom") == 0 )	veralign = 1;
				else
				{
					fprintf( stderr, "Illegal vertical alingment: %s\n", optarg );
					Usage( argv[0] );
					exit( 1 );
				}
				break;
				
			case 'bckc': // backcolor;
			{
				int r,g,b;
				if( sscanf(optarg,"#%02x%02x%02x", &r,&g,&b) != 3 )
				{
					fprintf( stderr, "Illegal color: %s\n", optarg );
					Usage( argv[0] );
					exit( 1 );
				}
				backcolor.red = r;
				backcolor.green = g;
				backcolor.blue = b;
				break;
			}
			case 'txtc': // textcolor;
			{
				int r,g,b;
				if( sscanf(optarg,"#%02x%02x%02x", &r,&g,&b) != 3 )
				{
					fprintf( stderr, "Illegal color: %s\n", optarg );
					Usage( argv[0] );
					exit( 1 );
				}
				textcolor.red = r;
				textcolor.green = g;
				textcolor.blue = b;
				break;
			}
				
			case 'ffam': // fontfamily
				fontfamily = optarg;
				break;
			case 'fstl': // fontstyle
				fontstyle = optarg;
				break;

			case 'lfmt':
				ListTranslators();
				exit( 0 );
			case 'lfnt':
				ListFonts();
				exit( 0 );

			default:
                Usage( argv[0] );
				exit( 1 );
		}
	}

	if( argc-optind != 2 )
    {
        fprintf( stderr, "Too many/few arguments\n" );
		Usage( argv[0] );
		exit( 1 );
    }                   

	const char *string = argv[optind++];
	const char *destfilename = argv[optind++];
	

//
// FIGURE OUT MIMETYPE FOR DEST IMAGE
//
	if( mimetype[0] == '\0' )
	{
		// guess filetype
		BMimeType *mime = damn::GuessMimetypeForExtension( destfilename );
		if( mime == NULL )
		{
			fprintf( stderr, "Can not guess the mimetype for %s, please use the --imageformat options\n", destfilename );
			exit( 1 );
		}
		strncpy( mimetype, mime->Type(), sizeof(mimetype) );
		delete mime;
	}


//
// FIGURE OUT WHAT TRANSLATOR WE SHOULD USE
//
	
	BMessage translators;
	damn::FindTranslatorsFor( BTranslatorRoster::Default(), B_TRANSLATOR_BITMAP, B_TRANSLATOR_BITMAP, mimetype, &translators );
	translator_id out_translator;
	uint32 out_type;
	if( translators.FindInt32("translators",(int32*)&out_translator)!=B_NO_ERROR || translators.FindInt32("types",(int32*)&out_type)!=B_NO_ERROR)
	{
		fprintf( stderr, "Mimetype %s is not supported\n", mimetype );
		exit( 1 );
	}


//
// CREATE FONT
//

	BFont font;
	status = font.SetFamilyAndStyle( fontfamily, fontstyle );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Illegal font family or style: \"%s\" \"%s\"\n", fontfamily?fontfamily:"default", fontstyle?fontstyle:"default" );
#if 1
		if( font.SetFamilyAndStyle(fontfamily, NULL) != B_NO_ERROR )
			font.SetFamilyAndStyle( NULL, NULL );
#else
		exit( 1 );
#endif
	}
	if( fontheight > 0 )
		font.SetSize(fontheight );
	
	if( verbose )
	{
		font_family family;
		font_style style;
		font.GetFamilyAndStyle( &family, &style );
		printf( "Selected font, family:\"%s\" style:\"%s\" size:%.1f\n", family, style, font.Size() );
	}

//
// FIND BOUNDING BOX OF FONT
//
	// get string width (and height)
	escapement_delta edeltas = {0,0};
	BRect rect;
	font.GetBoundingBoxesForStrings( &string, 1 , B_SCREEN_METRIC, &edeltas, &rect );
	BPoint textpos( -rect.left, -rect.top );
	rect.OffsetTo( 0, 0 );
	// get font height
	font_height fheight;
	font.GetHeight( &fheight );
	rect.bottom = rect.top + fheight.ascent + fheight.descent;
	textpos.y = fheight.ascent;

	if( verbose )
		printf( "Bounding box for string: %ldx%ld\n", rect.IntegerWidth()+1, rect.IntegerHeight()+1 );

	if( imagewidth < 0 )	imagewidth = rect.IntegerWidth()+1;
	if( imageheight < 0 )	imageheight = rect.IntegerHeight()+1;

	// align horizontal
	if( horalign < 0 ) ;
	else if( horalign == 0 ) textpos.x += (imagewidth-rect.Width()+1)/2.0f;
	else textpos.x += imagewidth-(rect.IntegerWidth()+1);

	// align vertical
	if( veralign < 0 ) ;
	else if( veralign == 0 ) textpos.y += (imageheight-rect.Height()+1)/2.0f;
	else textpos.y += imageheight-(rect.IntegerHeight()+1);
	
	imagewidth += padleft + padright;
	imageheight += padtop + padbottom;
	textpos.x += padleft;
	textpos.y += padtop;


//
// CREATE BITMAP
//

	BBitmap *bitmap = new BBitmap( BRect(0,0,imagewidth-1,imageheight-1), B_RGBA32, true );
	if( bitmap == NULL || !bitmap->IsValid() )
	{
		fprintf( stderr, "Could not create bitmap, maybe too large?\n" );
		exit( 1 );
	}

	BView *view = new BView( bitmap->Bounds(), "", B_FOLLOW_ALL, B_WILL_DRAW );
	bitmap->Lock();
	bitmap->AddChild( view );
	view->SetFont( &font );
	view->SetHighColor( backcolor );
	view->SetLowColor( backcolor );
//	view->SetHighColor( 0,0,0 );
//	view->SetLowColor( 0,0,0 );
	view->FillRect( view->Bounds() );
	view->SetHighColor( textcolor );
//	view->SetHighColor( 255,255,255 );
	view->DrawString( string, textpos );
	view->Sync();
	bitmap->Unlock();
/*	
	for( int iy=0; iy<imageheight; iy++ )
	{
		uint32 *scanline = (uint32*)(((uint8*)bitmap->Bits())+bitmap->BytesPerRow()*iy);
		for( int ix=0; ix<imagewidth; ix++ )
		{
			uint32 col = scanline[ix];
			uint8 alpha = (col>>16)&0xff; // red
			col = 0x00ffffff | alpha<<24;
			scanline[ix] = col;
		}
	}
*/

//
// WRITE IMAGE
//

	BFile destfile( destfilename, B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE );
	if( destfile.InitCheck() != B_NO_ERROR )
	{
		fprintf( stderr, "Could not open file %s: %s\n", destfilename, strerror(destfile.InitCheck()) );
		exit( 1 );
	}

	BBitmapStream input( bitmap );
	status = BTranslatorRoster::Default()->Translate( out_translator, &input, NULL, &destfile, out_type );
//	status = BTranslatorRoster::Default()->Translate( &input, NULL, NULL, &destfile, 'bits' );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Could not write image %s: %s\n", destfilename, strerror(status) );
		exit( 1 );
	}

	return 0;
}
