#include <lib9.h>

char*
utfecpy(char *to, char *e, const char *from)
{
	char *end;

	if(to >= e)
		return to;
	end = _memccpy(to, from, '\0', e - to);
	if(end == nil){
		end = e;
		while(end>to && (*--end&0xC0)==0x80)
			;
		*end = '\0';
	}else{
		end--;
	}
	return end;
}
