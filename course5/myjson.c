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

    number = [ "-" ] int[ frac ][ exp ]
            int = "0" / digit1 - 9 * digit
            frac = "." 1 * digit
            exp = ( "e" / "E" )[ "-" / "+" ] 1 * digit

    字符串格式规则
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
 */
/*json结构体*/
typedef struct {
    const char* json;
    char* stack;
    size_t size , top;
}json_struct;

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
#define PUTC(c, ch)         do { *(char*)json_strack_push(c, sizeof(char)) = (ch); } while(0)
static void* json_strack_push ( json_struct *c ,  size_t size )
{
    void* ret;
    if ( c->size <= c->top + size )
    {
        if (( c->size )==0)
            c->size = 256;
        while ( c->top + size >= c->size )
            c->size += c->size>>1;
        c->stack = ( char * ) realloc ( c->stack , c->size );
    }
    ret = (c->stack + c->top);
    c->top += size;
    return ret;
}
#define STRING_ERROR(str)	do{c->top = head; return str;}while(0)
const char* json_parse_hex4 ( const char* p , unsigned* u )
{
    int i;
    char ch;
    *u = 0;
    for (  i = 0; i < 4; i++ )
    {
        ch = *p++;
        *u <<= 4;
        if ( ch >= '0'&&ch <= '9' )	*u |= ch - '0';
        else if ( ch >= 'A'&&ch <= 'F' )	*u |= ch - ('A' - 10);
        else if ( ch >= 'a'&&ch <= 'f' )	*u |= ch - ( 'a' - 10 );
        else return NULL;
    }
    return p;
}
static void json_encode_utf8 ( json_struct *c , const unsigned u )
{
    if ( u <= 0x7F )
        PUTC ( c , (char)u );
    else if ( u <= 0x7FF )
    {/*这种情况下最多11位二进制，其中后6位二进制肯定要给第二个字节110xxxxx	10xxxxxx*/
        PUTC ( c , 0xC0 | ( ( u >> 6 ) & 0xFF ) );
        PUTC ( c , 0x80 | (    u        & 0x3F ) );
    }
    else if ( u <= 0xFFFF )
    {/*这种情况下最多16位二进制，1110xxxx	10xxxxxx	10xxxxxx*/
        PUTC ( c , 0xE0 | ( ( u >> 12 ) & 0xFF ) );
        PUTC ( c , 0x80 | ( ( u >> 6 )  & 0x3F ) );
        PUTC ( c , 0x80 | (   u         & 0x3F ) );
    }
    else
    {
        assert ( u <= 0x10FFFF );
        PUTC ( c , 0xF0 | ( ( u >> 18 ) & 0xFF ) );
        PUTC ( c , 0x80 | ( ( u >> 12 ) & 0x3F ) );
        PUTC ( c , 0x80 | ( ( u >> 6 )  & 0x3F ) );
        PUTC ( c , 0x80 | (   u         & 0x3F ) );
    }
}
static json_error json_parse_string( json_struct *c ,json_value *v)
{
        int ret;
        char *s;
        size_t len;
        if(ret=(json_parse_string_raw(c,&s,&len)==JSON_OK))
        {
             json_set_string ( v , ( const char * ) s,len);
        }
        return ret;
}
static json_error json_parse_string_raw ( json_struct *c , char** str , size_t * len)
{
    size_t head = c->top ;
    const char * p;
    assert ( *c->json == '\"' ); c->json++;/*跳过开头"*/
    p = c->json;
    for ( ;; )
    {
        unsigned u,u2;/*存储为码点 u*/
        char ch = *p++;
        switch ( ch )
        {
        case'\"': /*结束"*/
            /*记录长度(top-head)，将值拷贝至v,移动c */
            len = c->top - head;
            *str = josn_strack_pop ( c , *len );
            c->json = p;e
            return JSON_OK;
        case'\\':/*转义符*/
            /*跳过转义符，根据后面的值压入栈*/
            switch ( *p++ )
            {
            case 'u':
                if ( !( p = json_parse_hex4 ( p , &u ) ) )
                    STRING_ERROR ( JSON_INVALID_UNICODE );
                if ( u >= 0xD800 && u <= 0xDBFF )  /*有多个代理对*/
                {
                    if ( ( *p++ != '\\' ) || ( *p++ != 'u' )  )
                        STRING_ERROR ( JSON_INVALID_UNICODE_SURROGATE );
                    if  ( !( p = json_parse_hex4 ( p , &u2 ) ) )
                        STRING_ERROR ( JSON_INVALID_UNICODE );
                    if ( u2 < 0xDC00 || u2 > 0xDFFF )
                        STRING_ERROR ( JSON_INVALID_UNICODE_SURROGATE );
                    u = ( ( ( u - 0xD800 ) << 10 ) | ( u2 - 0xDC00 ) ) + 0x10000;
                }
                json_encode_utf8 ( c , u );
                break;
            case '\"':PUTC ( c , '\"' ); break;
            case '\\':PUTC ( c , '\\' ); break;
            case '/':  PUTC ( c , '/' ); break;
            case 'b':  PUTC ( c , '\b' ); break;
            case 'f':  PUTC ( c , '\f' ); break;
            case 'n':  PUTC ( c , '\n' ); break;
            case 'r':  PUTC ( c , '\r' ); break;
            case 't':  PUTC ( c , '\t' ); break;
            default:
                STRING_ERROR ( JSON_INPUT_ERROR );
            }
            break;
        case'\0':
            STRING_ERROR ( JSON_INPUT_ERROR );
        default:
            if ( ( unsigned char ) ch < 0x20 )
                STRING_ERROR ( JSON_INPUT_ERROR );
            PUTC ( c , ch );
        }
    }
}
static json_error json_parse_object ( json_struct *c , json_value *v)
{
    size_t len;
    json_member member;
    assert(c->json =='{');  c->json++;
    read_blank_first(c);
    if(c->json[]=='}')
    {
        c->json++;
        v->type = JSON_OBJECT;
        v->u.o.size=0;
        v->u.o = NULL;
		return JSON_OK;
    }
	for(;;){
		char *str;
		member.v = JSON_NULL;
		if(*c->json != '"'){
			ret = JSON_INPUT_ERROR;
			break;
		}
		if((ret == json_parse_string_raw(c ,&str ,&member.key_length)!= JSON_OK))
			break;
		memcpy(member.key = (char*)malloc(member.key_length+1),str,member.key_length);
		member.key[member.k_length] = '\0';
		read_blank_first(c);
		if(*c->json! = ':'){
			ret = JSON_INPUT_ERROR;
			break;
		}
		c->json++;
		read_blank_first(c);
		if((ret == json_prase(c,&member.v)!=JSON_OK))
			break;
		memcpy(josn_strack_pop(c,sizeof(json_member)),&m,sizeof(json_member));
		size++;
		member.key = NULL;
		read_blank_first(c);
		if(c->json[0]==',')
		{
			c->json++;
			read_blank_first(c);
		}
		else if(c->json[0]=='}')
		{
			size_t s = sizeof(json_member)*size;
			c->json++;
			v->type = JSON_OBJECT;
			v->u.o.size = size;
			memcpy(v->.u.o.member = (json_member*)malloc(s),json_stack_pop(c,s),s);
			return JSON_OK;
		}
		else{
			ret = JOSN_INPUT_ERROR;
			break;
		}
		free(.ember.key);
		for(i = 0;i<size;i++){
			json_member* m = (json_member*)json_stack_pop(c,sizeof(json_member));
			free(m.key);
			json_free(&m->v);
		}
		v->type = JONS_NULL;
		return ret;
	} 		
    return ret;
}

