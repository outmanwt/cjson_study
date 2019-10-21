#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myjson.h"


int main() 
{
	json_value v;
	v.type = JSON_FALSE;
	const char *json = "c ";
	json_error err = json_parse ( &v , json );
	fprintf ( stderr , "%d\n", err); 
	return 0;
}
