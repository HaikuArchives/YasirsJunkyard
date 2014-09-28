Yasir's Junkyard
============================
A collection of small BeOS programs and plugins.

Originally from: http://sourceforge.net/projects/yasirsjunkyard/  
Website: http://yasirsjunkyard.sourceforge.net/  
License: BSD

BUILDING:
	First you must install Bake, you can get it here:
	http://www.bebits.com/app/734

	Then run the setupenv.sh script like this :
	". setupenv.sh".
	
	You can bake one or more of the following targets:
	
	"bake all" : This will build all targets in the source tree
	"bake install" : This will build and install:
		HSIRawTranslator, AmigaIconTranslator,
		SimpleSlide, MakeThumbIcon, textimage,
		w8738x driver, and the optrex lcd driver.

		

DOCUMENTATION:
	If you have php* installed, you can build, and read it by running
	"bake gfx/textimage/install" and
	"bake documentation/readhtml"
	
	php* :The "Beta 3 - beos-3" version of php that is on www.bebits.com
	has several bugs and will not work...
