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
#include <fcntl.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//-------------------------------------
#include <kernel/OS.h>
#include <storage/VolumeRoster.h>
//-------------------------------------
#include "optrex/optrex_driver.h"
#include "w8378x/w8378x_driver.h"

//-----------------------------------------------------------------------------

/*
----------------
Mem:alloc/comit
*/

//-----------------------------------------------------------------------------

void UploadBarGraphChars( int optrix )
{
	ioctl( optrix, OPTREX_UPLOADGLYP0, "\x10\x10\x10\x10\x10\x10\x10\x10" );
	ioctl( optrix, OPTREX_UPLOADGLYP1, "\x18\x18\x18\x18\x18\x18\x18\x18" );
	ioctl( optrix, OPTREX_UPLOADGLYP2, "\x1c\x1c\x1c\x1c\x1c\x1c\x1c\x1c" );
	ioctl( optrix, OPTREX_UPLOADGLYP3, "\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e" );

	ioctl( optrix, OPTREX_UPLOADGLYP4, "\x01\x01\x01\x01\x01\x01\x01\x01" );
	ioctl( optrix, OPTREX_UPLOADGLYP5, "\x03\x03\x03\x03\x03\x03\x03\x03" );
	ioctl( optrix, OPTREX_UPLOADGLYP6, "\x07\x07\x07\x07\x07\x07\x07\x07" );
	ioctl( optrix, OPTREX_UPLOADGLYP7, "\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f" );
}

void PrintBarGraph( char *dst, int length, float value, bool base, int dir )
{
	const char barright[7] = "\x20\x00\x01\x02\x03\xff";
	const char barleft[7] = "\x20\x04\x05\x06\x07\xff";

	if( value < 0.0f ) value = 0.0f;
	if( value > 1.0f ) value = 1.0f;
	
	int pixcnt;
	if( base )
		pixcnt = int((length*5-1)*value + 1.0f + 0.5f);
	else
		pixcnt = int((length*5)*value + 0.5f);
	
	if( dir == 0 )
	{
		// right
		for( int i=0; i<length; i++ )
		{
			if( pixcnt <= 0 ) 		dst[i] = barright[0];
			else if( pixcnt >= 5 )	dst[i] = barright[5];
			else					dst[i] = barright[pixcnt];
			pixcnt -= 5;
		}
	}
	else
	{
		for( int i=0; i<length; i++ )
		{
			if( pixcnt <= 0 )		dst[length-1-i] = barleft[0];
			else if( pixcnt >= 5 )	dst[length-1-i] = barleft[5];
			else					dst[length-i-1] = barleft[pixcnt];
			pixcnt -= 5;
		}
	}
}

//-----------------------------------------------------------------------------

// note: it always rounds up, this should be an option (roundup,rounddown,roundnear)
void PrintSize( char *dst, int length, uint64 value )
{
	assert( length>=2 && length<=16 );

	const char postfix_list[] = "bkMGT???";
	int postfix_index = 0;

	long double fvalue = value;
	while( fvalue >= 1000 )
	{
		fvalue /= 1024;
		postfix_index++;
	}

	char postfix = postfix_list[postfix_index];

	char string[17];
	if( length == 2 )
	{
		if( ceil(fvalue) < 10 )	sprintf( string, "%d%c", int(ceil(fvalue)), postfix );
		else					sprintf( string, "1%c", postfix_list[postfix_index+1] );
	}
	else if( length == 3 )
	{
		if( ceil(fvalue) < 10 )				sprintf( string, " %d%c", int(ceil(fvalue)), postfix );
		else if( ceil(fvalue) < 100 )		sprintf( string, "%d%c", int(ceil(fvalue)), postfix );
		else if( ceil(fvalue*10) < 10000 )	sprintf( string, ".%d%c", int(ceil(fvalue))/100, postfix_list[postfix_index+1] );
		else								sprintf( string, " 1%c", postfix_list[postfix_index+1] );
	}
	else if( length == 4 )
	{
		if( ceil(fvalue*10) < 100 )	sprintf( string, "%1.1f%c", ceil(fvalue*10.0)/10.0, postfix );
		else						sprintf( string, "%3.0f%c", ceil(fvalue), postfix );
	}
	else if( length == 5 )
	{
		if( ceil(fvalue*100) < 1000 )		sprintf( string, "%1.2f%c", ceil(fvalue*100.0)/100.0, postfix );
		else if( ceil(fvalue*10) < 1000 )	sprintf( string, "%2.1f%c", ceil(fvalue*10.0)/10.0, postfix );
		else								sprintf( string, "% 3.0f%c", ceil(fvalue), postfix );
	}
	else
	{
		float r1 = pow(10.0,length-5);
		float r2 = r1*10.0;
		float r3 = r2*10.0;
		float r4 = r3*10.0;
		if( ceil(fvalue*r3) < r4 )		sprintf( string, "%1.*f%c", length-3, ceil(fvalue*r3)/r3, postfix );
		else if( ceil(fvalue*r2) < r4 )	sprintf( string, "%2.*f%c", length-4, ceil(fvalue*r2)/r2, postfix );
		else							sprintf( string, "%3.*f%c", length-5, ceil(fvalue*r1)/r1, postfix );
	}
	memcpy( dst, string, length );
}

