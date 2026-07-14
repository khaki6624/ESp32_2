#include "RuleExecutor.h"

#include <stdint.h>

RuleExecutor::RuleExecutor(const RuleManager& ruleManager, RuleTriggerQueue& triggerQueue,
    const AutomationCommandFactory& commandFactory, CommandIdProvider& commandIdProvider,
    RuntimeCommandSink& commandSink)
    : ruleManager_(ruleManager), triggerQueue_(triggerQueue), commandFactory_(commandFactory),
      commandIdProvider_(commandIdProvider), commandSink_(commandSink),
      state_(RuleExecutionState::IDLE), lastResult_(RuleExecutionResult::SUCCESS),
      currentTrigger_{}, currentActionArrayIndex_(0U),
      firstReservedCommandId_(INVALID_COMMAND_ID), nextCommandId_(INVALID_COMMAND_ID),
      activeCommandCount_(0U), delayStartedMs_(0U), delayDurationMs_(0U),
      reservedBranchFingerprint_(0U) {}

void RuleExecutor::update(uint32_t nowMs)
{
    if (isTerminalRuleExecutionState(state_))
    {
        clearCurrentRuntime();
        state_ = RuleExecutionState::IDLE;
        loadNextTrigger();
        return;
    }

    switch (state_)
    {
        case RuleExecutionState::IDLE: loadNextTrigger(); return;
        case RuleExecutionState::LOADING_TRIGGER:
        {
            const Rule* rule = nullptr;
            const RuleExecutionResult validation = validateCurrentRule(rule);
            if (validation != RuleExecutionResult::SUCCESS)
            { finishCurrent(RuleExecutionState::FAILED, validation); return; }
            activeCommandCount_ = countEnabledActions(*rule, currentTrigger_.branch);
            if (activeCommandCount_ == 0U)
            { finishCurrent(RuleExecutionState::COMPLETED, RuleExecutionResult::SUCCESS); return; }
            state_ = RuleExecutionState::RESERVING_COMMAND_IDS;
            return;
        }
        case RuleExecutionState::RESERVING_COMMAND_IDS:
        {
            const Rule* rule = nullptr;
            const RuleExecutionResult validation = validateCurrentRule(rule);
            if (validation != RuleExecutionResult::SUCCESS)
            { finishCurrent(RuleExecutionState::FAILED, validation); return; }
            const RuleExecutionResult result = reserveCommandIds(*rule);
            if (result != RuleExecutionResult::SUCCESS)
            { finishCurrent(RuleExecutionState::FAILED, result); return; }
            currentActionArrayIndex_ = 0U;
            state_ = RuleExecutionState::READY_ACTION;
            return;
        }
        case RuleExecutionState::READY_ACTION:
        {
            const RuleExecutionResult result = processReadyAction(nowMs);
            if (result != RuleExecutionResult::SUCCESS &&
                state_ != RuleExecutionState::WAITING_COMMAND_SINK)
                finishCurrent(RuleExecutionState::FAILED, result);
            return;
        }
        case RuleExecutionState::WAITING_DELAY:
            if (static_cast<uint32_t>(nowMs - delayStartedMs_) < delayDurationMs_) return;
            delayStartedMs_ = 0U;
            delayDurationMs_ = 0U;
            state_ = RuleExecutionState::WAITING_COMMAND_SINK;
            // پس از پایان Delay همان Action بدون آغاز دوباره‌ی Delay ارسال می‌شود.
            if (submitCurrentAction(nowMs) != RuleExecutionResult::SUCCESS)
            {
                const RuleExecutionResult result = lastResult_;
                if (state_ != RuleExecutionState::WAITING_COMMAND_SINK)
                    finishCurrent(RuleExecutionState::FAILED, result);
            }
            return;
        case RuleExecutionState::WAITING_COMMAND_SINK:
            if (commandSink_.isFull()) return;
            if (submitCurrentAction(nowMs) != RuleExecutionResult::SUCCESS)
            {
                const RuleExecutionResult result = lastResult_;
                if (state_ != RuleExecutionState::WAITING_COMMAND_SINK)
                    finishCurrent(RuleExecutionState::FAILED, result);
            }
            return;
        default: return;
    }
}

