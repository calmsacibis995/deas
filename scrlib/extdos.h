//
// MSDOS compatibility declarations.
//
// Copyright (C) 1994 Cronyx Ltd.
// Author: Serge Vakulenko, <vak@zebub.msk.su>
//
// This software is distributed with NO WARRANTIES, not even the implied
// warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Authors grant any other persons or organisations permission to use
// or modify this software as long as this message is kept with the software,
// all derivative works or modified versions.
//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

inline int lowercase (int c)
{
	if (c>='A' && c<='Z')
		return (c+'a'-'A');
	switch (c) {
	case 'А': return ('а');
	case 'Б': return ('б');
	case 'В': return ('в');
	case 'Г': return ('г');
	case 'Д': return ('д');
	case 'Е': return ('е');
	case 'Ж': return ('ж');
	case 'З': return ('з');
	case 'И': return ('и');
	case 'Й': return ('й');
	case 'К': return ('к');
	case 'Л': return ('л');
	case 'М': return ('м');
	case 'Н': return ('н');
	case 'О': return ('о');
	case 'П': return ('п');
	case 'Р': return ('р');
	case 'С': return ('с');
	case 'Т': return ('т');
	case 'У': return ('у');
	case 'Ф': return ('ф');
	case 'Х': return ('х');
	case 'Ц': return ('ц');
	case 'Ч': return ('ч');
	case 'Ш': return ('ш');
	case 'Щ': return ('щ');
	case 'Ъ': return ('ъ');
	case 'Ы': return ('ы');
	case 'Ь': return ('ь');
	case 'Э': return ('э');
	case 'Ю': return ('ю');
	case 'Я': return ('я');
	}
	return (c);
}
