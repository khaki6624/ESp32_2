#ifndef COMMAND_RESULT_H
#define COMMAND_RESULT_H
#include <CommandCommon.h>
#include <DeviceCommon.h>

struct CommandResult
{
    CommandId commandId;
    RequestId requestId;
    ExecutionStatus status;
    CommandErrorCode errorCode;
    DeviceValue actualValue;
    uint32_t startedTimestampMs;
    uint32_t completedTimestampMs;
    char message[COMMAND_RESULT_MESSAGE_MAX_LENGTH];

    CommandResult() : commandId(INVALID_COMMAND_ID),requestId(INVALID_REQUEST_ID),
        status(ExecutionStatus::RECEIVED),errorCode(CommandErrorCode::NONE),actualValue{},
        startedTimestampMs(0),completedTimestampMs(0),message{} {}
    bool isValid() const
    { return commandId!=INVALID_COMMAND_ID&&requestId!=INVALID_REQUEST_ID&&
             isValidExecutionStatus(status)&&isValidCommandErrorCode(errorCode)&&
             CommandText::isCanonical(message,sizeof(message)); }
    bool isTerminal() const { return isTerminalExecutionStatus(status); }
    bool isSuccess() const { return status==ExecutionStatus::SUCCESS&&errorCode==CommandErrorCode::NONE; }
    bool setMessage(const char* value) { return CommandText::set(message,sizeof(message),value); }
    bool transitionTo(ExecutionStatus newStatus,uint32_t timestampMs,
                      CommandErrorCode newErrorCode=CommandErrorCode::NONE)
    {
        if(!isValid()||!isValidExecutionStatus(newStatus)||!isValidCommandErrorCode(newErrorCode)) return false;
        if(!isValidExecutionStatusTransition(status,newStatus)) return false;
        if(newStatus==ExecutionStatus::SUCCESS&&newErrorCode!=CommandErrorCode::NONE) return false;

        uint32_t newStartedTimestampMs=startedTimestampMs;
        uint32_t newCompletedTimestampMs=completedTimestampMs;
        if(newStatus==ExecutionStatus::EXECUTING) newStartedTimestampMs=timestampMs;
        if(isTerminalExecutionStatus(newStatus)) newCompletedTimestampMs=timestampMs;

        // Commit نهایی فقط پس از کامل شدن تمام Validationها انجام می‌شود.
        status=newStatus;
        errorCode=newErrorCode;
        startedTimestampMs=newStartedTimestampMs;
        completedTimestampMs=newCompletedTimestampMs;
        return true;
    }
};
#endif
