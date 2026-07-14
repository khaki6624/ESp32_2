#include "Scheduler.h"

#include <math.h>

namespace
{
int compareDate(const AutomationDate& left, const AutomationDate& right)
{
    if (left.year != right.year) return left.year < right.year ? -1 : 1;
    if (left.month != right.month) return left.month < right.month ? -1 : 1;
    if (left.day != right.day) return left.day < right.day ? -1 : 1;
    return 0;
}
bool sameDate(const AutomationDate& left, const AutomationDate& right)
{ return compareDate(left, right) == 0; }
uint32_t secondsOf(const AutomationTime& value)
{ return static_cast<uint32_t>(value.hour) * 3600U +
         static_cast<uint32_t>(value.minute) * 60U + value.second; }
}

Scheduler::Scheduler(const ScheduleManager& scheduleManager, const TimeProvider& timeProvider,
    const SolarTimeProvider& solarTimeProvider, const AutomationCommandFactory& commandFactory,
    CommandIdProvider& commandIdProvider, RuntimeCommandSink& commandSink)
    : scheduleManager_(scheduleManager), timeProvider_(timeProvider),
      solarTimeProvider_(solarTimeProvider), commandFactory_(commandFactory),
      commandIdProvider_(commandIdProvider), commandSink_(commandSink), runtimeStates_{},
      runtimeStateCount_(0U), nextScheduleArrayIndex_(0U), nextCommandArrayIndex_(0U),
      state_(SchedulerState::IDLE), lastResult_(SchedulerResult::SUCCESS), pendingCommand_{},
      pendingRequest_{}, pendingCommandId_(INVALID_COMMAND_ID), pendingFingerprint_(0U),
      pendingOccurrenceDate_{} {}

