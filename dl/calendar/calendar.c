/* interface functions.

 * code by Shane Caraveo shane@caraveo.com
 * copy freely!
 *
 */

#include "sdncal.h"

/* 
 * this array should be set up as:
 * {"PHPScriptFunctionName",dllFunctionName,1} 
 */

function_entry calendar_functions[] = {
	PHP_FE(jdtogregorian, NULL)
	PHP_FE(gregoriantojd, NULL)
	PHP_FE(jdtojulian, NULL)
	PHP_FE(juliantojd, NULL)
	PHP_FE(jdtojewish, NULL)
	PHP_FE(jewishtojd, NULL)
	PHP_FE(jdtofrench, NULL)
	PHP_FE(frenchtojd, NULL)
	PHP_FE(jddayofweek, NULL)
	PHP_FE(jdmonthname, NULL)
	{NULL, NULL, NULL}
};


zend_module_entry calendar_module_entry = {
	"Calendar", 
	calendar_functions, 
	NULL, NULL, NULL, NULL, NULL,
	STANDARD_MODULE_PROPERTIES,
};


#if COMPILE_DL
DLEXPORT zend_module_entry *get_module(void) { return &calendar_module_entry; }
#endif

DLEXPORT PHP_FUNCTION(jdtogregorian)
{
	pval **julday;
	int year, month, day;
	char date[10];

	if (zend_get_parameters_ex(1, &julday) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(julday);
	SdnToGregorian((*julday)->value.lval, &year, &month, &day);
	sprintf(date, "%i/%i/%i", month, day, year);

	RETURN_STRING(date,1);
}


DLEXPORT PHP_FUNCTION(gregoriantojd)
{
	pval **year, **month, **day;
	int jdate;

	if (zend_get_parameters_ex(3, &month, &day, &year) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long_ex(month);
	convert_to_long_ex(day);
	convert_to_long_ex(year);

	jdate = GregorianToSdn((*year)->value.lval, (*month)->value.lval,(*day)->value.lval);

	RETURN_LONG(jdate);
}


DLEXPORT PHP_FUNCTION(jdtojulian) 
{
	pval **julday;
	int year, month, day;
	char date[10];

	if (zend_get_parameters_ex(1, &julday) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(julday);
	SdnToJulian((*julday)->value.lval, &year, &month, &day);
	sprintf(date, "%i/%i/%i", month, day, year);

	RETURN_STRING(date,1);
}


DLEXPORT PHP_FUNCTION(juliantojd)
{
	pval **year, **month, **day;
	int jdate;

	if (zend_get_parameters_ex(3, &month, &day, &year) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long_ex(month);
	convert_to_long_ex(day);
	convert_to_long_ex(year);

	jdate = JulianToSdn((*year)->value.lval,(*month)->value.lval, (*day)->value.lval);

	RETURN_LONG(jdate);
}


DLEXPORT PHP_FUNCTION(jdtojewish) 
{
	pval **julday;
	int year, month, day;
	char date[10];

	if (zend_get_parameters_ex(1, &julday) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long_ex(julday);
	
	SdnToJewish((*julday)->value.lval, &year, &month, &day);
	sprintf(date, "%i/%i/%i", month, day, year);

	RETURN_STRING(date,1);
}


DLEXPORT PHP_FUNCTION(jewishtojd)
{
	pval **year, **month, **day;
	int jdate;

	if (zend_get_parameters_ex(3, &month, &day, &year) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(month);
	convert_to_long_ex(day);
	convert_to_long_ex(year);

	jdate = JewishToSdn((*year)->value.lval,(*month)->value.lval, (*day)->value.lval);

	RETURN_LONG(jdate);
}


DLEXPORT PHP_FUNCTION(jdtofrench)
{
	pval **julday;
	int year, month, day;
	char date[10];

	if (zend_get_parameters_ex(1, &julday) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(julday);
	
	SdnToFrench((*julday)->value.lval, &year, &month, &day);
	sprintf(date, "%i/%i/%i", month, day, year);

	RETURN_STRING(date,1);
}


DLEXPORT PHP_FUNCTION(frenchtojd)
{
	pval **year, **month, **day;
	int jdate;

	if (zend_get_parameters_ex(3, &month, &day, &year) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long_ex(month);
	convert_to_long_ex(day);
	convert_to_long_ex(year);

	jdate = FrenchToSdn((*year)->value.lval,(*month)->value.lval,(*day)->value.lval);

	RETURN_LONG(jdate);
}


DLEXPORT PHP_FUNCTION(jddayofweek)
{
	pval **julday, **mode;
	int day;
	char *daynamel, *daynames;

	if (zend_get_parameters_ex(2, &julday, &mode) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long_ex(julday);
	convert_to_long_ex(mode);

	day = DayOfWeek((*julday)->value.lval);
	daynamel = DayNameLong[day];
	daynames = DayNameShort[day];

		switch ((*mode)->value.lval) {
			case 0L:
				RETURN_LONG(day);
				break;
			case 1L:
				RETURN_STRING(daynamel,1);
				break;
			case 2L:
				RETURN_STRING(daynames,1);
				break;
			default:
				RETURN_LONG(day);
				break;
		}
}


DLEXPORT PHP_FUNCTION(jdmonthname)
{
	pval **julday, **mode;
	char *monthname = NULL;
	int month, day, year;

	if (zend_get_parameters_ex(2, &julday, &mode) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long_ex(julday);
	convert_to_long_ex(mode);

		switch((*mode)->value.lval) {
			case 0L:			/* gregorian or julian month */
				SdnToGregorian((*julday)->value.lval,&year, &month, &day);
				monthname = MonthNameShort[month];
				break;
			case 1L:			/* gregorian or julian month */
				SdnToGregorian((*julday)->value.lval,&year, &month, &day);
				monthname = MonthNameLong[month];
				break;
			case 2L:			/* gregorian or julian month */
				SdnToJulian((*julday)->value.lval, &year,&month, &day);
				monthname = MonthNameShort[month];
				break;
			case 3L:			/* gregorian or julian month */
				SdnToJulian((*julday)->value.lval, &year,&month, &day);
				monthname = MonthNameLong[month];
				break;
			case 4L:			/* jewish month */
				SdnToJewish((*julday)->value.lval, &year,&month, &day);
				monthname = JewishMonthName[month];
				break;
			case 5L:			/* french month */
				SdnToFrench((*julday)->value.lval, &year,&month, &day);
				monthname = FrenchMonthName[month];
				break;
			default:			/* default gregorian */
				/* FIXME - need to set monthname to something here ?? */
				break;
		}

	RETURN_STRING(monthname,1);
}
