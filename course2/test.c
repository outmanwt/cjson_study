#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myjson.h"
int static test_count = 0;
int static err_count = 0;
int static test_pass = 0;
static char *str_err[]= {"JSON_OK","JSON_ONLY_BLANK","JSON_INPUT_ERROR","JSON_INPUT_NUMBER_TOO_BIG"};
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
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ((expect) == (actual), expect, actual, "%.17g")
#define TEST_ERROR(error, json)\
    do {\
        json_value v;\
        v.type = JSON_NULL;\
        EXPECT_EQ_PARSE(error, json_parse(&v, json));\
        EXPECT_EQ_EQUAL(JSON_NULL, json_get_type(&v));\
    } while(0)

#define TEST_NUMBER(expect, json)\
    do {\
        json_value v;\
        EXPECT_EQ_PARSE(JSON_OK, json_parse(&v, json));\
        EXPECT_EQ_EQUAL(JSON_NUMBER, json_get_type(&v));\
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
	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}	
static void test_parse_expect_value() {
    TEST_ERROR(JSON_ONLY_BLANK, "");
    TEST_ERROR(JSON_ONLY_BLANK, " ");

	TEST_ERROR(JSON_INPUT_ERROR, "?");
	TEST_ERROR(JSON_INPUT_ERROR, " true?");
    TEST_ERROR(JSON_INPUT_ERROR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(JSON_INPUT_ERROR, "0x0");
    TEST_ERROR(JSON_INPUT_ERROR, "0x123");
#if 1
	TEST_ERROR ( JSON_INPUT_ERROR , "+0" );
    TEST_ERROR(JSON_INPUT_ERROR, "+1");
    TEST_ERROR(JSON_INPUT_ERROR, ".123"); /* at least one digit before '.' */
    TEST_ERROR(JSON_INPUT_ERROR, "1.");   /* at least one digit after '.' */
    TEST_ERROR(JSON_INPUT_ERROR, "INF");
    TEST_ERROR(JSON_INPUT_ERROR, "inf");
    TEST_ERROR(JSON_INPUT_ERROR, "NAN");
    TEST_ERROR(JSON_INPUT_ERROR, "nan");
	    /* invalid number */
	TEST_ERROR ( JSON_INPUT_ERROR , "0123" ); /* after zero should be '.' , 'E' , 'e' or nothing */
	TEST_ERROR ( JSON_INPUT_ERROR , "0x0" );
	TEST_ERROR ( JSON_INPUT_ERROR , "0x123" );
#endif
}
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

static void parse_test()
{
	null_test();
	true_test();
	false_test();
	test_parse_number();
	test_parse_expect_value();
}
int main() 
{
	parse_test();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return 0;
}
