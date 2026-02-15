#include <zeno/extra/GlobalState.h>
#include <zeno/extra/GlobalComm.h>
#include <zeno/utils/logger.h>
#include <reflect/core.hpp>
#include <reflect/type.hpp>
#include <reflect/container/any>
#include <reflect/container/any>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

ZENO_API GlobalState state;

ZENO_API GlobalState::GlobalState() {}
ZENO_API GlobalState::~GlobalState() = default;

ZENO_API bool GlobalState::substepBegin() {
    if (has_substep_executed) {
        if (!time_step_integrated)
            return false;
    }
    if (has_frame_completed)
        return false;
    return true;
}

ZENO_API void GlobalState::substepEnd() {
    substepid++;
    has_substep_executed = true;
}

ZENO_API void GlobalState::frameBegin() {
    has_frame_completed = false;
    has_substep_executed = false;
    time_step_integrated = false;
    frame_time_elapsed = 0;
}

ZENO_API void GlobalState::frameEnd() {
    frameid++;
}

ZENO_API void GlobalState::clearState() {
    m_working = false;
    substepid = 0;
    frame_time = 1.f / 60.f;
    frame_time_elapsed = 0;
    has_frame_completed = false;
    has_substep_executed = false;
    time_step_integrated = false;
    total_time = 0.f;
    time_consumed = 0.f;
    processed_io_units = 0.f;
    sessionid++;
    log_debug("entering session id={}", sessionid);
}

ZENO_API int GlobalState::getFrameId() const {
    return frameid;
}

ZENO_API void GlobalState::updateFrameId(float frame) {
    //todo: mutex
    frameid = frame;
}

ZENO_API void GlobalState::updateFrameRange(int start, int end)
{
    frame_start = start;
    frame_end = end;
}

ZENO_API int GlobalState::getStartFrame() const
{
    return frame_start;
}

ZENO_API int GlobalState::getEndFrame() const
{
    return frame_end;
}

ZENO_API bool GlobalState::is_working() const {
    std::lock_guard lk(mtx);
    return m_working;
}

void GlobalState::init_total_runtime(float t) {
    total_time = t;
}

void GlobalState::update_consume_time(float t) {
    std::lock_guard lk(mtx);
    time_consumed += t;
}

float GlobalState::get_total_runtime() const {
    std::lock_guard lk(mtx);
    return total_time;
}

float GlobalState::get_consume_time() const {
    std::lock_guard lk(mtx);
    return time_consumed;
}


void GlobalState::inc_io_processed(int inc) {
    processed_io_units += inc;
}

int GlobalState::get_io_processed() const {
    return processed_io_units;
}

std::string GlobalState::get_report_error() const {
    return m_reported_error;
}

void GlobalState::report_error(const std::string& error) {
    //TODO: 考虑多线程情况下的race condition
    std::lock_guard lck(mtx);
    m_reported_error = error;
}

ZENO_API void GlobalState::set_working(bool working) {
    std::lock_guard lk(mtx);
    m_working = working;
}

ZENO_API void GlobalState::setCalcObjStatus(CalcObjStatus status) {
    m_status = status;
}

}
