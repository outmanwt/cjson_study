#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myjson.h"


int main() 
{
	json_value v;
	const char *json = "c ";
	json_error err;
	v.type = JSON_FALSE;
	err = json_parse ( &v , json );
	fprintf ( stderr , "%d\n", err); 
	return 0;
}
