#include <zeno/zeno.h>
#include <TreeSketchMath.h>
#include <TreeSketchTreeObj.h>
#include <zeno/types/ListObject.h>

namespace zeno
{
    struct CreateTree
        : zeno::INode
    {
        virtual void apply() override
        {
            auto start_x = get_input2<float>("start_x");
            auto start_y = get_input2<float>("start_y");
            auto start_z = get_input2<float>("start_z");
            auto offset_degree_min = get_input2<float>("offset_degree_min");
            auto offset_degree_max = get_input2<float>("offset_degree_max");
            auto length_min = get_input2<float>("length_min");
            auto length_max = get_input2<float>("length_max");
            auto radius_min = get_input2<float>("radius_min");
            auto radius_max = get_input2<float>("radius_max");
            auto turn_points_num_min = get_input2<int>("turn_points_num_min");
            auto turn_points_num_max = get_input2<int>("turn_points_num_max");
            auto turn_points_offset_min = get_input2<float>("turn_points_offset_min");
            auto turn_points_offset_max = get_input2<float>("turn_points_offset_max");

            zeno::vec4d start{start_x, start_y, start_z, 1.0};
            auto offset_radian_min{degreeToRadian(offset_degree_min)};
            auto offset_radian_max{degreeToRadian(offset_degree_max)};

            auto treeObj = std::make_shared<TreeObj>(
                start,
                offset_radian_min, offset_radian_max,
                length_min, length_max,
                radius_min, radius_max,
                turn_points_num_min, turn_points_num_max,
                turn_points_offset_min, turn_points_offset_max);
            set_output("treeObj", std::move(treeObj));
        }
    }; // struct CreateTree

    ZENDEFNODE(
        CreateTree,
        {
            {
                {gParamType_Float, "start_x", "0.0"},
                {gParamType_Float, "start_y", "0.0"},
                {gParamType_Float, "start_z", "0.0"},
                {gParamType_Float, "offset_degree_min", "0.0"},
                {gParamType_Float, "offset_degree_max", "0.0"},
                {gParamType_Float, "length_min", "0.0"},
                {gParamType_Float, "length_max", "0.0"},
                {gParamType_Float, "radius_min", "0.0"},
                {gParamType_Float, "radius_max", "0.0"},
                {gParamType_Int, "turn_points_num_min", "0"},
                {gParamType_Int, "turn_points_num_max", "0"},
                {gParamType_Float, "turn_points_offset_min", "0.0"},
                {gParamType_Float, "turn_points_offset_max", "0.0"},
            },
            {
                {gParamType_Unknown, "treeObj"},
            },
            {},
            {
                "TreeSketch",
            },
        } // CreateTree
    );

    struct TreeCreateBranchs
        : zeno::INode
    {
        virtual void apply() override
        {
            auto treeObj = get_input<TreeObj>("treeObj");

            auto num_min = get_input2<int>("num_min");
            auto num_max = get_input2<int>("num_max");
            auto offset_start_min = get_input2<float>("offset_start_min");
            auto offset_start_max = get_input2<float>("offset_start_max");
            auto offset_degree_min = get_input2<float>("offset_degree_min");
            auto offset_degree_max = get_input2<float>("offset_degree_max");
            auto length_min = get_input2<float>("length_min");
            auto length_max = get_input2<float>("length_max");
            auto radius_min = get_input2<float>("radius_min");
            auto radius_max = get_input2<float>("radius_max");
            auto turn_points_num_min = get_input2<int>("turn_points_num_min");
            auto turn_points_num_max = get_input2<int>("turn_points_num_max");
            auto turn_points_offset_min = get_input2<float>("turn_points_offset_min");
            auto turn_points_offset_max = get_input2<float>("turn_points_offset_max");

            auto offset_radian_min{degreeToRadian(offset_degree_min)};
            auto offset_radian_max{degreeToRadian(offset_degree_max)};

            treeObj->create_branchs(
                num_min, num_max,
                offset_start_min, offset_start_max,
                offset_radian_min, offset_radian_max,
                length_min, length_max,
                radius_min, radius_max,
                turn_points_num_min, turn_points_num_max,
                turn_points_offset_min, turn_points_offset_max);
            set_output("treeObj", std::move(treeObj));
        }
    }; // struct TreeCreateBranchs

    ZENDEFNODE(
        TreeCreateBranchs,
        {
            {
                {gParamType_Unknown, "treeObj"},
                {gParamType_Int, "num_min", "0"},
                {gParamType_Int, "num_max", "0"},
                {gParamType_Float, "offset_start_min", "0.0"},
                {gParamType_Float, "offset_start_max", "0.0"},
                {gParamType_Float, "offset_degree_min", "0.0"},
                {gParamType_Float, "offset_degree_max", "0.0"},
                {gParamType_Float, "length_min", "0.0"},
                {gParamType_Float, "length_max", "0.0"},
                {gParamType_Float, "radius_min", "0.0"},
                {gParamType_Float, "radius_max", "0.0"},
                {gParamType_Int, "turn_points_num_min", "0"},
                {gParamType_Int, "turn_points_num_max", "0"},
                {gParamType_Float, "turn_points_offset_min", "0.0"},
                {gParamType_Float, "turn_points_offset_max", "0.0"},
            },
            {
                {gParamType_Unknown, "treeObj"},
            },
            {},
            {
                "TreeSketch",
            },
        } // TreeCreateBranchs
    );

    struct TreeSetLeaves
        : zeno::INode
    {
        virtual void apply() override
        {
            auto treeObj = get_input<TreeObj>("treeObj");
            treeObj->set_leaves();
            set_output("treeObj", std::move(treeObj));
        }
    }; // struct TreeSetLeaves

    ZENDEFNODE(
        TreeSetLeaves,
        {
            {
                {gParamType_Unknown, "treeObj"},
            },
            {
                {gParamType_Unknown, "treeObj"},
            },
            {},
            {
                "TreeSketch",
            },
        } // TreeSetLeaves
    );

    struct TreeToPrimitiveLines
        : zeno::INode
    {
        virtual void apply() override
        {
            auto treeObj = get_input<TreeObj>("treeObj");

            auto prim = std::make_shared<zeno::PrimitiveObject>();
            auto primList = std::make_shared<zeno::ListObject>();
            auto res = std::vector<std::shared_ptr<zeno::PrimitiveObject>>();
            treeObj->to_primitive_lines(prim.get(), res);
            for(auto p:res)
            {
                primList->push_back(p);
            }

            set_output("prim", std::move(primList));
        }
    }; // struct TreeToPrimitiveLines

    ZENDEFNODE(
        TreeToPrimitiveLines,
        {
            {
                {gParamType_Unknown, "treeObj"},
            },
            {
                {gParamType_Primitive, "prim"},
            },
            {},
            {
                "TreeSketch",
            },
        } // TreeToPrimitiveLines
    );

}; // namespace zeno