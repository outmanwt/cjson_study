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
/*json结构体*/
typedef struct {
    const char* json;
	double n;
}json_struct;

static json_error json_parse_null(json_struct *c,json_value *v)
{
	/*再检查一遍c是否为n?有必要？*/
	assert(*c->json=='n');
	if(c->json[1]!='u'||c->json[2]!='l'||c->json[3]!='l')
		return JSON_INPUT_ERROR;
	c->json+=4;
	v->type = JSON_NULL;
	return JSON_OK;
}

static json_error json_parse_true ( json_struct *c , json_value *v )
{
	/*再检查一遍c是否为n?有必要？*/
	assert(*c->json=='t');
	if(c->json[1]!='r'||c->json[2]!='u'||c->json[3]!='e')
		return JSON_INPUT_ERROR;
	c->json+=4;
	v->type = JSON_TRUE;
	return JSON_OK;
}

static json_error json_parse_false ( json_struct *c , json_value *v )
{
	/*再检查一遍c是否为n?有必要？*/
	assert(*c->json=='f');
	if(c->json[1]!='a'||c->json[2]!='l'||c->json[3]!='s'||c->json[4]!='e')
		return JSON_INPUT_ERROR;
	c->json+=5;
	v->type = JSON_FALSE;
	return JSON_OK;
}

/*读空格*/
static json_error read_value ( json_struct *c , json_value *v )
{
	switch(*c->json)
	{
		case 'n':	return json_parse_null(c,v);
		case 't':   return json_parse_true(c,v);
        case 'f':   return json_parse_false(c,v);
		case '\0': 	return JSON_ONLY_BLANK;
		default : 	return JSON_INPUT_ERROR;
	}
}
static void read_blank_first ( json_struct *c )
{
	const char* temp = c->json;
	while ( *temp == ' ' || *temp == '\t' || *temp == '\n' || *temp == '\r' )
		temp ++;
	c->json = temp;
}

/*解析*/
json_error json_parse ( json_value *v , const char *json )
{
	json_struct c;
	json_error temp;
	c.json = json;
	assert ( v != NULL );
	v->type = JSON_NULL;
	/*读空格*/
	read_blank_first ( &c );
	/*读值*/
	if ( ( temp = read_value ( &c , v ) ) == JSON_OK )
	{
		read_blank_first ( &c );
		if ( *c.json != '\0' )
		{
			return JSON_INPUT_ERROR;
		}
	}

	return temp;
}
/*获取类型*/
json_type json_get_type ( const json_value *v )
{
	assert ( v != NULL );
	return v->type;
}