void Scheduler::update(const RequestContext& request)
{
    if (isTerminalSchedulerState(state_)) { clearPending(); state_ = SchedulerState::IDLE; }
    if (!request.isValid() || request.source != CommandSource::SCHEDULER)
    { state_ = SchedulerState::FAILED; lastResult_ = SchedulerResult::INVALID_REQUEST_CONTEXT; return; }

    TimeSnapshot now;
    const TimeProviderResult timeResult = timeProvider_.getCurrentTime(now);
    if (timeResult != TimeProviderResult::SUCCESS)
    {
        lastResult_ = timeResult == TimeProviderResult::INVALID_TIME ?
            SchedulerResult::INVALID_TIME_SNAPSHOT : SchedulerResult::TIME_UNAVAILABLE;
        if (state_ == SchedulerState::WAITING_COMMAND_SINK) state_ = SchedulerState::WAITING_COMMAND_SINK;
        else state_ = SchedulerState::FAILED;
        return;
    }
    if (!now.isValid())
    {
        lastResult_ = SchedulerResult::INVALID_TIME_SNAPSHOT;
        if (state_ != SchedulerState::WAITING_COMMAND_SINK) state_ = SchedulerState::FAILED;
        return;
    }

    if (state_ == SchedulerState::WAITING_COMMAND_SINK)
    {
        const SchedulerResult result = submitPending(now);
        if (result == SchedulerResult::SUCCESS) state_ = SchedulerState::COMPLETED;
        else if (result != SchedulerResult::COMMAND_SINK_FULL) state_ = SchedulerState::FAILED;
        lastResult_ = result;
        return;
    }

    if (scheduleManager_.isEmpty())
    {
        nextScheduleArrayIndex_ = 0U; nextCommandArrayIndex_ = 0U;
        cleanupRemovedStates(); state_ = SchedulerState::IDLE;
        lastResult_ = SchedulerResult::SUCCESS; return;
    }
    if (nextScheduleArrayIndex_ >= scheduleManager_.size())
    { nextScheduleArrayIndex_ = 0U; nextCommandArrayIndex_ = 0U; cleanupRemovedStates(); }

    const Schedule* schedule = scheduleManager_.getAt(nextScheduleArrayIndex_);
    if (schedule == nullptr)
    { advanceRoundRobin(); lastResult_ = SchedulerResult::SCHEDULE_NOT_FOUND; return; }
    if (!schedule->isValid())
    { advanceRoundRobin(); state_ = SchedulerState::FAILED; lastResult_ = SchedulerResult::SCHEDULE_INVALID; return; }
    if (!schedule->enabled)
    {
        for (size_t index = 0U; index < runtimeStateCount_;)
        {
            if (runtimeStates_[index].scheduleId == schedule->id)
            { for (size_t move = index + 1U; move < runtimeStateCount_; ++move)
                  runtimeStates_[move - 1U] = runtimeStates_[move];
              --runtimeStateCount_; runtimeStates_[runtimeStateCount_].clear(); }
            else ++index;
        }
        ++nextScheduleArrayIndex_; nextCommandArrayIndex_ = 0U;
        if (nextScheduleArrayIndex_ >= scheduleManager_.size())
        { nextScheduleArrayIndex_ = 0U; cleanupRemovedStates(); }
        lastResult_ = SchedulerResult::SCHEDULE_DISABLED; return;
    }
    if (schedule->isEmpty())
    {
        ++nextScheduleArrayIndex_; nextCommandArrayIndex_ = 0U;
        if (nextScheduleArrayIndex_ >= scheduleManager_.size())
        { nextScheduleArrayIndex_ = 0U; cleanupRemovedStates(); }
        lastResult_ = SchedulerResult::QUEUE_EMPTY; return;
    }
    if (nextCommandArrayIndex_ >= schedule->commandCount) nextCommandArrayIndex_ = 0U;
    const ScheduledCommand* command = schedule->getCommandAt(nextCommandArrayIndex_);
    state_ = SchedulerState::CHECKING;
    if (command == nullptr)
    { advanceRoundRobin(); state_ = SchedulerState::FAILED; lastResult_ = SchedulerResult::COMMAND_NOT_FOUND; return; }
    if (!command->enabled)
    {
        ScheduleRuntimeState* existing = findRuntimeStateMutable(schedule->id, command->commandIndex);
        if (existing != nullptr)
        {
            const size_t index = static_cast<size_t>(existing - runtimeStates_);
            for (size_t move = index + 1U; move < runtimeStateCount_; ++move)
                runtimeStates_[move - 1U] = runtimeStates_[move];
            --runtimeStateCount_; runtimeStates_[runtimeStateCount_].clear();
        }
        advanceRoundRobin(); state_ = SchedulerState::IDLE;
        lastResult_ = SchedulerResult::COMMAND_DISABLED; return;
    }
    if (!command->isValid())
    { advanceRoundRobin(); state_ = SchedulerState::FAILED; lastResult_ = SchedulerResult::COMMAND_INVALID; return; }

    bool due = false;
    SchedulerResult result = evaluateCommand(*schedule, *command, now, due);
    advanceRoundRobin();
    if (result != SchedulerResult::SUCCESS && result != SchedulerResult::NOT_DUE)
    { state_ = SchedulerState::FAILED; lastResult_ = result; return; }
    if (!due) { state_ = SchedulerState::IDLE; lastResult_ = SchedulerResult::NOT_DUE; return; }
    result = preparePendingExecution(*schedule, *command, request, now);
    if (result == SchedulerResult::SUCCESS) state_ = SchedulerState::COMPLETED;
    else if (result != SchedulerResult::COMMAND_SINK_FULL) state_ = SchedulerState::FAILED;
    lastResult_ = result;
}

