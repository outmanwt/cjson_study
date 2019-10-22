#include "myjson.h"
#include <assert.h>
#include <stdlib.h>
/*JSON格式
	JSON-text = ws value ws
	ws = *(%x20 / %x09 / %x0A / %x0D) 	
	value = null / false / true 
	null  = "null"
	false = "false"
	true  = "true"
	
	当中 %xhh 表示以 16 进制表示的字符，/是多选一，* 是零或多个，( ) 用于分组。
	那么第一行的意思是，JSON 文本由 3 部分组成，首先是空白（whitespace），接着是一个值，最后是空白。
	第二行告诉我们，所谓空白，是由零或多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）所组成。
*/
void read_blank_first ( const char **c );
json_error json_parse_null(const char *c,json_value *v);
json_error read_value(const char *c,json_value *v);
json_error json_parse_true(const char *c,json_value *v);
json_error json_parse_false(const char *c,json_value *v);
/*解析!int可以改个名字*/
json_error json_parse(json_value *v,const char *json)
{
	const char *c = json;
	assert(v!=NULL);
	v->type = JSON_NULL;
	/*读空格*/
	read_blank_first(&c);
	/*读值*/
	return read_value(c,v);	
}

json_error json_parse_null(const char *c,json_value *v)
{
	/*再检查一遍c是否为n?有必要？*/
	assert(*c=='n');
	if(c[1]!='u'||c[2]!='l'||c[3]!='l')
		return JSON_INPUT_ERROR;
	c+=4;
	v->type = JSON_NULL;
	return JSON_OK;
}

json_error json_parse_true(const char *c,json_value *v)
{
	/*再检查一遍c是否为n?有必要？*/
	assert(*c=='t');
	if(c[1]!='u'||c[2]!='r'||c[3]!='e')
		return JSON_INPUT_ERROR;
	c+=4;
	v->type = JSON_NULL;
	return JSON_OK;
}

json_error json_parse_false(const char *c,json_value *v)
{
	/*再检查一遍c是否为n?有必要？*/
	assert(*c=='f');
	if(c[1]!='a'||c[2]!='l'||c[3]!='s'||c[4]!='e')
		return JSON_INPUT_ERROR;
	c+=5;
	v->type = JSON_NULL;
	return JSON_OK;
}

/*读空格*/
json_error read_value(const char *c,json_value *v)
{
	switch(*c)
	{
		case 'n':	return json_parse_null(c,v);
		case 't':   return json_parse_true(c,v);
        case 'f':   return json_parse_false(c,v);
		case '\0': 	return JSON_ONLY_BLANK;
		default : 	return JSON_INPUT_ERROR;
	}
}
void read_blank_first ( const char **c )
{
	while ( **c == ' ' || **c == '\t' || **c == '\n' || **c == '\r' )
		*c = ( void * ) ( *c + 1 );
}

/*获取类型用来检验*/
json_type json_get_type ( const json_value *v )
{
	assert ( v != NULL );
	return v->type;
}
