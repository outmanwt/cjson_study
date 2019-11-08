#ifndef MYJSON_H__
#define MYJSON_H__
/*json的几种数据类型*/
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>  /* memcpy() */
#include <stddef.h>
typedef enum{
	JSON_NULL,JSON_TRUE,JSON_FALSE,JSON_NUMBER,JSON_STRING,JSON_ARRAY
} json_type;
typedef struct json_member json_member
typedef struct json_value json_value;
/* json结构体 */
struct json_value{
	union{
		double number;
		struct
		{
			char *str;
			size_t length;
		}s;
		struct
		{
			json_value *e;
			size_t size;
		}a;
		struct
		{
			json_member *m;
			size_t size;
		}o;
	}u;
    json_type type;
} ;
struct json_member
{
    char * key_name;
    json_value v;
    size_t key_length;
};
/*现有一个char的json字符串需要解析，返回结果，先写结果类型*/
typedef enum {
	JSON_OK,
	JSON_ONLY_BLANK,/*""或者" "*/
	JSON_INPUT_ERROR,/*不和json规矩，空白之后还有值"ture a"语法错误*/
	JSON_VALUE_ERROR,/*输入值的类型不在范围内*/
	JSON_INPUT_NUMBER_TOO_BIG,
	JSON_INVALID_UNICODE,
	JSON_INVALID_UNICODE_SURROGATE
} json_error;

/*解析!int可以改个名字*/
json_error json_parse(json_value *v,const char *json);

/*获取类型用来检验*/
json_type json_get_type(const json_value *v);

/*获取数字用来检验*/
double json_get_number(const json_value *v);
void json_set_number ( json_value *v ,double n);

int json_get_boolean ( const json_value * v);
void json_set_boolean ( json_value *v , int b );

void json_set_string ( json_value *v , const char * c, size_t len );
char * json_get_string ( json_value *v );
size_t json_get_string_length ( json_value *v );

size_t json_get_array_size ( json_value *v );
json_value * json_get_array_element (json_value *v , size_t index);

void json_free (json_value *v);
#endif