SchedulerResult Scheduler::evaluateCommand(const Schedule&, const ScheduledCommand& command,
    const TimeSnapshot& now, bool& due)
{
    due = false;
    if (command.mode == ScheduleMode::MANUAL) return SchedulerResult::NOT_DUE;
    uint32_t fingerprint = 0U;
    if (!calculateCommandFingerprint(command, fingerprint)) return SchedulerResult::COMMAND_INVALID;
    ScheduleRuntimeState* state = findOrCreateRuntimeState(command.scheduleId, command.commandIndex);
    if (state == nullptr) return SchedulerResult::RUNTIME_STATE_FULL;

    uint32_t targetSeconds = 0U;
    if (command.mode == ScheduleMode::SUNRISE_OFFSET || command.mode == ScheduleMode::SUNSET_OFFSET)
    {
        AutomationTime solar;
        const SolarTimeProviderResult solarResult = command.mode == ScheduleMode::SUNRISE_OFFSET ?
            solarTimeProvider_.getSunrise(now.date, solar) : solarTimeProvider_.getSunset(now.date, solar);
        if (solarResult != SolarTimeProviderResult::SUCCESS)
            return SchedulerResult::SOLAR_TIME_UNAVAILABLE;
        if (!solar.isValid()) return SchedulerResult::INVALID_SOLAR_TIME;
        const int32_t adjusted = static_cast<int32_t>(secondsOf(solar)) +
            command.solarOffsetMinutes * 60;
        if (adjusted < 0 || adjusted > 86399) return SchedulerResult::INVALID_SOLAR_TIME;
        targetSeconds = static_cast<uint32_t>(adjusted);
    }

    if (!state->initialized || state->commandFingerprint != fingerprint)
    {
        ScheduleRuntimeState baseline;
        baseline.scheduleId = command.scheduleId; baseline.commandIndex = command.commandIndex;
        baseline.initialized = true; baseline.commandFingerprint = fingerprint;
        if (command.mode == ScheduleMode::INTERVAL)
            baseline.lastExecutedMonotonicMs = now.monotonicMs;
        else if (command.mode == ScheduleMode::FIXED_DATE_TIME)
        {
            const int dateOrder = compareDate(now.date, command.date);
            baseline.fixedDateTimeCompleted = dateOrder > 0 ||
                (dateOrder == 0 && now.secondsSinceMidnight() >= secondsOf(command.time));
        }
        else
        {
            const uint32_t scheduled = command.mode == ScheduleMode::FIXED_TIME ?
                secondsOf(command.time) : targetSeconds;
            if (now.secondsSinceMidnight() >= scheduled) baseline.lastExecutedDate = now.date;
        }
        *state = baseline;
        return SchedulerResult::NOT_DUE;
    }

    switch (command.mode)
    {
        case ScheduleMode::FIXED_TIME:
            due = hasDay(command.daysMask, now.dayOfWeek) &&
                now.secondsSinceMidnight() >= secondsOf(command.time) &&
                !sameDate(state->lastExecutedDate, now.date); break;
        case ScheduleMode::FIXED_DATE_TIME:
        {
            const int dateOrder = compareDate(now.date, command.date);
            due = !state->fixedDateTimeCompleted && (dateOrder > 0 ||
                (dateOrder == 0 && now.secondsSinceMidnight() >= secondsOf(command.time)));
            break;
        }
        case ScheduleMode::INTERVAL:
            due = static_cast<uint32_t>(now.monotonicMs - state->lastExecutedMonotonicMs) >=
                command.intervalMs; break;
        case ScheduleMode::SUNRISE_OFFSET: case ScheduleMode::SUNSET_OFFSET:
            due = hasDay(command.daysMask, now.dayOfWeek) &&
                now.secondsSinceMidnight() >= targetSeconds &&
                !sameDate(state->lastExecutedDate, now.date); break;
        case ScheduleMode::MANUAL: case ScheduleMode::NONE: return SchedulerResult::NOT_DUE;
    }
    return due ? SchedulerResult::SUCCESS : SchedulerResult::NOT_DUE;
}

SchedulerResult Scheduler::preparePendingExecution(const Schedule&, const ScheduledCommand& command,
    const RequestContext& request, const TimeSnapshot& now)
{
    uint32_t fingerprint = 0U;
    if (!calculateCommandFingerprint(command, fingerprint)) return SchedulerResult::COMMAND_INVALID;
    CommandId candidate = INVALID_COMMAND_ID;
    state_ = SchedulerState::WAITING_COMMAND_ID;
    if (commandIdProvider_.reserveRange(1U, candidate) != CommandIdReservationResult::SUCCESS)
        return SchedulerResult::COMMAND_ID_RESERVATION_FAILED;
    if (candidate == INVALID_COMMAND_ID) return SchedulerResult::INVALID_COMMAND_ID;
    // تمام Pendingها فقط پس از موفقیت کامل Reservation با هم Commit می‌شوند.
    pendingCommand_ = command; pendingRequest_ = request; pendingCommandId_ = candidate;
    pendingFingerprint_ = fingerprint; pendingOccurrenceDate_ = now.date;
    state_ = SchedulerState::WAITING_COMMAND_SINK;
    return submitPending(now);
}

