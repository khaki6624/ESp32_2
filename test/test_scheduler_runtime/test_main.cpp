#include <Scheduler.h>
#include <SceneExecutionQueueSink.h>
#include <Arduino.h>
#include <unity.h>

class FakeTimeProvider final : public TimeProvider
{
public:
    mutable size_t calls; TimeProviderResult result; TimeSnapshot value;
    FakeTimeProvider():calls(0U),result(TimeProviderResult::SUCCESS),value{}{}
    TimeProviderResult getCurrentTime(TimeSnapshot& output)const override
    {++calls;if(result==TimeProviderResult::SUCCESS)output=value;return result;}
};
class FakeSolarTimeProvider final : public SolarTimeProvider
{
public:
    SolarTimeProviderResult sunriseResult;SolarTimeProviderResult sunsetResult;
    AutomationTime sunrise;AutomationTime sunset;
    FakeSolarTimeProvider():sunriseResult(SolarTimeProviderResult::SUCCESS),
        sunsetResult(SolarTimeProviderResult::SUCCESS),sunrise{},sunset{}{}
    SolarTimeProviderResult getSunrise(const AutomationDate&,AutomationTime& output)const override
    {if(sunriseResult==SolarTimeProviderResult::SUCCESS)output=sunrise;return sunriseResult;}
    SolarTimeProviderResult getSunset(const AutomationDate&,AutomationTime& output)const override
    {if(sunsetResult==SolarTimeProviderResult::SUCCESS)output=sunset;return sunsetResult;}
};
class FakeSchedulerIdProvider final : public CommandIdProvider
{
public: CommandIdReservationResult result;CommandId next;size_t calls;size_t count;
    FakeSchedulerIdProvider():result(CommandIdReservationResult::SUCCESS),next(100U),calls(0U),count(0U){}
    CommandIdReservationResult reserveRange(size_t value,CommandId& output)override
    {++calls;count=value;if(result==CommandIdReservationResult::SUCCESS)output=next;return result;}
};

static ScheduleManager managerValue;static FakeTimeProvider timeValue;
static FakeSolarTimeProvider solarValue;static AutomationCommandFactory factoryValue;
static FakeSchedulerIdProvider idValue;static SceneExecutionQueue queueValue;
static SceneExecutionQueueSink sinkValue(queueValue);
static Scheduler schedulerValue(managerValue,timeValue,solarValue,factoryValue,idValue,sinkValue);

static AutomationDate dateValue(uint16_t y,uint8_t m,uint8_t d)
{AutomationDate v;v.year=y;v.month=m;v.day=d;return v;}
static AutomationTime timeOf(uint8_t h,uint8_t m,uint8_t s=0U)
{AutomationTime v;v.hour=h;v.minute=m;v.second=s;return v;}
static TimeSnapshot snapshot(uint8_t h,uint8_t m,uint32_t monotonic=0U,uint8_t day=1U)
{TimeSnapshot v;v.date=dateValue(2026,7,14);v.time=timeOf(h,m);v.dayOfWeek=static_cast<DayOfWeek>(day);
 v.monotonicMs=monotonic;v.valid=true;return v;}
static RequestContext requestValue()
{RequestContext r;r.requestId=77U;r.source=CommandSource::SCHEDULER;return r;}
static AutomationCommand action()
{AutomationCommand c;c.domain=CommandDomain::OUT;c.domainIndex=1U;c.hasDomainIndex=true;
 c.operation=CommandOperation::ON;return c;}
static ScheduledCommand fixedCommand(AutomationStepIndex index=1U)
{ScheduledCommand c;c.scheduleId=1U;c.commandIndex=index;c.command=action();c.mode=ScheduleMode::FIXED_TIME;
 c.time=timeOf(10,0);c.daysMask=allDaysMask();c.enabled=true;return c;}
static Schedule scheduleWith(const ScheduledCommand& command)
{Schedule s;s.id=1U;s.setName("Scheduler Test");s.enabled=true;s.addCommand(command);return s;}
static void resetFixture(){schedulerValue.resetRuntimeStates();managerValue.clear();queueValue.clear();
 timeValue=FakeTimeProvider{};solarValue=FakeSolarTimeProvider{};idValue=FakeSchedulerIdProvider{};}