//-----------------------------------------------------------------------------

class Stat
{
public:
	Stat() {};
	virtual ~Stat() {};
	virtual	const char *Update( int step, int stepcnt ) =0;
protected:
	char	fData[32];
};

//--------------------------------------

class StatDate : public Stat
{
public:
//	StatDate() : Stat() {}
//	~StatDate() {}
	const char *Update( int step, int stepcnt )
	{
		if( step != 0 ) return NULL;
		// Mon 22 May 22:33
		const char *daystr[7]= { "Sun","Mon","Tue","Web","Thu","Fri", "Sat" };
		const char *monstr[12]= { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
		time_t t = time(NULL);
		struct tm *tmtime = localtime(&t);
		sprintf( fData, "%s %2d %s %02d:%02d", daystr[tmtime->tm_wday], tmtime->tm_mday, monstr[tmtime->tm_mon], tmtime->tm_hour, tmtime->tm_min );
		return fData;
	}
};

//--------------------------------------

class StatUp : public Stat
{
public:
//	StatUp() : Stat() {}
//	~StatUp() {}
	const char *Update( int step, int stepcnt )
	{
		// Up: 11d 11:22.33
		bigtime_t uptime = system_time();
		uptime /= 1000000;
		int upsecs = uptime%60;
		uptime /= 60;
		int upmins = uptime%60;
		uptime /= 60;
		int uphours = uptime%24;
		uptime /= 24;
		int updays = uptime;
		
		sprintf( fData, "Up:%3dd %2d:%02d.%02d", updays, uphours, upmins, upsecs );
		return fData;
	}
};

//--------------------------------------

class StatIdle : public Stat
{
public:
//	StatIdle() : Stat() {}
//	~StatIdle() {}
	const char *Update( int step, int stepcnt )
	{
		if( step != 0 ) return NULL;
		// Idle:100.00%
		system_info nowinfo;
		bigtime_t nowtime;
		get_system_info( &nowinfo );
		nowtime = system_time();
		bigtime_t usetime = 0;
		for( int i=0; i<nowinfo.cpu_count; i++ )
			usetime += nowinfo.cpu_infos[0].active_time/nowinfo.cpu_count;
		sprintf( fData, "Idle: %6.2f%%   ", 100.0f - (float(usetime)*100.0f/float(nowtime)) );
		return fData;
	}
};

//--------------------------------------

class StatCpuTemp : public Stat
{
public:
	StatCpuTemp( int winbond ) : Stat(), fWinbond(winbond) {}
	~StatCpuTemp() {}
	const char *Update( int step, int stepcnt )
	{
		if( step != 0 ) return NULL;
		// Cpu:-xx.x*-xx.x*
		int temp2=0, temp3=0;
		ioctl( fWinbond, W8378x_READ_TEMP1, &temp2 );
		ioctl( fWinbond, W8378x_READ_TEMP2, &temp3 );
		sprintf( fData, "Cpu:%5.1f\xdf%5.1f\xdf", float(temp2)/256.0f, float(temp3)/256.0f );
		return fData;
	}
private:
	int fWinbond;
};

class StatMoboTemp : public Stat
{
public:
	StatMoboTemp( int winbond ) : Stat(), fWinbond(winbond) {}
	~StatMoboTemp() {}
	const char *Update( int step, int stepcnt )
	{
		if( step != 0 ) return NULL;
		// Mobo: -xx.x*
		int temp1=0;
		ioctl( fWinbond, W8378x_READ_TEMP3, &temp1 );
		sprintf( fData, "Mobo: %5.1f\xdf    ", float(temp1)/256.0f );
		return fData;
	}
private:
	int fWinbond;
};

//--------------------------------------

class StatFanSpeed : public Stat
{
public:
	StatFanSpeed( int winbond, int fan ) : Stat(), fWinbond(winbond), fFan(fan) {}
	~StatFanSpeed() {}
	const char *Update( int step, int stepcnt )
	{
		if( step != 0 ) return NULL;
		// Fan1: 4444rpm
		int fanrpm=0;
		ioctl( fWinbond, W8378x_READ_FAN1+fFan, &fanrpm );
		sprintf( fData, "Fan%d: %4drpm   ", fFan+1, fanrpm );
		return fData;
	}
private:
	int fWinbond;
	int	fFan;
};

//--------------------------------------

class StatDiskFree : public Stat
{
public:
	const char *Update( int step, int stepcnt )
	{
		if( step != 0 ) return NULL;
		BVolume vol;
		do
		{
			if( fVolRoster.GetNextVolume(&vol) != B_NO_ERROR )
				fVolRoster.Rewind();
		}
		while( vol.InitCheck()!=B_NO_ERROR || vol.Capacity()<=0 );

		char volname[B_FILE_NAME_LENGTH];
		if( vol.GetName(volname) != B_NO_ERROR )
			strcpy( volname, "{noname}" );
		sprintf( fData, "%-8.8s:", volname );
		PrintSize( fData+9, 7, vol.FreeBytes() );
		return fData;
	}
private:
	BVolumeRoster	fVolRoster;
};

//-----------------------------------------------------------------------------

int main()
{
	int optrix = open( "/dev/misc/optrex", O_RDWR );
	assert( optrix >= 0 );

	int windbond = open( "/dev/misc/w8378x", O_RDWR );
	assert( windbond >= 0 );

	UploadBarGraphChars( optrix );
	
	system_info lastinfo;
	bigtime_t lasttime;
	int cpuupdate = 0;
	get_system_info( &lastinfo );
	lasttime = system_time();

	int statindex = 0;
	int updatestep = 0;

	char disp[2][16];
	memset( disp, ' ', 16*2 );
	
	BList statlist;
	statlist.AddItem( new StatDate() );
	statlist.AddItem( new StatUp() );
	statlist.AddItem( new StatIdle() );
	statlist.AddItem( new StatCpuTemp(windbond) );
	statlist.AddItem( new StatMoboTemp(windbond) );
//	statlist.AddItem( new StatFanSpeed(windbond,0) );
//	statlist.AddItem( new StatFanSpeed(windbond,1) );
//	statlist.AddItem( new StatFanSpeed(windbond,2) );
	statlist.AddItem( new StatDiskFree() );
	
	while( 1 )
	{
		// CPU load:
		if( cpuupdate++ == 2 )
		{
			system_info nowinfo;
			bigtime_t nowtime;
			get_system_info( &nowinfo );
			nowtime = system_time();
			if( nowinfo.cpu_count == 1 )
			{
				float load0 = float(nowinfo.cpu_infos[0].active_time-lastinfo.cpu_infos[0].active_time) / float(nowtime-lasttime);
				disp[0][0] = '0';
				PrintBarGraph( &disp[0][1], 15, load0, true, 0 );
			}
			else if( nowinfo.cpu_count == 2 )
			{
				float load0 = float(nowinfo.cpu_infos[0].active_time-lastinfo.cpu_infos[0].active_time) / float(nowtime-lasttime);
				float load1 = float(nowinfo.cpu_infos[1].active_time-lastinfo.cpu_infos[1].active_time) / float(nowtime-lasttime);
				disp[0][0] = '0';
				disp[0][8] = '1';
				PrintBarGraph( &disp[0][1], 7, load0, true, 0 );
				PrintBarGraph( &disp[0][9], 7, load1, true, 0 );
			}
			lastinfo = nowinfo;
			lasttime = nowtime;

			cpuupdate = 0;
		}

		assert( statindex < statlist.CountItems() );
		Stat *s = (Stat*)statlist.ItemAt( statindex );
		const char *line2 = s->Update( updatestep, 25 );
		if( line2 ) strncpy( disp[1], line2, 16 );
		if( ++updatestep >= 25 )
		{
			if( ++statindex >= statlist.CountItems() )
				statindex = 0;
			updatestep = 0;
		}
		
		write( optrix, disp, 16*2 );
		snooze( 1000000/10 );
	}	

	close( windbond );
	close( optrix );
	return 0;
}

