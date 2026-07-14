#include "ConditionEvaluator.h"

ConditionEvaluator::ConditionEvaluator(const ConditionValueProvider& valueProvider):valueProvider_(valueProvider){}

ConditionResolveResult ConditionEvaluator::resolveOperand(const ConditionOperand& operand,
    ConditionResolvedValue& output)const
{
    if(!operand.isValid())return ConditionResolveResult::INVALID_OPERAND;
    ConditionResolvedValue resolved;
    if(operand.isConstant())
    {
        switch(operand.valueType)
        {
            case ConditionValueType::BOOLEAN:resolved=ConditionResolvedValue::makeBoolean(operand.booleanValue);break;
            case ConditionValueType::INTEGER:resolved=ConditionResolvedValue::makeInteger(operand.integerValue);break;
            case ConditionValueType::FLOAT:resolved=ConditionResolvedValue::makeFloat(operand.floatValue);break;
            case ConditionValueType::PERCENTAGE:resolved=ConditionResolvedValue::makePercentage(operand.percentageValue);break;
            case ConditionValueType::ENUM_VALUE:resolved=ConditionResolvedValue::makeEnum(operand.enumValue);break;
            default:return ConditionResolveResult::INVALID_OPERAND;
        }
    }
    else
    {
        const ConditionResolveResult providerResult=valueProvider_.resolve(operand,resolved);
        if(providerResult!=ConditionResolveResult::SUCCESS)return providerResult;
    }
    if(!resolved.isValid())return ConditionResolveResult::PROVIDER_ERROR;
    if(resolved.type!=operand.valueType)return ConditionResolveResult::VALUE_TYPE_MISMATCH;
    output=resolved;
    return ConditionResolveResult::SUCCESS;
}

namespace
{
template<typename T> bool compareOrdered(T left,ComparisonOperator op,T right,bool& output)
{
    switch(op)
    {
        case ComparisonOperator::EQUAL:output=left==right;return true;
        case ComparisonOperator::NOT_EQUAL:output=left!=right;return true;
        case ComparisonOperator::LESS_THAN:output=left<right;return true;
        case ComparisonOperator::LESS_THAN_OR_EQUAL:output=left<=right;return true;
        case ComparisonOperator::GREATER_THAN:output=left>right;return true;
        case ComparisonOperator::GREATER_THAN_OR_EQUAL:output=left>=right;return true;
        default:return false;
    }
}
}

bool ConditionEvaluator::compareValues(const ConditionResolvedValue& left,ComparisonOperator op,
    const ConditionResolvedValue& right,bool& output)const
{
    if(!left.isValid()||!right.isValid()||left.type!=right.type)return false;
    if(left.type==ConditionValueType::BOOLEAN)
    {
        if(op==ComparisonOperator::EQUAL){output=left.booleanValue==right.booleanValue;return true;}
        if(op==ComparisonOperator::NOT_EQUAL){output=left.booleanValue!=right.booleanValue;return true;}
        return false;
    }
    if(left.type==ConditionValueType::ENUM_VALUE)
    {
        if(op==ComparisonOperator::EQUAL){output=left.enumValue==right.enumValue;return true;}
        if(op==ComparisonOperator::NOT_EQUAL){output=left.enumValue!=right.enumValue;return true;}
        return false;
    }
    if(left.type==ConditionValueType::INTEGER)return compareOrdered(left.integerValue,op,right.integerValue,output);
    if(left.type==ConditionValueType::FLOAT)return compareOrdered(left.floatValue,op,right.floatValue,output);
    if(left.type==ConditionValueType::PERCENTAGE)return compareOrdered(left.percentageValue,op,right.percentageValue,output);
    return false;
}

ConditionEvaluatorResult ConditionEvaluator::evaluateComparison(const ConditionComparison& comparison,
    ConditionEvaluationResult& output)const
{
    output=ConditionEvaluationResult::ERROR_RESULT;
    if(!isValidComparisonOperator(comparison.comparisonOperator)||comparison.comparisonOperator==ComparisonOperator::NONE)
        return ConditionEvaluatorResult::INVALID_COMPARISON;
    if(!comparison.left.isValid()||!comparison.right.isValid())
    {
        if((comparison.left.valueType==ConditionValueType::FLOAT&&!isfinite(comparison.left.floatValue))||
           (comparison.right.valueType==ConditionValueType::FLOAT&&!isfinite(comparison.right.floatValue)))
            return ConditionEvaluatorResult::NON_FINITE_VALUE;
        return ConditionEvaluatorResult::INVALID_COMPARISON;
    }
    if(comparison.left.valueType!=comparison.right.valueType)return ConditionEvaluatorResult::TYPE_MISMATCH;
    ConditionResolvedValue left;ConditionResolvedValue right;
    const ConditionResolveResult leftResult=resolveOperand(comparison.left,left);
    if(leftResult!=ConditionResolveResult::SUCCESS)
        return leftResult==ConditionResolveResult::VALUE_TYPE_MISMATCH?ConditionEvaluatorResult::TYPE_MISMATCH:ConditionEvaluatorResult::RESOLVE_FAILED;
    const ConditionResolveResult rightResult=resolveOperand(comparison.right,right);
    if(rightResult!=ConditionResolveResult::SUCCESS)
        return rightResult==ConditionResolveResult::VALUE_TYPE_MISMATCH?ConditionEvaluatorResult::TYPE_MISMATCH:ConditionEvaluatorResult::RESOLVE_FAILED;
    bool logicalResult=false;
    if(!compareValues(left,comparison.comparisonOperator,right,logicalResult))return ConditionEvaluatorResult::UNSUPPORTED_OPERATOR;
    output=logicalResult?ConditionEvaluationResult::TRUE_RESULT:ConditionEvaluationResult::FALSE_RESULT;
    return ConditionEvaluatorResult::SUCCESS;
}

ConditionEvaluatorResult ConditionEvaluator::evaluate(const ConditionExpression& expression,
    ConditionEvaluationResult& output)const
{
    output=ConditionEvaluationResult::ERROR_RESULT;
    if(!expression.isValid())return ConditionEvaluatorResult::INVALID_EXPRESSION;
    ConditionEvaluationResult comparisonResult;
    ConditionEvaluatorResult result=evaluateComparison(expression.comparisons[0],comparisonResult);
    if(result!=ConditionEvaluatorResult::SUCCESS)return result;
    bool currentAndGroup=comparisonResult==ConditionEvaluationResult::TRUE_RESULT;
    bool finalOrResult=false;
    for(size_t index=1U;index<expression.comparisonCount;++index)
    {
        result=evaluateComparison(expression.comparisons[index],comparisonResult);
        if(result!=ConditionEvaluatorResult::SUCCESS)return result;
        const bool next=comparisonResult==ConditionEvaluationResult::TRUE_RESULT;
        if(expression.operators[index-1U]==LogicalOperator::AND)currentAndGroup=currentAndGroup&&next;
        else{finalOrResult=finalOrResult||currentAndGroup;currentAndGroup=next;}
    }
    output=(finalOrResult||currentAndGroup)?ConditionEvaluationResult::TRUE_RESULT:ConditionEvaluationResult::FALSE_RESULT;
    return ConditionEvaluatorResult::SUCCESS;
}