SchedulerResult Scheduler::submitPending(const TimeSnapshot& now)
{
    const Schedule* schedule = scheduleManager_.findById(pendingCommand_.scheduleId);
    if (schedule == nullptr || !schedule->enabled || !schedule->isValid())
        return SchedulerResult::SCHEDULE_CHANGED_DURING_EXECUTION;
    const ScheduledCommand* command = schedule->findCommand(pendingCommand_.commandIndex);
    uint32_t fingerprint = 0U;
    if (command == nullptr || !command->enabled || !command->isValid() ||
        !calculateCommandFingerprint(*command, fingerprint) || fingerprint != pendingFingerprint_)
        return SchedulerResult::SCHEDULE_CHANGED_DURING_EXECUTION;
    if (pendingCommandId_ == INVALID_COMMAND_ID) return SchedulerResult::INVALID_COMMAND_ID;
    ScheduleRuntimeState* runtime = findRuntimeStateMutable(command->scheduleId, command->commandIndex);
    if (runtime == nullptr || runtime->commandFingerprint != pendingFingerprint_)
        return SchedulerResult::SCHEDULE_CHANGED_DURING_EXECUTION;
    const bool dateBased = command->mode == ScheduleMode::FIXED_TIME ||
        command->mode == ScheduleMode::FIXED_DATE_TIME ||
        command->mode == ScheduleMode::SUNRISE_OFFSET ||
        command->mode == ScheduleMode::SUNSET_OFFSET;
    if (dateBased && !pendingOccurrenceDate_.isValid())
        return SchedulerResult::SCHEDULE_CHANGED_DURING_EXECUTION;
    Command runtimeCommand;
    if (commandFactory_.create(pendingCommand_.command, pendingRequest_, pendingCommandId_,
        now.monotonicMs, runtimeCommand) != AutomationCommandFactoryResult::SUCCESS)
        return SchedulerResult::COMMAND_FACTORY_FAILED;
    if (commandSink_.isFull()) return SchedulerResult::COMMAND_SINK_FULL;
    const RuntimeCommandSubmitResult submitted = commandSink_.submit(runtimeCommand);
    if (submitted == RuntimeCommandSubmitResult::SINK_FULL) return SchedulerResult::COMMAND_SINK_FULL;
    if (submitted != RuntimeCommandSubmitResult::SUCCESS) return SchedulerResult::COMMAND_SINK_REJECTED;
    // Fixed و Solar روز وقوع اصلی را ثبت می‌کنند؛ Interval از Submit واقعی آغاز می‌شود.
    if (command->mode == ScheduleMode::INTERVAL)
        runtime->lastExecutedMonotonicMs = now.monotonicMs;
    else if (dateBased)
        runtime->lastExecutedDate = pendingOccurrenceDate_;
    if (command->mode == ScheduleMode::FIXED_DATE_TIME)
        runtime->fixedDateTimeCompleted = true;
    return SchedulerResult::SUCCESS;
}

ScheduleRuntimeState* Scheduler::findRuntimeStateMutable(ScheduleId scheduleId,
    AutomationStepIndex commandIndex)
{ for (size_t i=0U;i<runtimeStateCount_;++i) if(runtimeStates_[i].scheduleId==scheduleId &&
    runtimeStates_[i].commandIndex==commandIndex)return &runtimeStates_[i]; return nullptr; }
const ScheduleRuntimeState* Scheduler::findRuntimeState(ScheduleId scheduleId,
    AutomationStepIndex commandIndex) const
{ for (size_t i=0U;i<runtimeStateCount_;++i) if(runtimeStates_[i].scheduleId==scheduleId &&
    runtimeStates_[i].commandIndex==commandIndex)return &runtimeStates_[i]; return nullptr; }
ScheduleRuntimeState* Scheduler::findOrCreateRuntimeState(ScheduleId scheduleId,
    AutomationStepIndex commandIndex)
{
    ScheduleRuntimeState* found=findRuntimeStateMutable(scheduleId,commandIndex);if(found)return found;
    if(runtimeStateCount_>=SCHEDULER_RUNTIME_STATE_CAPACITY)return nullptr;
    ScheduleRuntimeState& value=runtimeStates_[runtimeStateCount_++];value.clear();
    value.scheduleId=scheduleId;value.commandIndex=commandIndex;return &value;
}