bool RuleExecutor::loadNextTrigger()
{
    const RuleTrigger* queued = triggerQueue_.peek();
    if (queued == nullptr) return false;
    currentTrigger_ = *queued;
    triggerQueue_.consume();
    // ناسازگاری Branch با Edge جداگانه در مرحله‌ی Rule گزارش می‌شود.
    if (currentTrigger_.ruleId == INVALID_RULE_ID || !currentTrigger_.request.isValid() ||
        currentTrigger_.triggerType == RuleTriggerType::NONE ||
        !isValidRuleTriggerType(currentTrigger_.triggerType))
    { finishCurrent(RuleExecutionState::FAILED, RuleExecutionResult::INVALID_TRIGGER); return false; }
    state_ = RuleExecutionState::LOADING_TRIGGER;
    return true;
}

RuleExecutionResult RuleExecutor::validateCurrentRule(const Rule*& outputRule) const
{
    outputRule = nullptr;
    if (currentTrigger_.ruleId == INVALID_RULE_ID) return RuleExecutionResult::INVALID_RULE_ID;
    if ((currentTrigger_.triggerType == RuleTriggerType::RISING_EDGE &&
         currentTrigger_.branch != RuleBranch::THEN_BRANCH) ||
        (currentTrigger_.triggerType == RuleTriggerType::FALLING_EDGE &&
         currentTrigger_.branch != RuleBranch::ELSE_BRANCH) ||
        (currentTrigger_.branch != RuleBranch::THEN_BRANCH &&
         currentTrigger_.branch != RuleBranch::ELSE_BRANCH))
        return RuleExecutionResult::INVALID_BRANCH;
    outputRule = ruleManager_.findById(currentTrigger_.ruleId);
    if (outputRule == nullptr) return RuleExecutionResult::RULE_NOT_FOUND;
    if (!outputRule->isValid()) return RuleExecutionResult::RULE_INVALID;
    if (!outputRule->enabled) return RuleExecutionResult::RULE_DISABLED;
    return RuleExecutionResult::SUCCESS;
}

size_t RuleExecutor::countEnabledActions(const Rule& rule, RuleBranch branch) const
{
    size_t count = 0U;
    for (size_t index = 0U; index < rule.actionCount(branch); ++index)
    {
        const RuleActionStep* step = rule.getActionStepAt(branch, index);
        if (step != nullptr && step->enabled) ++count;
    }
    return count;
}

const RuleActionStep* RuleExecutor::findNextEnabledAction(const Rule& rule, RuleBranch branch,
    size_t startIndex, size_t& foundIndex) const
{
    for (size_t index = startIndex; index < rule.actionCount(branch); ++index)
    {
        const RuleActionStep* step = rule.getActionStepAt(branch, index);
        if (step != nullptr && step->enabled) { foundIndex = index; return step; }
    }
    return nullptr;
}

RuleExecutionResult RuleExecutor::reserveCommandIds(const Rule& rule)
{
    const size_t currentCount = countEnabledActions(rule, currentTrigger_.branch);
    if (currentCount != activeCommandCount_ || currentCount == 0U)
        return RuleExecutionResult::RULE_INVALID;
    uint32_t fingerprint = 0U;
    if (!calculateBranchFingerprint(rule, currentTrigger_.branch, fingerprint))
        return RuleExecutionResult::RULE_INVALID;
    CommandId candidate = INVALID_COMMAND_ID;
    if (commandIdProvider_.reserveRange(currentCount, candidate) != CommandIdReservationResult::SUCCESS)
        return RuleExecutionResult::COMMAND_ID_RESERVATION_FAILED;
    if (candidate == INVALID_COMMAND_ID || currentCount - 1U > UINT32_MAX ||
        candidate > UINT32_MAX - static_cast<uint32_t>(currentCount - 1U))
        return RuleExecutionResult::INVALID_COMMAND_ID_RANGE;
    firstReservedCommandId_ = candidate;
    nextCommandId_ = candidate;
    reservedBranchFingerprint_ = fingerprint;
    return RuleExecutionResult::SUCCESS;
}

