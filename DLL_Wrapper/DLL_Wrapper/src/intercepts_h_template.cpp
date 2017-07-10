#include "stdafx.h"

/*
	fprintf...
	%u	-	call_table_size
*/
const char		*intercepts_h_header = R"INTERCEPTSHEADER(
/*
** Header generated by DLL_Wrapper.
**
** This file contains function declerations for each intercepted DLL function.
** 
*/
#pragma once
#include <stdio.h>
#include <Windows.h>
#include "forwards.h"

#define	INTERCEPTED_API __declspec(dllexport)

extern "C" void		*(c_ext_intercepts[%u]);

)INTERCEPTSHEADER";


/*
	fprintf...
	%s	-	return type
	%s	-	function name
	%s	-	paramater type(s) + name(s)
	%s	-	return type
	%s	-	function bridge name
	%s	-	paramater type(s) + name(s)
*/
const char		*intercepts_h_function = R"INTERCEPTFNCT(

extern "C" INTERCEPTED_API %s		%s(%s);
extern "C" %s						%s(%s);

)INTERCEPTFNCT";