#pragma once

#include <zeno/zeno.h>
#include <zeno/core/data.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/funcs/ObjectCodec.h>
#include <zeno/geo/geometryutil.h>
#include <zeno/utils/log.h>
#include <zeno/utils/uuid.h>
#include <zeno/extra/GlobalComm.h>
#if defined(_WIN32)
#include <Windows.h>
#else
#endif

namespace zeno
{
/*
 Solver是一个帧相关的节点（只要帧切换即标脏），对于外部图执行而言，Solver只是输出当前帧解算的内容或者空对象（如果没有），至于Solver
 如何得到这个解算的内容，外部并不关心（包括上下游，时间轴ui等）
 至于如何获取解算内容，Solver会起解算进程，往特定cache路径写内容。

 Solver的apply目前与其他执行的节点处于同一线程，额外有监听线程与解算进程通信，监听线程可通知ui
 */

class FlipSolver : public INode2 {
public:
    FlipSolver();
    ~FlipSolver();
    NodeType type() const override;
    ZErrorCode apply(INodeData* pNodeData) override;
    void clearCalcResults() {}
    void getIconResource(char* recv, size_t cap) {
        const char* icon = ":/icons/node/fluid.svg";
        strcpy(recv, icon);
        recv[strlen(icon)] = '\0';
    }
    void getBackgroundClr(char* recv, size_t cap) {
        const char* bg = "#246283";
        strcpy(recv, bg);
        recv[strlen(bg)] = '\0';
    }
    float time() const { return 1.0f; }

    void terminate_solve();
    void terminate_job();
    void objs_cleaned(NodeImpl* m_pAdapter);
    void CloseIPCHandles();
    void initHandles();
    zany2 checkCache(std::string cache_path, int frame);
    std::string get_cachepath(NodeImpl* m_pAdapter) const;
    void clear_cache(NodeImpl* m_pAdapter) const;
    void on_dirty_changed(NodeImpl* m_pAdapter, bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively);

    HANDLE get_pipe_read() const;
    HANDLE get_pipe_write() const;
    HANDLE get_sync_event() const;
    HANDLE get_readpipe_event() const;

private:
#if defined(_WIN32)
    HANDLE m_shm_initFluid = INVALID_HANDLE_VALUE;  //初始流体共享内存文件句柄
    HANDLE m_shm_staticCollider = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_solver_read = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_main_write = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_main_read = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_sovler_write = INVALID_HANDLE_VALUE;

    HANDLE m_hJob = INVALID_HANDLE_VALUE;
    HANDLE m_hSyncEvent = INVALID_HANDLE_VALUE;     //用于其他线程与通讯线程同步消息
    HANDLE m_hReadPipeEvent = INVALID_HANDLE_VALUE; //管道异步IO监听的事件

    DWORD m_hIPCThread;     //通讯线程
#else
    //TODO: linux pipe
#endif
};
}
