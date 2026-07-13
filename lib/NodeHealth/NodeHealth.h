#ifndef NODE_HEALTH_H
#define NODE_HEALTH_H

#include <limits.h>

#include <NodeCommon.h>

// حقیقت مستقل مشاهده Runtime؛ زمان‌ها فقط از Caller دریافت می‌شوند.
struct NodeHealth
{
    NodeId nodeId;
    NodeHealthState state;
    uint32_t lastSeenMs;
    uint32_t lastStateChangeMs;
    uint16_t consecutiveFailures;
    uint16_t latencyMs;

    NodeHealth() :
        nodeId(INVALID_NODE_ID),
        state(NodeHealthState::UNKNOWN),
        lastSeenMs(0),
        lastStateChangeMs(0),
        consecutiveFailures(0),
        latencyMs(0)
    {
    }

    bool isValid() const
    {
        return nodeId != INVALID_NODE_ID && isValidNodeHealthState(state);
    }

    bool isReachable() const
    {
        return state == NodeHealthState::ONLINE || state == NodeHealthState::DEGRADED;
    }

    bool isHealthy() const
    {
        return state == NodeHealthState::ONLINE;
    }

    bool setState(NodeHealthState newState, uint32_t timestampMs)
    {
        if (!isValidNodeHealthState(newState) || newState == state)
            return false;

        state = newState;
        lastStateChangeMs = timestampMs;
        return true;
    }

    void markSeen(uint32_t timestampMs, uint16_t measuredLatencyMs = 0)
    {
        lastSeenMs = timestampMs;
        latencyMs = measuredLatencyMs;
        consecutiveFailures = 0;
    }

    void recordFailure()
    {
        if (consecutiveFailures < UINT16_MAX)
            ++consecutiveFailures;
    }

    void clearFailures()
    {
        consecutiveFailures = 0;
    }

    void reset()
    {
        *this = NodeHealth{};
    }
};

#endif
