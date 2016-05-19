# Component makefile for extras/http

# EXTRA_CFLAGS = -DNDEBUG
INC_DIRS += $(http_ROOT)

# args for passing into compile rule generation
http_INC_DIR =  $(http_ROOT)http_parser
http_SRC_DIR =  $(http_ROOT) $(http_ROOT)http_parser

$(eval $(call component_compile_rules,http))