void testTimeSnapshotAndProviders()
{
    TimeSnapshot value;TEST_ASSERT_FALSE(value.isValid());value=snapshot(12,34,0U,2U);
    TEST_ASSERT_TRUE(value.isValid());TEST_ASSERT_EQUAL_UINT32(45240U,value.secondsSinceMidnight());
    value.dayOfWeek=DayOfWeek::NONE;TEST_ASSERT_FALSE(value.isValid());value.clear();TEST_ASSERT_FALSE(value.isValid());
    TimeSnapshot unchanged=snapshot(1,2);timeValue.result=TimeProviderResult::TIME_UNAVAILABLE;
    timeValue.value=snapshot(3,4);TEST_ASSERT_TRUE(timeValue.getCurrentTime(unchanged)==TimeProviderResult::TIME_UNAVAILABLE);
    TEST_ASSERT_EQUAL_UINT8(1U,unchanged.time.hour);
    AutomationTime solar=timeOf(1,1);solarValue.sunrise=timeOf(6,0);
    TEST_ASSERT_TRUE(solarValue.getSunrise(dateValue(2026,7,14),solar)==SolarTimeProviderResult::SUCCESS);
    TEST_ASSERT_EQUAL_UINT8(6U,solar.hour);solarValue.sunriseResult=SolarTimeProviderResult::SUNRISE_UNAVAILABLE;
    TEST_ASSERT_TRUE(solarValue.getSunrise(dateValue(2026,7,14),solar)!=SolarTimeProviderResult::SUCCESS);
    TEST_ASSERT_EQUAL_UINT8(6U,solar.hour);
}

void testFixedTimeBaselineAndDailyCommit()
{
    resetFixture();managerValue.add(scheduleWith(fixedCommand()));timeValue.value=snapshot(9,0);
    schedulerValue.update(requestValue());TEST_ASSERT_TRUE(queueValue.isEmpty());TEST_ASSERT_EQUAL_UINT32(1U,schedulerValue.runtimeStateCount());
    timeValue.value=snapshot(10,0,10U);schedulerValue.update(requestValue());
    TEST_ASSERT_EQUAL_UINT32(1U,queueValue.size());TEST_ASSERT_EQUAL_UINT32(100U,queueValue.peek()->context.commandId);
    TEST_ASSERT_EQUAL_UINT32(77U,queueValue.peek()->context.request.requestId);
    TEST_ASSERT_EQUAL_UINT32(10U,queueValue.peek()->context.createdTimestampMs);
    schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(1U,queueValue.size());

    resetFixture();managerValue.add(scheduleWith(fixedCommand()));timeValue.value=snapshot(20,0);
    schedulerValue.update(requestValue());schedulerValue.update(requestValue());TEST_ASSERT_TRUE(queueValue.isEmpty());
}

void testFixedDateIntervalAndWrap()
{
    resetFixture();ScheduledCommand c=fixedCommand();c.mode=ScheduleMode::FIXED_DATE_TIME;c.daysMask=0U;
    c.date=dateValue(2026,7,14);c.time=timeOf(10,0);managerValue.add(scheduleWith(c));
    timeValue.value=snapshot(9,0);schedulerValue.update(requestValue());timeValue.value=snapshot(10,0,5U);
    schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(1U,queueValue.size());
    schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(1U,queueValue.size());

    resetFixture();c=fixedCommand();c.mode=ScheduleMode::INTERVAL;c.time.clear();c.daysMask=0U;c.intervalMs=5U;
    managerValue.add(scheduleWith(c));timeValue.value=snapshot(0,0,UINT32_MAX-2U);schedulerValue.update(requestValue());
    timeValue.value=snapshot(0,0,1U);schedulerValue.update(requestValue());TEST_ASSERT_TRUE(queueValue.isEmpty());
    timeValue.value=snapshot(0,0,2U);schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(1U,queueValue.size());
}

void testSolarManualAndInvalidOffset()
{
    resetFixture();ScheduledCommand c=fixedCommand();c.mode=ScheduleMode::SUNRISE_OFFSET;c.time.clear();
    c.solarOffsetMinutes=30;solarValue.sunrise=timeOf(6,0);managerValue.add(scheduleWith(c));
    timeValue.value=snapshot(6,0);schedulerValue.update(requestValue());timeValue.value=snapshot(6,30);
    schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(1U,queueValue.size());

    resetFixture();c=fixedCommand();c.mode=ScheduleMode::SUNRISE_OFFSET;c.time.clear();c.solarOffsetMinutes=-60;
    solarValue.sunrise=timeOf(0,30);managerValue.add(scheduleWith(c));timeValue.value=snapshot(0,0);
    schedulerValue.update(requestValue());TEST_ASSERT_TRUE(schedulerValue.getLastResult()==SchedulerResult::INVALID_SOLAR_TIME);

    resetFixture();c=fixedCommand();c.mode=ScheduleMode::MANUAL;c.time.clear();c.daysMask=0U;
    managerValue.add(scheduleWith(c));timeValue.value=snapshot(12,0);schedulerValue.update(requestValue());
    TEST_ASSERT_TRUE(queueValue.isEmpty());TEST_ASSERT_EQUAL_UINT32(0U,idValue.calls);
}

