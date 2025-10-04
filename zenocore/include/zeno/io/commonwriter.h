#ifndef __COMMON_WRITER_H__
#define __COMMON_WRITER_H__

#include <zeno/core/data.h>
#include <zeno/io/iocommon.h>

namespace zenoio
{
    class CommonWriter
    {
    public:
        CommonWriter();
        ZENO_API void set_proj_path(const std::string& path);

    protected:
        void dumpGraph(zeno::GraphData graph, RAPIDJSON_WRITER& writer);
        void dumpNode(const zeno::NodeData& data, RAPIDJSON_WRITER& writer);
        void dumpObjectParam(zeno::ParamObject param, RAPIDJSON_WRITER& writer);
        void dumpPrimitiveParam(zeno::ParamPrimitive info, RAPIDJSON_WRITER& writer);
        void dumpTimeline(zeno::TimelineInfo info, RAPIDJSON_WRITER& writer);
        void dumpCustomUI(zeno::CustomUI customUi, RAPIDJSON_WRITER& writer);

    private:
        std::string m_proj_path;
    };
}

#endif