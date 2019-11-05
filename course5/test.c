#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myjson.h"
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
int static test_count = 0;
int static err_count = 0;
int static test_pass = 0;
static char *str_err[ ] = { "JSON_OK" , "JSON_ONLY_BLANK" , "JSON_INPUT_ERROR" , "JSON_VALUE_ERROR" , "JSON_INPUT_NUMBER_TOO_BIG" , "JSON_INVALID_UNICODE" ,"JSON_INVALID_UNICODE_SURROGATE"};
static char *str_type[]= {"JSON_NULL","JSON_TRUE","JSON_FALSE","JSON_NUMBER","JSON_STRING","JSON_ARRAY"};
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
#define EXPECT_EQ_ERROR(expect, actual) EXPECT_EQ((expect) == (actual), str_err[expect], str_err[actual], "%s")
#define EXPECT_EQ_TYPE(expect, actual) EXPECT_EQ((expect) == (actual), str_type[expect], str_type[actual], "%s")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ((expect) == (actual), expect, actual, "%.17g")
#define TEST_ERROR(error, json)\
    do {\
        json_value v;\
        v.type = JSON_NULL;\
        EXPECT_EQ_ERROR(error, json_parse(&v, json));\
        EXPECT_EQ_TYPE(JSON_NULL, json_get_type(&v));\
    } while(0)

#define TEST_NUMBER(expect, json)\
    do {\
        json_value v;\
        EXPECT_EQ_ERROR(JSON_OK, json_parse(&v, json));\
        EXPECT_EQ_TYPE(JSON_NUMBER, json_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, json_get_number(&v));\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}	
static void test_parse_expect_value() {
    TEST_ERROR(JSON_ONLY_BLANK, "");
    TEST_ERROR(JSON_ONLY_BLANK, " ");

	TEST_ERROR(JSON_VALUE_ERROR, "?");
	TEST_ERROR(JSON_INPUT_ERROR, " true?");

#if 1
	TEST_ERROR ( JSON_VALUE_ERROR , "+0" );
    TEST_ERROR(JSON_VALUE_ERROR, "+1");
    TEST_ERROR(JSON_VALUE_ERROR, ".123"); /* at least one digit before '.' */
    TEST_ERROR(JSON_INPUT_ERROR, "1.");   /* at least one digit after '.' */
    TEST_ERROR(JSON_VALUE_ERROR, "INF");
    TEST_ERROR(JSON_VALUE_ERROR, "inf");
    TEST_ERROR(JSON_VALUE_ERROR, "NAN");
    TEST_ERROR(JSON_INPUT_ERROR, "nan");
#endif
}
static void null_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_ERROR(JSON_OK,json_parse(&v,"null"));
	/*解析类型是否相等*/
	EXPECT_EQ_TYPE(JSON_NULL,json_get_type(&v));
}

static void true_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_ERROR(JSON_OK,json_parse(&v,"true"));
	/*解析类型是否相等*/
	EXPECT_EQ_TYPE(JSON_TRUE,json_get_type(&v));

	/*是否解析成功*/
	EXPECT_EQ_ERROR(JSON_OK,json_parse(&v," true"));
	/*解析类型是否相等*/
	EXPECT_EQ_TYPE(JSON_TRUE,json_get_type(&v));
}

static void false_test()
{
	json_value v;
	v.type = JSON_FALSE;
	/*是否解析成功*/
	EXPECT_EQ_ERROR(JSON_OK,json_parse(&v,"false"));
	/*解析类型是否相等*/
	EXPECT_EQ_TYPE(JSON_FALSE,json_get_type(&v));

	/*是否解析成功*/
	EXPECT_EQ_ERROR(JSON_OK,json_parse(&v," false"));
	/*解析类型是否相等*/
	EXPECT_EQ_TYPE(JSON_FALSE,json_get_type(&v));
}
#define IS_STRING(expect,actual,len)	EXPECT_EQ((memcmp(expect,actual,len)==0&&sizeof(expect)-1==len), expect, actual, "%s")

#define STRING_TEST(expect, json)\
    do {\
        json_value v;\
        v.type = JSON_NULL;\
		EXPECT_EQ_ERROR(JSON_OK,json_parse(&v,json));\
		EXPECT_EQ_TYPE(JSON_STRING,json_get_type(&v));\
        IS_STRING(expect, json_get_string(&v), json_get_string_length(&v));\
        json_free(&v);\
	    } while(0)

