#ifndef SECURITY_COMMON_H
#define SECURITY_COMMON_H
#include <CommandCommon.h>

enum class SystemModeCheckResult : uint8_t { ALLOWED=0, INVALID_MODE, SYSTEM_LOCKED, MAINTENANCE_REJECTED, SYSTEM_UPDATING, EMERGENCY_LOCK_ACTIVE, POLICY_ERROR };
inline bool isValidSystemModeCheckResult(SystemModeCheckResult v){switch(v){case SystemModeCheckResult::ALLOWED:case SystemModeCheckResult::INVALID_MODE:case SystemModeCheckResult::SYSTEM_LOCKED:case SystemModeCheckResult::MAINTENANCE_REJECTED:case SystemModeCheckResult::SYSTEM_UPDATING:case SystemModeCheckResult::EMERGENCY_LOCK_ACTIVE:case SystemModeCheckResult::POLICY_ERROR:return true;default:return false;}}
enum class AuthorizationCheckResult : uint8_t { ALLOWED=0, INVALID_REQUEST, USER_NOT_FOUND, USER_DISABLED, UNAUTHORIZED, PERMISSION_DENIED, POLICY_ERROR };
inline bool isValidAuthorizationCheckResult(AuthorizationCheckResult v){switch(v){case AuthorizationCheckResult::ALLOWED:case AuthorizationCheckResult::INVALID_REQUEST:case AuthorizationCheckResult::USER_NOT_FOUND:case AuthorizationCheckResult::USER_DISABLED:case AuthorizationCheckResult::UNAUTHORIZED:case AuthorizationCheckResult::PERMISSION_DENIED:case AuthorizationCheckResult::POLICY_ERROR:return true;default:return false;}}
enum class SafetyCheckResult : uint8_t { ALLOWED=0, INVALID_COMMAND, INTERLOCK_ACTIVE, DEVICE_UNSAFE, NODE_UNSAFE, TIME_RESTRICTED, CRITICAL_FAULT, POLICY_ERROR };
inline bool isValidSafetyCheckResult(SafetyCheckResult v){switch(v){case SafetyCheckResult::ALLOWED:case SafetyCheckResult::INVALID_COMMAND:case SafetyCheckResult::INTERLOCK_ACTIVE:case SafetyCheckResult::DEVICE_UNSAFE:case SafetyCheckResult::NODE_UNSAFE:case SafetyCheckResult::TIME_RESTRICTED:case SafetyCheckResult::CRITICAL_FAULT:case SafetyCheckResult::POLICY_ERROR:return true;default:return false;}}
enum class ConfirmTokenCheckResult : uint8_t { NOT_REQUIRED=0, VALID, REQUIRED, INVALID_TOKEN, EXPIRED_TOKEN, TOKEN_COMMAND_MISMATCH, TOKEN_REQUEST_MISMATCH, TOKEN_ALREADY_USED, POLICY_ERROR };
inline bool isValidConfirmTokenCheckResult(ConfirmTokenCheckResult v){switch(v){case ConfirmTokenCheckResult::NOT_REQUIRED:case ConfirmTokenCheckResult::VALID:case ConfirmTokenCheckResult::REQUIRED:case ConfirmTokenCheckResult::INVALID_TOKEN:case ConfirmTokenCheckResult::EXPIRED_TOKEN:case ConfirmTokenCheckResult::TOKEN_COMMAND_MISMATCH:case ConfirmTokenCheckResult::TOKEN_REQUEST_MISMATCH:case ConfirmTokenCheckResult::TOKEN_ALREADY_USED:case ConfirmTokenCheckResult::POLICY_ERROR:return true;default:return false;}}
#endif
