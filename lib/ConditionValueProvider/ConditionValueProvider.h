#ifndef CONDITION_VALUE_PROVIDER_H
#define CONDITION_VALUE_PROVIDER_H

#include <ConditionEvaluationCommon.h>
#include <ConditionOperand.h>

class ConditionValueProvider
{
public:
    virtual ~ConditionValueProvider()=default;
    virtual ConditionResolveResult resolve(const ConditionOperand& reference,
                                             ConditionResolvedValue& output)const=0;
};

#endif
