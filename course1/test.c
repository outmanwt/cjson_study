#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myjson.h"


int main() 
{
	json_value v;
	char *str[]= {"JSON_OK","JSON_ONLY_BLANK","JSON_INPUT_ERROR","JSON_VALUE_ERROR"};
	
	v.type = JSON_FALSE;
	fprintf ( stderr , "%s:%s\n", "",str[json_parse ( &v , "" )]); 
	fprintf ( stderr , "%s:%s\n", " ", str[json_parse ( &v , " " )]); 
	fprintf ( stderr , "%s:%s\n", "Null",str[json_parse ( &v , "Null" )]); 
	fprintf ( stderr , "%s:%s\n", "null",str[json_parse ( &v , "null" )]); 
	fprintf ( stderr , "%s:%s\n", " null",str[json_parse ( &v , " null" )]); 
	fprintf ( stderr , "%s:%s\n", "nullx",str[json_parse ( &v , "nullx" )]); 
	fprintf ( stderr , "%s:%s\n", "ac",str[json_parse ( &v , "ac" )]); 
	fprintf ( stderr , "%s:%s\n", "ture",str[json_parse ( &v , "ture" )]); 
	fprintf ( stderr , "%s:%s\n", "false",str[json_parse ( &v , "false" )]); 
	return 0;
}