static void test_parse_string ()
{
	STRING_TEST ( "" , "\"\"" );
	STRING_TEST ( "Hello" , "\"Hello\"" );
	STRING_TEST ( "Hello\nWorld" , "\"Hello\\nWorld\"" );
	STRING_TEST ( "\" \\ / \b \f \n \r \t" , "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"" );
	STRING_TEST ( "Hello\0World" , "\"Hello\\u0000World\"" );
	STRING_TEST ( "\x24" , "\"\\u0024\"" );         /* Dollar sign U+0024 */
	STRING_TEST ( "\xC2\xA2" , "\"\\u00A2\"" );     /* Cents sign U+00A2 */
	STRING_TEST ( "\xE2\x82\xAC" , "\"\\u20AC\"" ); /* Euro sign U+20AC */
	STRING_TEST ( "\xF0\x9D\x84\x9E" , "\"\\uD834\\uDD1E\"" );  /* G clef sign U+1D11E */
	STRING_TEST ( "\xF0\x9D\x84\x9E" , "\"\\ud834\\udd1e\"" );  /* G clef sign U+1D11E */
}
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect,actual) EXPECT_EQ((expect==actual),(size_t)expect,(size_t)actual,"%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect,actual) EXPECT_EQ((expect==actual),(size_t)expect,(size_t)actual,"%zu")
#endif

static void test_parse_array ()
{
    json_value v;
    v.type = JSON_NULL;
    EXPECT_EQ_ERROR(JSON_OK,json_parse(&v,"[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_TYPE(JSON_ARRAY,json_get_type(&v));
    EXPECT_EQ_SIZE_T(5, json_get_array_size(&v));
    EXPECT_EQ_TYPE(JSON_NULL,json_get_type(json_get_array_element(&v,0)));
    EXPECT_EQ_TYPE(JSON_FALSE,  json_get_type(json_get_array_element(&v, 1)));
    EXPECT_EQ_TYPE(JSON_TRUE,   json_get_type(json_get_array_element(&v, 2)));
    EXPECT_EQ_TYPE(JSON_NUMBER, json_get_type(json_get_array_element(&v, 3)));
    EXPECT_EQ_TYPE(JSON_STRING, json_get_type(json_get_array_element(&v, 4)));
    EXPECT_EQ_DOUBLE(123.0, json_get_number(json_get_array_element(&v, 3)));
    IS_STRING("abc",json_get_string(json_get_array_element(&v, 4)),json_get_string_length(json_get_array_element(&v, 4)));
    json_free(&v);
}
#define IS_TRUE(actual)		EXPECT_EQ((actual!= 0), "true", "false", "%s")
#define IS_FALSE(actual)	EXPECT_EQ(actual==0, "false", "true", "%s")
#define IS_NUMBER(expect,actual)	EXPECT_EQ((expect) == (actual), expect, actual, "%.17g")

static void test_access_boolean ( )
{
	json_value v;
	v.type = JSON_NULL;
	json_set_boolean (&v ,1);
	IS_TRUE ( json_get_boolean ( &v ) );
	json_set_boolean ( &v , 0 );
	IS_FALSE ( json_get_boolean ( &v ) );
}

static void test_access_number ()
{
	json_value v;
	v.type = JSON_NULL;
	json_set_number ( &v , 1 );
	IS_NUMBER ( 1.0,json_get_number ( &v ) );
}
static void test_json_invalid_string_escape ()
{
	TEST_ERROR ( JSON_INPUT_ERROR , "\"\\v\"" );
	TEST_ERROR ( JSON_INPUT_ERROR , "\"\\'\"" );
	TEST_ERROR ( JSON_INPUT_ERROR , "\"\\0\"" );
	TEST_ERROR ( JSON_INPUT_ERROR , "\"\\x12\"" );
}

static void test_parse_invalid_string_char ()
{
	TEST_ERROR ( JSON_INPUT_ERROR , "\"\x01\"" );
	TEST_ERROR ( JSON_INPUT_ERROR , "\"\x1F\"" );
}


static void parse_test()
{
	null_test();
	true_test();
	false_test();
	test_parse_number();
	test_parse_expect_value();
	test_parse_string ();

	test_access_boolean ();
	test_access_number ();
	test_json_invalid_string_escape ();
    test_parse_invalid_string_char();
    test_parse_array ();
}
int main() 
{
	parse_test();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return 0;
}