bool Scheduler::calculateCommandFingerprint(const ScheduledCommand& value, uint32_t& output) const
{
    if(!value.isValid())return false;uint32_t hash=2166136261UL;
    const auto b=[&hash](uint8_t v){hash^=v;hash*=16777619UL;};
    const auto u=[&b](uint32_t v){b(static_cast<uint8_t>(v));b(static_cast<uint8_t>(v>>8U));
        b(static_cast<uint8_t>(v>>16U));b(static_cast<uint8_t>(v>>24U));};
    u(value.scheduleId);u(value.commandIndex);b(value.enabled?1U:0U);b(static_cast<uint8_t>(value.mode));
    u(value.date.year);b(value.date.month);b(value.date.day);b(value.time.hour);b(value.time.minute);
    b(value.time.second);u(value.intervalMs);u(static_cast<uint32_t>(value.solarOffsetMinutes));b(value.daysMask);
    const AutomationCommand& c=value.command;b(static_cast<uint8_t>(c.domain));u(c.domainIndex);
    b(c.hasDomainIndex?1U:0U);u(static_cast<uint32_t>(c.operation));b(c.argumentCount);
    for(size_t i=0U;i<c.argumentCount;++i){const AutomationCommandArgument* a=c.getArgument(i);
        if(!a||!a->isValid())return false;b(static_cast<uint8_t>(a->type));switch(a->type){
        case AutomationArgumentType::BOOLEAN:b(a->booleanValue?1U:0U);break;
        case AutomationArgumentType::INTEGER:u(static_cast<uint32_t>(a->integerValue));break;
        case AutomationArgumentType::FLOAT:{int e=0;const bool n=signbit(a->floatValue);
            const uint32_t s=static_cast<uint32_t>(ldexpf(frexpf(fabsf(a->floatValue),&e),24));
            b(n?1U:0U);u(static_cast<uint32_t>(e));u(s);break;}
        case AutomationArgumentType::PERCENTAGE:b(a->percentageValue);break;
        case AutomationArgumentType::DURATION_MS:u(a->durationMs);break;
        case AutomationArgumentType::IDENTIFIER:u(a->identifierValue);break;
        case AutomationArgumentType::ENUM_VALUE:u(static_cast<uint32_t>(a->enumValue));break;
        default:return false;}}
    u(c.durationMs);b(c.hasDurationValue?1U:0U);output=hash==0U?1U:hash;return true;
}

void Scheduler::advanceRoundRobin()
{
    const Schedule* current=scheduleManager_.getAt(nextScheduleArrayIndex_);++nextCommandArrayIndex_;
    if(current==nullptr||nextCommandArrayIndex_>=current->commandCount)
    {nextCommandArrayIndex_=0U;++nextScheduleArrayIndex_;if(nextScheduleArrayIndex_>=scheduleManager_.size())
        {nextScheduleArrayIndex_=0U;cleanupRemovedStates();}}
}
void Scheduler::cleanupRemovedStates()
{
    for(size_t i=0U;i<runtimeStateCount_;){const Schedule* s=scheduleManager_.findById(runtimeStates_[i].scheduleId);
        const ScheduledCommand* c=s?s->findCommand(runtimeStates_[i].commandIndex):nullptr;
        if(s==nullptr||!s->enabled||c==nullptr||!c->enabled){for(size_t m=i+1U;m<runtimeStateCount_;++m)
            runtimeStates_[m-1U]=runtimeStates_[m];--runtimeStateCount_;runtimeStates_[runtimeStateCount_].clear();}
        else ++i;}
}
void Scheduler::clearPending(){pendingCommand_.clear();pendingRequest_=RequestContext{};
    pendingCommandId_=INVALID_COMMAND_ID;pendingFingerprint_=0U;pendingOccurrenceDate_.clear();}
void Scheduler::resetRuntimeStates(){for(size_t i=0U;i<SCHEDULER_RUNTIME_STATE_CAPACITY;++i)
    runtimeStates_[i].clear();runtimeStateCount_=0U;nextScheduleArrayIndex_=0U;
    nextCommandArrayIndex_=0U;state_=SchedulerState::IDLE;lastResult_=SchedulerResult::SUCCESS;clearPending();}
SchedulerState Scheduler::getState()const{return state_;}SchedulerResult Scheduler::getLastResult()const{return lastResult_;}
size_t Scheduler::getNextScheduleArrayIndex()const{return nextScheduleArrayIndex_;}
size_t Scheduler::getNextCommandArrayIndex()const{return nextCommandArrayIndex_;}
size_t Scheduler::runtimeStateCount()const{return runtimeStateCount_;}
size_t Scheduler::runtimeStateCapacity()const{return SCHEDULER_RUNTIME_STATE_CAPACITY;}