bool RuleExecutor::calculateBranchFingerprint(const Rule& rule, RuleBranch branch,
    uint32_t& output) const
{
    if (branch != RuleBranch::THEN_BRANCH && branch != RuleBranch::ELSE_BRANCH) return false;
    constexpr uint32_t FNV_OFFSET = 2166136261UL;
    constexpr uint32_t FNV_PRIME = 16777619UL;
    uint32_t hash = FNV_OFFSET;
    const auto addByte = [&hash](uint8_t value)
    { hash ^= value; hash *= FNV_PRIME; };
    const auto addUint32 = [&addByte](uint32_t value)
    {
        addByte(static_cast<uint8_t>(value));
        addByte(static_cast<uint8_t>(value >> 8U));
        addByte(static_cast<uint8_t>(value >> 16U));
        addByte(static_cast<uint8_t>(value >> 24U));
    };

    addByte(static_cast<uint8_t>(branch));
    const size_t slotCount = rule.actionCount(branch);
    addUint32(static_cast<uint32_t>(slotCount));
    for (size_t index = 0U; index < slotCount; ++index)
    {
        const RuleActionStep* step = rule.getActionStepAt(branch, index);
        if (step == nullptr || !step->isValid() || step->branch != branch ||
            !step->command.isValid()) return false;
        const AutomationCommand& command = step->command;
        addUint32(static_cast<uint32_t>(step->stepIndex));
        addByte(static_cast<uint8_t>(step->branch));
        addByte(step->enabled ? 1U : 0U);
        addUint32(step->delayBeforeMs);
        addByte(static_cast<uint8_t>(command.domain));
        addUint32(static_cast<uint32_t>(command.domainIndex));
        addByte(command.hasDomainIndex ? 1U : 0U);
        addUint32(static_cast<uint32_t>(command.operation));
        addByte(command.argumentCount);
        for (size_t argumentIndex = 0U; argumentIndex < command.argumentCount; ++argumentIndex)
        {
            const AutomationCommandArgument* argument = command.getArgument(argumentIndex);
            if (argument == nullptr || !argument->isValid()) return false;
            addByte(static_cast<uint8_t>(argument->type));
            switch (argument->type)
            {
                case AutomationArgumentType::BOOLEAN:
                    addByte(argument->booleanValue ? 1U : 0U); break;
                case AutomationArgumentType::INTEGER:
                    addUint32(static_cast<uint32_t>(argument->integerValue)); break;
                case AutomationArgumentType::FLOAT:
                {
                    // Float بدون خواندن حافظه خام به sign/exponent/significand شکسته می‌شود.
                    int exponent = 0;
                    const bool negative = signbit(argument->floatValue);
                    const float absoluteValue = fabsf(argument->floatValue);
                    const float fraction = frexpf(absoluteValue, &exponent);
                    const uint32_t significand = static_cast<uint32_t>(ldexpf(fraction, 24));
                    addByte(negative ? 1U : 0U);
                    addUint32(static_cast<uint32_t>(exponent));
                    addUint32(significand);
                    break;
                }
                case AutomationArgumentType::PERCENTAGE:
                    addByte(argument->percentageValue); break;
                case AutomationArgumentType::DURATION_MS:
                    addUint32(argument->durationMs); break;
                case AutomationArgumentType::IDENTIFIER:
                    addUint32(argument->identifierValue); break;
                case AutomationArgumentType::ENUM_VALUE:
                    addUint32(static_cast<uint32_t>(argument->enumValue)); break;
                case AutomationArgumentType::NONE: default: return false;
            }
        }
        addUint32(command.durationMs);
        addByte(command.hasDurationValue ? 1U : 0U);
    }
    output = hash;
    return true;
}

