// Saturday, January 04, 2019

#ifndef KUSH_COMPILER_AST_EXPRESSION_ANNOTATION_H
#define KUSH_COMPILER_AST_EXPRESSION_ANNOTATION_H

#include <kush/Configuration.h>

/*******************************************************************************
 * ExpressionAnnotation                                                        *
 *******************************************************************************/

/**
 * @ingroup 
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
enum k_ExpressionAnnotation_t {
    KUSH_EXPRESSION_ANNOTATION_UNKNOWN,
    KUSH_EXPRESSION_ANNOTATION_PLACEHOLDER,
    KUSH_EXPRESSION_ANNOTATION_VALUE
};

typedef enum k_ExpressionAnnotation_t k_ExpressionAnnotation_t;

#endif /* KUSH_COMPILER_AST_EXPRESSION_ANNOTATION_H */
