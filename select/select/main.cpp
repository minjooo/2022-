

#pragma once
#include "typedef.h"
#include "Select.h"


UxInt32 main( UxInt32 argc, UxInt8* argv[] ) 
{
	Select select;

	UxInt16 inputPort { 0 };
	UxBool	inputPortFlag { false };
	if ( argc > 1 )
	{
		inputPort = (UxInt16)( atoi( argv[1] ) );
		inputPortFlag = true;
	}

	if ( inputPortFlag )
	{
		select.Initialize(inputPort);
	}
	else
	{
		select.Initialize();
	}
	select.Run();
}