RuleExecutionResult RuleExecutor::processReadyAction(uint32_t nowMs)
{
    const Rule* rule = nullptr;
    const RuleExecutionResult validation = validateCurrentRule(rule);
    if (validation != RuleExecutionResult::SUCCESS) return validation;
    uint32_t fingerprint = 0U;
    if (!calculateBranchFingerprint(*rule, currentTrigger_.branch, fingerprint))
        return RuleExecutionResult::RULE_INVALID;
    if (fingerprint != reservedBranchFingerprint_)
        return RuleExecutionResult::RULE_CHANGED_DURING_EXECUTION;
    size_t foundIndex = 0U;
    const RuleActionStep* step = findNextEnabledAction(*rule, currentTrigger_.branch,
        currentActionArrayIndex_, foundIndex);
    if (step == nullptr)
    { finishCurrent(RuleExecutionState::COMPLETED, RuleExecutionResult::SUCCESS); return RuleExecutionResult::SUCCESS; }
    currentActionArrayIndex_ = foundIndex;
    if (!step->isValid() || step->branch != currentTrigger_.branch)
        return RuleExecutionResult::INVALID_ACTION;
    if (step->delayBeforeMs > 0U)
    {
        delayStartedMs_ = nowMs;
        delayDurationMs_ = step->delayBeforeMs;
        state_ = RuleExecutionState::WAITING_DELAY;
        return RuleExecutionResult::SUCCESS;
    }
    state_ = RuleExecutionState::WAITING_COMMAND_SINK;
    return submitCurrentAction(nowMs);
}

RuleExecutionResult RuleExecutor::submitCurrentAction(uint32_t nowMs)
{
    const Rule* rule = nullptr;
    RuleExecutionResult result = validateCurrentRule(rule);
    if (result != RuleExecutionResult::SUCCESS) { lastResult_ = result; state_ = RuleExecutionState::READY_ACTION; return result; }
    uint32_t fingerprint = 0U;
    if (!calculateBranchFingerprint(*rule, currentTrigger_.branch, fingerprint))
    { lastResult_ = RuleExecutionResult::RULE_INVALID; state_ = RuleExecutionState::READY_ACTION; return lastResult_; }
    if (fingerprint != reservedBranchFingerprint_)
    { lastResult_ = RuleExecutionResult::RULE_CHANGED_DURING_EXECUTION; state_ = RuleExecutionState::READY_ACTION; return lastResult_; }
    const RuleActionStep* step = rule->getActionStepAt(currentTrigger_.branch, currentActionArrayIndex_);
    if (step == nullptr) { lastResult_ = RuleExecutionResult::ACTION_NOT_FOUND; state_ = RuleExecutionState::READY_ACTION; return lastResult_; }
    if (!step->enabled || !step->isValid() || step->branch != currentTrigger_.branch)
    { lastResult_ = RuleExecutionResult::INVALID_ACTION; state_ = RuleExecutionState::READY_ACTION; return lastResult_; }
    const CommandId lastReservedCommandId = firstReservedCommandId_ +
        static_cast<uint32_t>(activeCommandCount_ - 1U);
    if (nextCommandId_ == INVALID_COMMAND_ID || nextCommandId_ < firstReservedCommandId_ ||
        nextCommandId_ > lastReservedCommandId)
    { lastResult_ = RuleExecutionResult::INVALID_COMMAND_ID_RANGE; state_ = RuleExecutionState::READY_ACTION; return lastResult_; }
    Command command;
    if (commandFactory_.create(step->command, currentTrigger_.request, nextCommandId_, nowMs, command) !=
        AutomationCommandFactoryResult::SUCCESS)
    { lastResult_ = RuleExecutionResult::COMMAND_FACTORY_FAILED; state_ = RuleExecutionState::READY_ACTION; return lastResult_; }
    if (commandSink_.isFull())
    { state_ = RuleExecutionState::WAITING_COMMAND_SINK; lastResult_ = RuleExecutionResult::COMMAND_SINK_FULL; return lastResult_; }
    const RuntimeCommandSubmitResult submitted = commandSink_.submit(command);
    if (submitted == RuntimeCommandSubmitResult::SINK_FULL)
    { state_ = RuleExecutionState::WAITING_COMMAND_SINK; lastResult_ = RuleExecutionResult::COMMAND_SINK_FULL; return lastResult_; }
    if (submitted != RuntimeCommandSubmitResult::SUCCESS)
    { state_ = RuleExecutionState::READY_ACTION; lastResult_ = RuleExecutionResult::COMMAND_SINK_REJECTED; return lastResult_; }

    ++currentActionArrayIndex_;
    size_t nextIndex = 0U;
    const RuleActionStep* next = findNextEnabledAction(*rule, currentTrigger_.branch,
        currentActionArrayIndex_, nextIndex);
    if (next == nullptr)
    { finishCurrent(RuleExecutionState::COMPLETED, RuleExecutionResult::SUCCESS); return RuleExecutionResult::SUCCESS; }
    if (nextCommandId_ == UINT32_MAX)
    { lastResult_ = RuleExecutionResult::INVALID_COMMAND_ID_RANGE; state_ = RuleExecutionState::READY_ACTION; return lastResult_; }
    ++nextCommandId_;
    state_ = RuleExecutionState::READY_ACTION;
    lastResult_ = RuleExecutionResult::SUCCESS;
    return RuleExecutionResult::SUCCESS;
}