void testBackpressureAndPendingMutation()
{
    resetFixture();Schedule schedule=scheduleWith(fixedCommand());managerValue.add(schedule);
    timeValue.value=snapshot(9,0);schedulerValue.update(requestValue());
    Command filler;RequestContext request=requestValue();factoryValue.create(action(),request,1U,0U,filler);
    while(!queueValue.isFull())queueValue.enqueue(filler);timeValue.value=snapshot(10,0,20U);
    schedulerValue.update(requestValue());TEST_ASSERT_TRUE(schedulerValue.getState()==SchedulerState::WAITING_COMMAND_SINK);
    TEST_ASSERT_EQUAL_UINT32(1U,idValue.calls);const ScheduleRuntimeState* state=schedulerValue.findRuntimeState(1U,1U);
    TEST_ASSERT_TRUE(state->lastExecutedDate.isEmpty());schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(1U,idValue.calls);
    queueValue.consume();schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(8U,queueValue.size());
    TEST_ASSERT_EQUAL_UINT32(100U,queueValue.getAt(7U)->context.commandId);

    resetFixture();schedule=scheduleWith(fixedCommand());managerValue.add(schedule);timeValue.value=snapshot(9,0);
    schedulerValue.update(requestValue());while(!queueValue.isFull())queueValue.enqueue(filler);
    timeValue.value=snapshot(10,0);schedulerValue.update(requestValue());schedule.commands[0].time=timeOf(11,0);
    managerValue.update(schedule);queueValue.consume();schedulerValue.update(requestValue());
    TEST_ASSERT_TRUE(schedulerValue.getLastResult()==SchedulerResult::SCHEDULE_CHANGED_DURING_EXECUTION);
    TEST_ASSERT_EQUAL_UINT32(7U,queueValue.size());
}

void testDisabledRoundRobinCleanupAndRequest()
{
    resetFixture();Schedule s=scheduleWith(fixedCommand());managerValue.add(s);timeValue.value=snapshot(9,0);
    schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(1U,schedulerValue.runtimeStateCount());
    s.commands[0].enabled=false;managerValue.update(s);schedulerValue.update(requestValue());
    TEST_ASSERT_EQUAL_UINT32(0U,schedulerValue.runtimeStateCount());
    RequestContext bad=requestValue();bad.source=CommandSource::APP;schedulerValue.update(bad);
    TEST_ASSERT_TRUE(schedulerValue.getLastResult()==SchedulerResult::INVALID_REQUEST_CONTEXT);
}

void testFixedTimeOccurrenceDateAcrossMidnight()
{
    resetFixture();ScheduledCommand command=fixedCommand();command.time=timeOf(23,59);
    managerValue.add(scheduleWith(command));timeValue.value=snapshot(23,58,1U);
    schedulerValue.update(requestValue());
    Command filler;factoryValue.create(action(),requestValue(),1U,0U,filler);
    while(!queueValue.isFull())queueValue.enqueue(filler);
    timeValue.value=snapshot(23,59,2U);schedulerValue.update(requestValue());
    TEST_ASSERT_TRUE(schedulerValue.getState()==SchedulerState::WAITING_COMMAND_SINK);
    timeValue.value=snapshot(0,1,3U);timeValue.value.date=dateValue(2026,7,15);
    timeValue.value.dayOfWeek=DayOfWeek::WEDNESDAY;queueValue.consume();schedulerValue.update(requestValue());
    const ScheduleRuntimeState* state=schedulerValue.findRuntimeState(1U,1U);
    TEST_ASSERT_NOT_NULL(state);TEST_ASSERT_EQUAL_UINT16(2026U,state->lastExecutedDate.year);
    TEST_ASSERT_EQUAL_UINT8(7U,state->lastExecutedDate.month);TEST_ASSERT_EQUAL_UINT8(14U,state->lastExecutedDate.day);
    queueValue.consume();timeValue.value=snapshot(23,59,4U);timeValue.value.date=dateValue(2026,7,15);
    timeValue.value.dayOfWeek=DayOfWeek::WEDNESDAY;schedulerValue.update(requestValue());
    TEST_ASSERT_EQUAL_UINT32(2U,idValue.calls);
    TEST_ASSERT_EQUAL_UINT8(15U,state->lastExecutedDate.day);
}

