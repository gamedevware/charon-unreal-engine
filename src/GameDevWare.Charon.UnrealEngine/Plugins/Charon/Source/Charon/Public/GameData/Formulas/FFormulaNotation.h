// Copyright GameDevWare, Denis Zykov 2025

#pragma once

#include "CoreMinimal.h"

/**
 * Global notation constants for Formula expressions.
 */
namespace FFormulaNotation
{
    // Attributes
    static const FString EXPRESSION_TYPE_ATTRIBUTE = TEXT("expressionType");
    static const FString EXPRESSION_ATTRIBUTE = TEXT("expression");
    static const FString BINDINGS_ATTRIBUTE = TEXT("bindings");
    static const FString INITIALIZERS_ATTRIBUTE = TEXT("initializers");
    static const FString ARGUMENTS_ATTRIBUTE = TEXT("arguments");
    static const FString NEW_ATTRIBUTE = TEXT("new");
    static const FString LEFT_ATTRIBUTE = TEXT("left");
    static const FString RIGHT_ATTRIBUTE = TEXT("right");
    static const FString TEST_ATTRIBUTE = TEXT("test");
    static const FString IF_TRUE_ATTRIBUTE = TEXT("ifTrue");
    static const FString IF_FALSE_ATTRIBUTE = TEXT("ifFalse");
    static const FString TYPE_ATTRIBUTE = TEXT("type");
    static const FString VALUE_ATTRIBUTE = TEXT("value");
    static const FString PROPERTY_OR_FIELD_NAME_ATTRIBUTE = TEXT("propertyOrFieldName");
    static const FString NAME_ATTRIBUTE = TEXT("name");
    static const FString USE_NULL_PROPAGATION_ATTRIBUTE = TEXT("useNullPropagation");

    // Expression types
    static const FString EXPRESSION_TYPE_PROPERTY_OR_FIELD = TEXT("PropertyOrField");
    static const FString EXPRESSION_TYPE_MEMBER_RESOLVE = TEXT("MemberResolve");
    static const FString EXPRESSION_TYPE_CONSTANT = TEXT("Constant");
    static const FString EXPRESSION_TYPE_CONVERT = TEXT("Convert");
    static const FString EXPRESSION_TYPE_CONVERT_CHECKED = TEXT("ConvertChecked");
    static const FString EXPRESSION_TYPE_GROUP = TEXT("Group");
    static const FString EXPRESSION_TYPE_INVOKE = TEXT("Invoke");
    static const FString EXPRESSION_TYPE_LAMBDA = TEXT("Lambda");
    static const FString EXPRESSION_TYPE_INDEX = TEXT("Index");
    static const FString EXPRESSION_TYPE_UNCHECKED_SCOPE = TEXT("UncheckedScope");
    static const FString EXPRESSION_TYPE_CHECKED_SCOPE = TEXT("CheckedScope");
    static const FString EXPRESSION_TYPE_TYPE_OF = TEXT("TypeOf");
    static const FString EXPRESSION_TYPE_DEFAULT = TEXT("Default");
    static const FString EXPRESSION_TYPE_NEW = TEXT("New");
    static const FString EXPRESSION_TYPE_NEW_ARRAY_BOUNDS = TEXT("NewArrayBounds");
    static const FString EXPRESSION_TYPE_NEW_ARRAY_INIT = TEXT("NewArrayInit");
    static const FString EXPRESSION_TYPE_MEMBER_INIT = TEXT("MemberInit");
    static const FString EXPRESSION_TYPE_LIST_BINDING = TEXT("ListBinding");
    static const FString EXPRESSION_TYPE_MEMBER_BINDING = TEXT("MemberBinding");
    static const FString EXPRESSION_TYPE_ELEMENT_INIT_BINDING = TEXT("ElementInitBinding");
    static const FString EXPRESSION_TYPE_ASSIGNMENT_BINDING = TEXT("Assignment");
    static const FString EXPRESSION_TYPE_ADD = TEXT("Add");
    static const FString EXPRESSION_TYPE_ADD_CHECKED = TEXT("AddChecked");
    static const FString EXPRESSION_TYPE_SUBTRACT = TEXT("Subtract");
    static const FString EXPRESSION_TYPE_SUBTRACT_CHECKED = TEXT("SubtractChecked");
    static const FString EXPRESSION_TYPE_LEFT_SHIFT = TEXT("LeftShift");
    static const FString EXPRESSION_TYPE_RIGHT_SHIFT = TEXT("RightShift");
    static const FString EXPRESSION_TYPE_GREATER_THAN = TEXT("GreaterThan");
    static const FString EXPRESSION_TYPE_GREATER_THAN_OR_EQUAL = TEXT("GreaterThanOrEqual");
    static const FString EXPRESSION_TYPE_LESS_THAN = TEXT("LessThan");
    static const FString EXPRESSION_TYPE_LESS_THAN_OR_EQUAL = TEXT("LessThanOrEqual");
    static const FString EXPRESSION_TYPE_NEGATE = TEXT("Negate");
    static const FString EXPRESSION_TYPE_NEGATE_CHECKED = TEXT("NegateChecked");
    static const FString EXPRESSION_TYPE_POWER = TEXT("Power");
    static const FString EXPRESSION_TYPE_COMPLEMENT = TEXT("Complement");
    static const FString EXPRESSION_TYPE_DIVIDE = TEXT("Divide");
    static const FString EXPRESSION_TYPE_DIVIDE_CHECKED = TEXT("DivideChecked");
    static const FString EXPRESSION_TYPE_MULTIPLY = TEXT("Multiply");
    static const FString EXPRESSION_TYPE_MULTIPLY_CHECKED = TEXT("MultiplyChecked");
    static const FString EXPRESSION_TYPE_MODULO = TEXT("Modulo");
    static const FString EXPRESSION_TYPE_TYPE_IS = TEXT("TypeIs");
    static const FString EXPRESSION_TYPE_TYPE_AS = TEXT("TypeAs");
    static const FString EXPRESSION_TYPE_NOT = TEXT("Not");
    static const FString EXPRESSION_TYPE_EQUAL = TEXT("Equal");
    static const FString EXPRESSION_TYPE_NOT_EQUAL = TEXT("NotEqual");
    static const FString EXPRESSION_TYPE_AND = TEXT("And");
    static const FString EXPRESSION_TYPE_OR = TEXT("Or");
    static const FString EXPRESSION_TYPE_EXCLUSIVE_OR = TEXT("ExclusiveOr");
    static const FString EXPRESSION_TYPE_AND_ALSO = TEXT("AndAlso");
    static const FString EXPRESSION_TYPE_OR_ELSE = TEXT("OrElse");
    static const FString EXPRESSION_TYPE_COALESCE = TEXT("Coalesce");
    static const FString EXPRESSION_TYPE_CONDITION = TEXT("Condition");
    static const FString EXPRESSION_TYPE_UNARY_PLUS = TEXT("UnaryPlus");

