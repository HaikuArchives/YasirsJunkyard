// Copyright 1999 Jesper Hansen. Free for use under the Gnu Public License
//-----------------------------------------------------------------------------
#include <assert.h>
//-------------------------------------
#include <app/Application.h>
#include <storage/File.h>
//-------------------------------------
#include "gfx/WriteGifImage.h"
//-----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	BApplication app( "application/x-vnd.DamnRednex-GifImageTest" );

	assert( argc == 2 );
	BFile file( argv[1], B_CREATE_FILE|B_ERASE_FILE|B_WRITE_ONLY );
	assert( file.InitCheck() == B_NO_ERROR );

#define SIZE 255

	uint8 data[SIZE*SIZE];

	damn::WriteGifImage image( &file );
	image.WriteHeader( SIZE, SIZE );
	for( int i=0; i<255; i++ )
	{
		for( uint y=0; y<SIZE; y++ )
			for( uint x=0; x<SIZE; x++ )
				data[x+y*SIZE] = x+y+i;
	
		image.WriteImage( data, SIZE, SIZE, SIZE, 0, 0 );
		if( i!=9 )
			image.WriteExtensionBlock(  0, 0, -1, 1 );
	}
//	image.WriteExtensionBlock( 0, 0, -1, 2 );
	image.WriteEOF();

	return 0;
};

//-----------------------------------------------------------------------------
