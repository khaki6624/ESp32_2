#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <AutomationCommandFactory.h>
#include <CommandIdProvider.h>
#include <RuntimeCommandSink.h>
#include <ScheduleManager.h>
#include <ScheduleRuntimeState.h>
#include <SolarTimeProvider.h>
#include <TimeProvider.h>

class Scheduler
{
public:
    Scheduler(const ScheduleManager& scheduleManager, const TimeProvider& timeProvider,
        const SolarTimeProvider& solarTimeProvider, const AutomationCommandFactory& commandFactory,
        CommandIdProvider& commandIdProvider, RuntimeCommandSink& commandSink);
    void update(const RequestContext& request);
    void resetRuntimeStates();
    SchedulerState getState() const;
    SchedulerResult getLastResult() const;
    size_t getNextScheduleArrayIndex() const;
    size_t getNextCommandArrayIndex() const;
    size_t runtimeStateCount() const;
    size_t runtimeStateCapacity() const;
    const ScheduleRuntimeState* findRuntimeState(ScheduleId scheduleId,
        AutomationStepIndex commandIndex) const;
private:
    const ScheduleManager& scheduleManager_;
    const TimeProvider& timeProvider_;
    const SolarTimeProvider& solarTimeProvider_;
    const AutomationCommandFactory& commandFactory_;
    CommandIdProvider& commandIdProvider_;
    RuntimeCommandSink& commandSink_;
    ScheduleRuntimeState runtimeStates_[SCHEDULER_RUNTIME_STATE_CAPACITY];
    size_t runtimeStateCount_;
    size_t nextScheduleArrayIndex_;
    size_t nextCommandArrayIndex_;
    SchedulerState state_;
    SchedulerResult lastResult_;
    ScheduledCommand pendingCommand_;
    RequestContext pendingRequest_;
    CommandId pendingCommandId_;
    uint32_t pendingFingerprint_;
    AutomationDate pendingOccurrenceDate_;
    ScheduleRuntimeState* findRuntimeStateMutable(ScheduleId scheduleId,
        AutomationStepIndex commandIndex);
    ScheduleRuntimeState* findOrCreateRuntimeState(ScheduleId scheduleId,
        AutomationStepIndex commandIndex);
    SchedulerResult evaluateCommand(const Schedule& schedule, const ScheduledCommand& command,
        const TimeSnapshot& now, bool& due);
    SchedulerResult preparePendingExecution(const Schedule& schedule,
        const ScheduledCommand& command, const RequestContext& request, const TimeSnapshot& now);
    SchedulerResult submitPending(const TimeSnapshot& now);
    bool calculateCommandFingerprint(const ScheduledCommand& command, uint32_t& output) const;
    void advanceRoundRobin();
    void cleanupRemovedStates();
    void clearPending();
};

#endif
