#include "myjson.h"

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
	char* stack;
	size_t size , top;
}json_struct;

static json_error json_parse_null ( json_struct *c , json_value *v )
{
	/*再检查一遍c是否为n?有必要？*/
	assert ( *c->json == 'n' );
	if ( c->json[ 1 ] != 'u' || c->json[ 2 ] != 'l' || c->json[ 3 ] != 'l' )
		return JSON_INPUT_ERROR;
	c->json += 4;
	v->type = JSON_NULL;
	return JSON_OK;
}

static json_error json_parse_true ( json_struct *c , json_value *v )
{
	/*再检查一遍c是否为n?有必要？*/
	assert ( *c->json == 't' );
	if ( c->json[ 1 ] != 'r' || c->json[ 2 ] != 'u' || c->json[ 3 ] != 'e' )
		return JSON_INPUT_ERROR;
	c->json += 4;
	v->type = JSON_TRUE;
	return JSON_OK;
}

static json_error json_parse_false ( json_struct *c , json_value *v )
{
	/*再检查一遍c是否为n?有必要？*/
	assert ( *c->json == 'f' );
	if ( c->json[ 1 ] != 'a' || c->json[ 2 ] != 'l' || c->json[ 3 ] != 's' || c->json[ 4 ] != 'e' )
		return JSON_INPUT_ERROR;
	c->json += 5;
	v->type = JSON_FALSE;
	return JSON_OK;
}
static json_error json_parse_literal ( json_struct *c , json_value *v , const char *literal , json_type type )
{
	size_t i;
	/*再检查一遍c是否为n?有必要？*/
	assert ( c->json[ 0 ] == literal[ 0 ] );
	for ( i = 0; literal[ i + 1 ] != '\0'; i++ )
	{
		if ( c->json[ i ] != literal[ i ] )
			return JSON_INPUT_ERROR;
	}

	c->json += i + 1;
	v->type = type;
	return JSON_OK;
}
#if 0
	/*字符串格式规则*/
	string = quotation - mark *char quotation - mark
	char = unescaped /
	escape (
	%x22 / ; "    quotation mark  U+0022
	%x5C / ; \    reverse solidus U + 005C
	% x2F / ; / solidus         U + 002F
	% x62 / ; b    backspace       U + 0008
	% x66 / ; f    form feed       U + 000C
	% x6E / ; n    line feed       U + 000A
	% x72 / ; r    carriage return U + 000D
	% x74 / ; t    tab             U + 0009
	% x75 4HEXDIG ); uXXXX                U + XXXX
	escape = %x5C; \
	quotation - mark = %x22; "
	unescaped = %x20 - 21 / %x23 - 5B / %x5D - 10FFFF
#endif
void json_set_string ( json_value *v , const char * c, size_t len )
{
	assert ( v != NULL &&(c!=NULL||len==0));
	json_free ( v );
	v->u.s.str = ( char* ) malloc ( len + 1 );
	memcpy ( v->u.s.str , c , len );
	v->u.s.str[ len ] = '\0';
	v->u.s.length = len;
	v->type = JSON_STRING;
}