void testSolarOccurrenceDateAndIntervalSubmitTime()
{
    resetFixture();ScheduledCommand command=fixedCommand();command.mode=ScheduleMode::SUNRISE_OFFSET;
    command.time.clear();command.solarOffsetMinutes=0;solarValue.sunrise=timeOf(6,0);
    managerValue.add(scheduleWith(command));timeValue.value=snapshot(5,59,10U);schedulerValue.update(requestValue());
    Command filler;factoryValue.create(action(),requestValue(),1U,0U,filler);
    while(!queueValue.isFull())queueValue.enqueue(filler);timeValue.value=snapshot(6,0,11U);
    schedulerValue.update(requestValue());timeValue.value=snapshot(0,1,12U);
    timeValue.value.date=dateValue(2026,7,15);timeValue.value.dayOfWeek=DayOfWeek::WEDNESDAY;
    queueValue.consume();schedulerValue.update(requestValue());const ScheduleRuntimeState* state=schedulerValue.findRuntimeState(1U,1U);
    TEST_ASSERT_EQUAL_UINT8(14U,state->lastExecutedDate.day);
    queueValue.consume();timeValue.value=snapshot(6,0,13U);timeValue.value.date=dateValue(2026,7,15);
    timeValue.value.dayOfWeek=DayOfWeek::WEDNESDAY;schedulerValue.update(requestValue());
    TEST_ASSERT_EQUAL_UINT32(2U,idValue.calls);

    resetFixture();command=fixedCommand();command.mode=ScheduleMode::INTERVAL;command.time.clear();
    command.daysMask=0U;command.intervalMs=10U;managerValue.add(scheduleWith(command));
    timeValue.value=snapshot(0,0,100U);schedulerValue.update(requestValue());
    while(!queueValue.isFull())queueValue.enqueue(filler);timeValue.value=snapshot(0,0,110U);
    schedulerValue.update(requestValue());timeValue.value=snapshot(0,0,150U);queueValue.consume();
    schedulerValue.update(requestValue());state=schedulerValue.findRuntimeState(1U,1U);
    TEST_ASSERT_EQUAL_UINT32(150U,state->lastExecutedMonotonicMs);
    queueValue.consume();timeValue.value=snapshot(0,0,159U);schedulerValue.update(requestValue());
    TEST_ASSERT_EQUAL_UINT32(1U,idValue.calls);timeValue.value=snapshot(0,0,160U);
    schedulerValue.update(requestValue());TEST_ASSERT_EQUAL_UINT32(2U,idValue.calls);
}

void testInvalidRequestContextResult()
{
    resetFixture();managerValue.add(scheduleWith(fixedCommand()));timeValue.value=snapshot(9,0);
    RequestContext request=requestValue();request.requestId=INVALID_REQUEST_ID;schedulerValue.update(request);
    TEST_ASSERT_TRUE(schedulerValue.getLastResult()==SchedulerResult::INVALID_REQUEST_CONTEXT);
    request=requestValue();request.source=CommandSource::APP;schedulerValue.update(request);
    TEST_ASSERT_TRUE(schedulerValue.getLastResult()==SchedulerResult::INVALID_REQUEST_CONTEXT);
    schedulerValue.update(requestValue());
    TEST_ASSERT_TRUE(schedulerValue.getLastResult()==SchedulerResult::NOT_DUE);
}

static_assert(sizeof(ScheduleRuntimeState)<=32U,"Runtime State بیش از هدف است");
static_assert(sizeof(Scheduler)<5120U,"Scheduler بیش از هدف است");
void setup(){UNITY_BEGIN();RUN_TEST(testTimeSnapshotAndProviders);RUN_TEST(testFixedTimeBaselineAndDailyCommit);
 RUN_TEST(testFixedDateIntervalAndWrap);RUN_TEST(testSolarManualAndInvalidOffset);
 RUN_TEST(testBackpressureAndPendingMutation);RUN_TEST(testDisabledRoundRobinCleanupAndRequest);
 RUN_TEST(testFixedTimeOccurrenceDateAcrossMidnight);RUN_TEST(testSolarOccurrenceDateAndIntervalSubmitTime);
 RUN_TEST(testInvalidRequestContextResult);UNITY_END();}
void loop(){}