#define	NUM09(ch) ((ch) >= '0' && (ch) <= '9')
#define	NUM19(ch) ((ch) >= '1' && (ch) <= '9')

static json_error read_number ( json_struct *c , json_value *v )
{
    const char* p = c->json;
    if ( p[ 0 ] == '-' )	p++;
    if ( p[ 0 ] == '0' )	p++;
    else
    {
        if ( !NUM19 ( p[ 0 ] ) )
            return JSON_VALUE_ERROR;
        for ( p++; NUM09 ( p[ 0 ] ); p++ );
    }
    if ( p[ 0 ] == '.' )
    {
        p++;
        if ( !NUM09 ( p[ 0 ] ) )
            return JSON_INPUT_ERROR;
        for ( p++; NUM09 ( p[ 0 ] ); p++ );
    }
    if ( p[ 0 ] == 'e' || p[ 0 ] == 'E' )
    {
        p++;
        if ( p[ 0 ] == '+' || p[ 0 ] == '-' )
            p++;
        if ( !NUM09 ( p[ 0 ] ) )
            return JSON_INPUT_ERROR;
        for ( p++; NUM09 ( p[ 0 ] ); p++ );
    }
    errno = 0;
    v->u.number = strtod ( c->json , NULL );
    if ( errno == ERANGE && ( v->u.number == HUGE_VAL || v->u.number == -HUGE_VAL ) )
        return JSON_INPUT_NUMBER_TOO_BIG;
    v->type = JSON_NUMBER;
    c->json = p;
    return JSON_OK;
}
static json_error read_value ( json_struct *c , json_value *v );
static void read_blank_first ( json_struct *c );
static json_error json_parse_array(json_struct *c , json_value *v)
{
    int size=0,i=0;
    json_error ret;
    assert(c->json[0]=='[');
    c->json++;
    read_blank_first(c);
    if(c->json[0]==']')
    {
        c->json++;
        v->u.a.e = NULL;
        v->u.a.size =0;
        v->type = JSON_ARRAY;
        return JSON_OK;
    }
    for (;;) {
        json_value temp_value;
        temp_value.type = JSON_NULL;
        if((ret = read_value(c,&temp_value)!=JSON_OK))
            break;
        /*压栈*/
        memcpy(json_strack_push(c,sizeof (json_value)),&temp_value,sizeof (json_value));
        size++;
        read_blank_first(c);
        if(c->json[0]==',')
        {
            c->json++;
            read_blank_first(c);
        }
        else if(c->json[0]==']')
        {
            c->json++;
            v->u.a.size =size;
            v->type = JSON_ARRAY;
            size *= sizeof (json_value);
            memcpy(v->u.a.e = (json_value*)malloc(size),josn_strack_pop(c,size),size);
            return JSON_OK;
        }
        else
            break;
    }
    for (i=0;i<size;i++) {
        json_free((json_value*)josn_strack_pop(c,sizeof (json_value)));
    }
    return ret;
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
    case '[':   return json_parse_array( c , v );
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
    size_t i;
    assert ( v != NULL );
    switch (v->type) {
    case JSON_STRING:
        free ( v->u.s.str );
        break;
    case JSON_ARRAY:
        for (i=0;i<v->u.a.size;i++)
            json_free(&v->u.a.e[i]);
        free(v->u.a.e);
        break;
    default:  break;
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
char * json_get_string ( json_value *v )
{
    assert ( v != NULL&&v->type == JSON_STRING );
    return v->u.s.str;
}
size_t json_get_string_length ( json_value *v )
{
    assert ( v != NULL&&v->type == JSON_STRING );
    return v->u.s.length;
}
size_t json_get_array_size ( json_value *v )
{
    assert(v != NULL&&v->type == JSON_ARRAY);
    return  v->u.a.size;
}
json_value * json_get_array_element ( json_value *v,size_t index )
{
    assert(v != NULL&&v->type == JSON_ARRAY);
    assert(index<v->u.a.size);
    return &v->u.a.e[index];
}
