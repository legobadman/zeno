//#include <zeno/zeno.h>
#include <zeno/core/Session.h>
#include <zeno/core/NodeRegister.h>


int main_editor(int argc, char* argv[]);
int main_cmd(int argc, char* argv[]);

#ifdef ZENO_WITH_VLD
#include <vld.h>
struct VLDGuard {
    VLDGuard() {
        VLDGlobalDisable(); // 尽可能早地停止记录
    }
};
static VLDGuard _earlyVLDGuard; // 构造函数会在 main() 之前调用
#endif


int main(int argc, char *argv[]) 
{
    //VLDGlobalDisable();
    auto coreApp = zeno::createApplication();
    auto& sess = zeno::getSession();
    zeno::scope_exit sp([&]() {
        sess.destroy();
    });

    if (argc > 1) {
        return main_cmd(argc, argv);
    }
    else {
        return main_editor(argc, argv);
    }
}

int main_cmd(int argc, char* argv[]) {
    return 0;
}