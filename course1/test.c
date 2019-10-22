#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myjson.h"
int static test_count = 0;
int static err_count = 0;
int static test_pass = 0;
static char *str_err[]= {"JSON_OK","JSON_ONLY_BLANK","JSON_INPUT_ERROR","JSON_VALUE_ERROR"};
static char *str_type[]= {"JSON_NULL","JSON_TRUE","JSON_FALSE"};
#define EXPECT_EQ(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: \n expect: " format " \n actual: " format "\n", __FILE__, __LINE__, expect,actual );\
            err_count = 1;\
        }\
    } while(0)
#define EXPECT_EQ_PARSE(expect, actual) EXPECT_EQ((expect) == (actual), str_err[expect], str_err[actual], "%s")
#define EXPECT_EQ_EQUAL(expect, actual) EXPECT_EQ((expect) == (actual), str_type[expect], str_type[actual], "%s")

static void null_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_OK,json_parse(&v,"null"));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_NULL,json_get_type(&v));
}

static void true_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_OK,json_parse(&v,"true"));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_TRUE,json_get_type(&v));

	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_OK,json_parse(&v," true"));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_TRUE,json_get_type(&v));
}

static void false_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_OK,json_parse(&v,"false"));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_FALSE,json_get_type(&v));

	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_OK,json_parse(&v," false"));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_FALSE,json_get_type(&v));
}
static void empty_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_ONLY_BLANK,json_parse(&v,""));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_NULL,json_get_type(&v));

	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_ONLY_BLANK,json_parse(&v," "));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_NULL,json_get_type(&v));
}

static void invalid_input_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_INPUT_ERROR,json_parse(&v,"?"));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_NULL,json_get_type(&v));

	/*是否解析成功*/
	EXPECT_EQ_PARSE(JSON_INPUT_ERROR,json_parse(&v," true?"));
	/*解析类型是否相等*/
	EXPECT_EQ_EQUAL(JSON_NULL,json_get_type(&v));
}
int main() 
{
/*
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
	fprintf ( stderr , "%s:%s\n", "true",str[json_parse ( &v , "true" )]); 
	fprintf ( stderr , "%s:%s\n", "false",str[json_parse ( &v , "false" )]); 
*/
	null_test();
	true_test();
	false_test();
	empty_test();
	invalid_input_test();

	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return 0;
}