    // Known types
    static const FString KNOWN_TYPE_ARRAY = TEXT("Array");
    static const FString KNOWN_TYPE_SYSTEM_VOID = TEXT("System.Void");
    static const FString KNOWN_TYPE_SYSTEM_CHAR = TEXT("System.Char");
    static const FString KNOWN_TYPE_SYSTEM_BOOL = TEXT("System.Boolean");
    static const FString KNOWN_TYPE_SYSTEM_BYTE = TEXT("System.Byte");
    static const FString KNOWN_TYPE_SYSTEM_SBYTE = TEXT("System.SByte");
    static const FString KNOWN_TYPE_SYSTEM_DECIMAL = TEXT("System.Decimal");
    static const FString KNOWN_TYPE_SYSTEM_DATE_TIME = TEXT("System.DateTime");
    static const FString KNOWN_TYPE_SYSTEM_TIME_SPAN = TEXT("System.TimeSpan");
    static const FString KNOWN_TYPE_SYSTEM_DOUBLE = TEXT("System.Double");
    static const FString KNOWN_TYPE_SYSTEM_SINGLE = TEXT("System.Single");
    static const FString KNOWN_TYPE_SYSTEM_INT32 = TEXT("System.Int32");
    static const FString KNOWN_TYPE_SYSTEM_UINT32 = TEXT("System.UInt32");
    static const FString KNOWN_TYPE_SYSTEM_INT64 = TEXT("System.Int64");
    static const FString KNOWN_TYPE_SYSTEM_UINT64 = TEXT("System.UInt64");
    static const FString KNOWN_TYPE_SYSTEM_OBJECT = TEXT("System.Object");
    static const FString KNOWN_TYPE_SYSTEM_INT16 = TEXT("System.Int16");
    static const FString KNOWN_TYPE_SYSTEM_UINT16 = TEXT("System.UInt16");
    static const FString KNOWN_TYPE_SYSTEM_STRING = TEXT("System.String");
    static const FString KNOWN_TYPE_SYSTEM_TYPE = TEXT("System.Type");
    static const FString KNOWN_TYPE_SYSTEM_ARRAY = TEXT("System.Array");

    // Notations
    static const FString NOTATION_TRUE_STRING = TEXT("true");
    static const FString NOTATION_FALSE_STRING = TEXT("false");
    static const FString NOTATION_NULL_STRING = TEXT("null");
}