static void* josn_strack_pop ( json_struct *c , size_t	len )
{
	assert ( c->top >= len );
	return c->stack + ( c->top -= len );
}
#define INIT_SIZE 256
static void json_strack_push ( json_struct *c ,  char s )
{
	size_t size = sizeof ( char );
	if ( c->size <= c->top + size )
	{
		if (( c->size )==0)
			c->size = 256;
		while ( c->top + size >= c->size )
			c->size += c->size>>1;
		c->stack = ( char * ) realloc ( c->stack , c->size );
	}
	*((char * ) (c->stack + c->top)) = s;
	c->top += size;
}
static json_error json_parse_string ( json_struct *c , json_value *v )
{
	size_t head = c->top , len;
	const char * p;
	assert ( *c->json == '\"' ); c->json++;/*跳过开头"*/
	p = c->json;
	while ( 1 )
	{
		char ch = *p++;
		switch ( ch )
		{
			case'\"': /*结束"*/
				/*记录长度(top-head)，将值拷贝至v,移动c*/
				len = c->top - head;
				json_set_string ( v , ( const char * ) josn_strack_pop ( c , len ) , len );
				c->json = p;
				return JSON_OK;
			case'\\':/*转义符*/
				/*跳过转义符，根据后面的值压入栈*/
				switch ( *p++ )
				{
					case '\"':json_strack_push ( c , '\"' ); break;
					case '\\':json_strack_push ( c , '\\' ); break;
					case '/':  json_strack_push ( c , '/' ); break;
					case 'b':  json_strack_push ( c , '\b' ); break;
					case 'f':  json_strack_push ( c , '\f' ); break;
					case 'n':  json_strack_push ( c , '\n' ); break;
					case 'r':  json_strack_push ( c , '\r' ); break;
					case 't':  json_strack_push ( c , '\t' ); break;
					default:
						c->top = head;/*转义符后面错误保证回到开头top*/
						return JSON_INPUT_ERROR;
				}                
				break;
			case'\0':
				c->top = head;/*转义符后面错误保证回到开头top*/
				return JSON_INPUT_ERROR;
			default:
				if ( ( unsigned char ) ch < 0x20 ) 
				{
					c->top = head;/*转义符后面错误保证回到开头top*/
					return JSON_INPUT_ERROR;
				}
				json_strack_push ( c , ch );
		}		
	}
}
#define	NUM09(ch) ((ch) >= '0' && (ch) <= '9')
#define	NUM19(ch) ((ch) >= '1' && (ch) <= '9')

static json_error read_number ( json_struct *c , json_value *v )
{
	char* other;
	/* \TODO validate number */
	v->u.number = strtod ( c->json , &other );
	if ( c->json == other )
		return JSON_VALUE_ERROR;
	c->json = other;
	v->type = JSON_NUMBER;
	return JSON_OK;
}

/*读空格*/
static json_error read_value ( json_struct *c , json_value *v )
{
	switch ( *c->json )
	{
		case 'n':	return json_parse_literal ( c , v , "null" , JSON_NULL );
		case 't':   return json_parse_literal ( c , v , "true" , JSON_TRUE );
		case 'f':   return json_parse_literal ( c , v , "false" , JSON_FALSE );
		case '\0': 	return JSON_ONLY_BLANK;
		case '"':	return json_parse_string ( c , v );
		default: 	return read_number ( c , v );
	}
}
static void read_blank_first ( json_struct *c )
{
	const char* temp = c->json;
	while ( *temp == ' ' || *temp == '\t' || *temp == '\n' || *temp == '\r' )
		temp++;
	c->json = temp;
}


/*解析*/
json_error json_parse ( json_value *v , const char *json )
{
	json_struct c;
	json_error temp;
	assert ( v != NULL );
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	v->type = JSON_NULL;
	/*读空格*/
	read_blank_first ( &c );
	/*读值*/
	if ( ( temp = read_value ( &c , v ) ) == JSON_OK )
	{
		read_blank_first ( &c );
		if ( *c.json != '\0' )
		{
			v->type = JSON_NULL;
			return JSON_INPUT_ERROR;
		}
	}
	assert ( c.top == 0 );
	free ( c.stack );
	return temp;
}
void json_free ( json_value *v )
{
	assert ( v != NULL );
	if ( v->type == JSON_STRING )
	{
		free ( v->u.s.str );
	}
	v->type = JSON_NULL;
}
/*获取类型*/
json_type json_get_type ( const json_value *v )
{
	assert ( v != NULL );
	return v->type;
}
/*获取数字*/
double json_get_number ( const json_value *v )
{
	assert ( v != NULL && ( v->type == JSON_NUMBER ) );
	return v->u.number;
}

void json_set_number ( json_value *v , double n )
{
	json_free ( v );
	v->u.number = n;
	v->type = JSON_NUMBER;
}

int json_get_boolean ( const json_value * v )
{
	assert ( v != NULL && ( v->type == JSON_FALSE || v->type == JSON_TRUE ) );
	return v->type == JSON_TRUE;
}

void json_set_boolean ( json_value *v , int b )
{
	json_free ( v );
	v->type = b ? JSON_TRUE : JSON_FALSE;
}
const char * json_get_string ( json_value *v )
{
	assert ( v != NULL&&v->type == JSON_STRING );
	return v->u.s.str;
}
const size_t json_get_string_length ( json_value *v )
{
	assert ( v != NULL&&v->type == JSON_STRING );
	return v->u.s.length;
}