RuleExecutionResult RuleExecutor::cancelCurrent()
{
    if (!isRunning()) return RuleExecutionResult::NOT_RUNNING;
    finishCurrent(RuleExecutionState::CANCELLED, RuleExecutionResult::CANCELLED);
    return RuleExecutionResult::CANCELLED;
}

void RuleExecutor::reset()
{
    clearCurrentRuntime();
    state_ = RuleExecutionState::IDLE;
    lastResult_ = RuleExecutionResult::SUCCESS;
}

RuleExecutionState RuleExecutor::getState() const { return state_; }
RuleExecutionResult RuleExecutor::getLastResult() const { return lastResult_; }
bool RuleExecutor::isRunning() const
{ return state_ != RuleExecutionState::IDLE && !isTerminalRuleExecutionState(state_); }
RuleId RuleExecutor::getCurrentRuleId() const { return currentTrigger_.ruleId; }
RuleBranch RuleExecutor::getCurrentBranch() const { return currentTrigger_.branch; }
AutomationStepIndex RuleExecutor::getCurrentStepIndex() const
{
    const Rule* rule = ruleManager_.findById(currentTrigger_.ruleId);
    const RuleActionStep* step = rule == nullptr ? nullptr :
        rule->getActionStepAt(currentTrigger_.branch, currentActionArrayIndex_);
    return step == nullptr ? INVALID_AUTOMATION_STEP_INDEX : step->stepIndex;
}

void RuleExecutor::finishCurrent(RuleExecutionState terminalState, RuleExecutionResult result)
{
    state_ = terminalState;
    lastResult_ = result;
    delayStartedMs_ = 0U;
    delayDurationMs_ = 0U;
}

void RuleExecutor::clearCurrentRuntime()
{
    currentTrigger_ = RuleTrigger{};
    currentActionArrayIndex_ = 0U;
    firstReservedCommandId_ = INVALID_COMMAND_ID;
    nextCommandId_ = INVALID_COMMAND_ID;
    activeCommandCount_ = 0U;
    delayStartedMs_ = 0U;
    delayDurationMs_ = 0U;
    reservedBranchFingerprint_ = 0U;
}
