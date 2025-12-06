# ZENO node system

[![License](https://img.shields.io/badge/license-MPLv2-blue)](LICENSE)
[![Version](https://img.shields.io/github/v/release/zenustech/zeno)](https://github.com/zenustech/zeno/releases)


[Download](https://github.com/zenustech/zeno/releases) | [Videos](https://space.bilibili.com/263032155) | [Build from source](https://github.com/zenustech/zeno/blob/master/BUILD.md)

ZENO is an open-source, Node based 3D system able to produce cinematic physics effects at High Efficiency, it was designed for large scale simulations and has been tested on complex setups.
Aside of its simulation Tools, ZENO provides necessary visualization nodes for users to import and run simulations if you feel that the current software you are using is too slow.
Open-source node system framework, to change your algorithmic code into useful tools to create much more complicated simulations!

<img src="https://zenustech.oss-cn-beijing.aliyuncs.com/Place-in-Github/202312/ZENO2_v2023.jpg" width="640" position="left">


## Features
- [Graph Node Editor](docs/feature_grapheditor.md)
- [Geometry Manipatuion](docs/feature_calculation.md)
- [ZFX Script System](docs/feature_zfx.md)
- [Zeno Digital Asset](docs/feature_zda.md)
- [Software Extension](docs/feature_extension.md)

## Gallery

Fig.1 - Geometry


Fig.2 - Nvdia Optix Renderer


Fig.3 - Fluid simulation (用swe02.zsg)

<img src="https://zenustech.oss-cn-beijing.aliyuncs.com/Place-in-Github/202304/flip.png" width="640" position="left">
<img src="https://zenustech.oss-cn-beijing.aliyuncs.com/Place-in-Github/202304/liulang.gif" width="640" position="left">

Fig.4 - Rigid simulation

<img src="https://zenustech.oss-cn-beijing.aliyuncs.com/Place-in-Github/202208/Bullet_Simulation.gif" width="640" position="left">


Fig.5 - Procedural Terrain

<img src="https://zenustech.oss-cn-beijing.aliyuncs.com/Place-in-Github/202304/programmatic.gif" width="640" position="left">

Fig.6 - Human rendering

<img src="https://zenustech.oss-cn-beijing.aliyuncs.com/Place-in-Github/202304/face.png" width="640" position="left">


https://user-images.githubusercontent.com/25457920/234779878-a2f43b2f-5b9b-463b-950b-8842dad0c651.MP4



# End-user Installation

## Download binary release

Go to the [release page](https://github.com/zenustech/zeno/releases/), and click Assets -> download `zeno-windows-20xx.x.x.zip` (`zeno-linux-20xx.x.x.tar.gz` for Linux).

Then, extract this archive, and simply run `000_start.bat` (`./000_start.sh` for Linux), then the node editor window will shows up if everything is working well.

Apart from the GitHub release page, we also offer binary download from our official site for convinence of Chinese users: https://zenustech.com/d/

## How to play

There are some example graphs in the `misc/graphs/` folder, you may open them in the editor and have fun!
Hint: To run an animation for 100 frames, change the `1` on the bottom-right of the viewport to `100`, then click `Run`.
Also MMB to drag in the node editor, LMB click on sockets to create connections.
MMB drag in the viewport to orbit camera, Shift+MMB to pan camera.
More details are available in [our official tutorial](https://doc.zenustech.com/) and [my video tutorials](https://space.bilibili.com/263032155).

## Bug report

If you find the binary version didn't worked properly or some error message has been thrown on your machine, please let me know by opening an [issue](https://github.com/zenustech/zeno/issues) on GitHub, thanks for you support!


# Developer Build

To build ZENO from source, you need:

- GCC 9+ or MSVC 19.28+, and CMake 3.16+ to build ZENO.
- Qt 5.14+ to build the ZENO Qt editor.
- (Optional) TBB for parallel support.
- (Optional) OpenVDB for volume nodes.
- (Optional) Eigen3 for solver nodes.
- (Optional) CGAL for geometry nodes.
- (Optional) CUDA 11.6 for GPU nodes.

> Hint: WSL is not recommended because of its limited GUI and OpenGL support.

- [Click me for detailed build instructions](BUILD.md)


# Miscellaneous

## Contributors

Thank you to all the people who have already contributed to ZENO!

[![Contributors](https://contrib.rocks/image?repo=zenustech/zeno)](https://github.com/zenustech/zeno/graphs/contributors)

- [Contributor guidelines and helps](docs/CONTRIBUTING.md)

## Write your own extension!

See [`projects/FBX`](https://github.com/zenustech/zeno/projects/FBX) for an example on how to write custom nodes in ZENO.

## Legacy version of Zeno

Currently the [`master`](https://github.com/zenustech/tree/master) branch is for Zeno 2.0.
You may find Zeno 1.0 in the [`zeno_old_stable`](https://github.com/zenustech/tree/zeno_old_stable) branch.

## License

ZENO is licensed under the Mozilla Public License Version 2.0, see [LICENSE](LICENSE) for more information.

ZENO have also used many third-party libraries, some of which has little modifications. Their licenses could be found at [docs/licenses](docs/licenses).

## Contact us

You may contact us via WeChat:

* @zhxx1987: shinshinzhang

* @legobadman: zhihao-lu
