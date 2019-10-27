#ifndef MYJSON_H__
#define MYJSON_H__
/*json的几种数据类型*/
typedef enum{
	JSON_NULL,JSON_TRUE,JSON_FALSE,JSON_NUMBER,JSON_STRING
} json_type;

/* json结构体 */
typedef struct {
	union{
		double number;
		struct s
		{
			char *str;
			size_t length;
		};
	}u;
    json_type type;

} json_value;

/*现有一个char的json字符串需要解析，返回结果，先写结果类型*/
typedef enum {
	JSON_OK,
	JSON_ONLY_BLANK,/*""或者" "*/
	JSON_INPUT_ERROR,/*不和json规矩，空白之后还有值"ture a"语法错误*/
	JSON_VALUE_ERROR/*输入值的类型不在范围内*/
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

void json_free (json_value *v); 
#endif 
