/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: krakjoe                                                      |
  +----------------------------------------------------------------------+
*/
#ifndef HAVE_PHP_UI_CBUTTON
#define HAVE_PHP_UI_CBUTTON
#include <ui.h>

#include "php.h"

#include <classes/control.h>
#include <classes/cbutton.h>
#include <classes/color.h>

zend_object_handlers php_ui_cbutton_handlers;

zend_object* php_ui_cbutton_create(zend_class_entry *ce) {
	php_ui_cbutton_t *cbutton = 
		(php_ui_cbutton_t*) ecalloc(1, sizeof(php_ui_cbutton_t) + zend_object_properties_size(ce));

	zend_object_std_init(&cbutton->std, ce);

	object_properties_init(&cbutton->std, ce);

	cbutton->std.handlers = &php_ui_cbutton_handlers;

	return &cbutton->std;
}

void php_ui_cbutton_free(zend_object *o) {
	php_ui_cbutton_t *cbutton = php_ui_cbutton_from(o);

	if (cbutton->change.fci.size) {
		if (Z_TYPE(cbutton->change.fci.function_name)) {
			zval_ptr_dtor(&cbutton->change.fci.function_name);
		}
	}

	zend_object_std_dtor(o);
}

void php_ui_cbutton_change_handler(uiColorButton *u, void *_button) {
	php_ui_cbutton_t *button = (php_ui_cbutton_t*) _button;

	if (button->change.fci.size) {
		zval rv;
		zval ctrl;

		ZVAL_UNDEF(&rv);
		ZVAL_OBJ(&ctrl, &button->std);

		button->change.fci.retval = &rv;

		zend_fcall_info_argn(&button->change.fci, 1, &ctrl);

		if (zend_call_function(&button->change.fci, &button->change.fcc) != SUCCESS) {
			return;
		}

		zend_fcall_info_args_clear(&button->change.fci, 1);

		if (Z_TYPE(rv) != IS_UNDEF) {
			zval_ptr_dtor(&rv);
		}
	}
}

ZEND_BEGIN_ARG_INFO_EX(php_ui_cbutton_construct_info, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* {{{ proto ColorButton ColorButton::__construct() */
PHP_METHOD(ColorButton, __construct) 
{
	php_ui_cbutton_t *cbutton = php_ui_cbutton_fetch(getThis());

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	cbutton->b = uiNewColorButton();

	uiColorButtonOnChanged(cbutton->b, php_ui_cbutton_change_handler, cbutton);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(php_ui_cbutton_set_color_info, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, color, UI\\Draw\\Color, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void ColorButton::setColor(UI\Draw\Color color) */
PHP_METHOD(ColorButton, setColor)
{
	php_ui_cbutton_t *cbutton = php_ui_cbutton_fetch(getThis());
	zval *color = NULL;
	php_ui_color_t *c;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "O", &color, uiDrawColor_ce) != SUCCESS) {
		return;
	}
	
	c = php_ui_color_fetch(color);

	uiColorButtonSetColor(cbutton->b, c->r, c->g, c->b, c->a);
} /* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(php_ui_cbutton_get_color_info, 0, 0, IS_OBJECT, "UI\\Draw\\Color", 0)
ZEND_END_ARG_INFO()

/* {{{ proto UI\Draw\Color ColorButton::getColor(void) */
PHP_METHOD(ColorButton, getColor)
{
	php_ui_cbutton_t *cbutton = php_ui_cbutton_fetch(getThis());
	php_ui_color_t *color;
	double r = 0, g = 0, b = 0, a = 0;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	uiColorButtonColor(cbutton->b, &r, &g, &b, &a);

	object_init_ex(return_value, uiDrawColor_ce);
	
	color = php_ui_color_fetch(return_value);
	color->r = r;
	color->g = g;
	color->b = b;
	color->a = a;
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(php_ui_cbutton_on_change_info, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void ColorButton::onChange(callable handler) */
PHP_METHOD(ColorButton, onChange)
{
	php_ui_cbutton_t *button = php_ui_cbutton_fetch(getThis());
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "f", &fci, &fcc) != SUCCESS) {
		return;
	}

	if (button->change.fci.size) {
		if (Z_TYPE(button->change.fci.function_name)) {
			zval_ptr_dtor(&button->change.fci.function_name);
		}
	}

	memcpy(&button->change.fci, &fci, sizeof(zend_fcall_info));
	memcpy(&button->change.fcc, &fcc, sizeof(zend_fcall_info_cache));

	if (Z_TYPE(button->change.fci.function_name)) {
		Z_ADDREF(button->change.fci.function_name);
	}
} /* }}} */

/* {{{ */
const zend_function_entry php_ui_cbutton_methods[] = {
	PHP_ME(ColorButton, __construct,          php_ui_cbutton_construct_info,              ZEND_ACC_PUBLIC)
	PHP_ME(ColorButton, setColor,	          php_ui_cbutton_set_color_info,              ZEND_ACC_PUBLIC)
	PHP_ME(ColorButton, getColor,	          php_ui_cbutton_get_color_info,              ZEND_ACC_PUBLIC)
	PHP_ME(ColorButton, onChange,             php_ui_cbutton_on_change_info,              ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

/* {{{ */
PHP_MINIT_FUNCTION(UI_ColorButton) 
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "UI\\Control", "ColorButton", php_ui_cbutton_methods);

	uiColorButton_ce = zend_register_internal_class_ex(&ce, uiControl_ce);
	uiColorButton_ce->create_object = php_ui_cbutton_create;
	uiColorButton_ce->ce_flags |= ZEND_ACC_FINAL;

	memcpy(&php_ui_cbutton_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	php_ui_cbutton_handlers.free_obj = php_ui_cbutton_free;
	php_ui_cbutton_handlers.offset = XtOffsetOf(php_ui_cbutton_t, std);

	return SUCCESS;
} /* }}} */
#endif
