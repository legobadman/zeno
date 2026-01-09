#include <zeno/core/SolverImpl.h>
#include <zeno/extra/flipsolver.h>


namespace zeno
{
    SolverImpl::SolverImpl(INode* pNode) : NodeImpl(pNode) {

    }

    SolverImpl::~SolverImpl() {

    }

    void SolverImpl::terminate_solve() {
        if (get_nodecls() == "FlipSolver") {
#ifdef _WIN32
            FlipSolver* pNode = static_cast<FlipSolver*>(coreNode());
            pNode->terminate_solve();
#endif
        }
    }

    void SolverImpl::clearCalcResults() {
        NodeImpl::clearCalcResults();
        if (get_nodecls() == "FlipSolver") {
#ifdef _WIN32
            FlipSolver* pNode = static_cast<FlipSolver*>(coreNode());
            pNode->objs_cleaned();
#endif
        }
    }

    void SolverImpl::dirty_changed(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively) {
        if (get_nodecls() == "FlipSolver") {
#ifdef _WIN32
            FlipSolver* pNode = static_cast<FlipSolver*>(coreNode());
            pNode->on_dirty_changed(bOn, reason, bWholeSubnet, bRecursively);
#endif
        }
    }

    IObject* SolverImpl::get_default_output_object() {
        if (get_nodecls() == "FlipSolver") {
#ifdef _WIN32
            FlipSolver* pNode = static_cast<FlipSolver*>(coreNode());
            int frame = zeno::getSession().globalState->getFrameId();
            std::string cache_path;
            if (has_input("Cache Path")) {
                cache_path = get_input_prim<std::string>("Cache Path");
            }
            zany result = pNode->checkCache(cache_path, frame);
            if (result && result->key().empty()) {
                result->update_key(stdString2zs(get_uuid()));
            }
            return result.release();
#endif
        }
        return nullptr;
    }
}