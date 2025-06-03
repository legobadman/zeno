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
class FlipSolver : public INode {
public:
    FlipSolver();
    ~FlipSolver();
    NodeType type() const override;

    void terminate_solve();
    void terminate_job();
    void objs_cleaned();
    void CloseIPCHandles();
    void initHandles();
    zany checkCache(std::string cache_path, int frame);
    std::string get_cachepath() const;
    void clear_cache() const;
    void on_dirty_changed(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively);
    void apply() override;
    CustomUI export_customui() const override;
    HANDLE get_pipe_read() const;
    HANDLE get_pipe_write() const;
    HANDLE get_sync_event() const;
    HANDLE get_readpipe_event() const;

private:
#if defined(_WIN32)
    HANDLE m_shm_initFluid = INVALID_HANDLE_VALUE;  //��ʼ���干���ڴ��ļ����
    HANDLE m_shm_staticCollider = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_solver_read = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_main_write = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_main_read = INVALID_HANDLE_VALUE;
    HANDLE m_hPipe_sovler_write = INVALID_HANDLE_VALUE;

    HANDLE m_hJob = INVALID_HANDLE_VALUE;
    HANDLE m_hSyncEvent = INVALID_HANDLE_VALUE;     //���������߳���ͨѶ�߳�ͬ����Ϣ
    HANDLE m_hReadPipeEvent = INVALID_HANDLE_VALUE; //�ܵ��첽IO�������¼�

    DWORD m_hIPCThread;     //ͨѶ�߳�
#else
    //TODO: linux pipe
#endif
};
}
