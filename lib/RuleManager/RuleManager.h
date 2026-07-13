#ifndef RULE_MANAGER_H
#define RULE_MANAGER_H
#include <Rule.h>
class RuleManager
{
public:
    RuleManager(); void clear(); AutomationModelResult add(const Rule&); AutomationModelResult update(const Rule&); AutomationModelResult remove(RuleId);
    const Rule* findById(RuleId)const; const Rule* findByName(const char*)const; const Rule* getAt(size_t)const;
    bool contains(RuleId)const; size_t size()const; size_t capacity()const; bool isFull()const; bool isEmpty()const;
private: Rule rules_[RULE_MANAGER_CAPACITY];size_t count_;size_t findIndexById(RuleId)const;bool nameBelongsToAnother(const char*,RuleId)const;
};
#endif
