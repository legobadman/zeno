import hou

from geometry_inputs import create_input_node, get_extrude_input_specs, normalize_input_spec
from testdata_common import (
    clear_output_json,
    dump_geometry,
    safe_set_by_label,
    try_set_by_label,
    write_dataset_json,
)


OUTPUT_DIR = "extrude_dataset"


def run_extrude_test(input_spec, params, test_id, output_dir=OUTPUT_DIR):
    """
    Build:
      input primitive -> PolyExtrude
    cook and export input/output geometry.
    """
    input_type, _ = normalize_input_spec(input_spec)
    obj = hou.node("/obj")

    geo_container, input_node = create_input_node(obj, input_spec)

    extrude = geo_container.createNode("polyextrude")
    extrude.setInput(0, input_node)

    safe_set_by_label(extrude, "Distance", params.get("distance", 1.0))
    safe_set_by_label(extrude, "Inset", params.get("inset", 0.0))
    safe_set_by_label(extrude, "Divisions", params.get("divisions", 1))

    for label, key in [
        ("Output Front", "output_front"),
        ("Output Back", "output_back"),
        ("Output Side", "output_side"),
    ]:
        if key in params:
            try_set_by_label(extrude, label, 1 if params[key] else 0)

    extrude.moveToGoodPosition()
    extrude.setDisplayFlag(True)
    extrude.setRenderFlag(True)
    extrude.cook(force=True)

    input_geo = input_node.geometry()
    output_geo = extrude.geometry()

    dataset = {
        "node_type": "polyextrude",
        "input_type": input_type,
        "input_spec": input_spec if isinstance(input_spec, str) else dict(input_spec),
        "parameters": params,
        "input_geometry": dump_geometry(input_geo),
        "output_geometry": dump_geometry(output_geo),
    }

    filename = "{}_{}.json".format(input_type, test_id)
    write_dataset_json(output_dir, filename, dataset)

    geo_container.destroy()


def main():
    clear_output_json(OUTPUT_DIR)

    extrude_tests = [
        {"distance": 1.0, "inset": 0.0, "output_front": True, "output_back": True, "output_side": True},
        {"distance": 0.5, "inset": 0.0, "output_front": True, "output_back": True, "output_side": True},
        {"distance": -0.5, "inset": 0.0, "output_front": True, "output_back": True, "output_side": True},
        {"distance": 1.0, "inset": 0.2, "output_front": True, "output_back": True, "output_side": True},
        {"distance": 0.0, "inset": 0.2, "output_front": True, "output_back": True, "output_side": True},
    ]

    output_toggle_tests = [
        {"output_front": True, "output_back": True, "output_side": True},
        {"output_front": True, "output_back": False, "output_side": True},
        {"output_front": False, "output_back": True, "output_side": True},
        {"output_front": True, "output_back": True, "output_side": False},
        {"output_front": True, "output_back": False, "output_side": False},
        {"output_front": False, "output_back": True, "output_side": False},
        {"output_front": False, "output_back": False, "output_side": True},
    ]

    focused_cases = [
        {
            "input_spec": {"type": "tube", "Primitive Type": "poly", "End Caps": 0, "Rows": 2, "Columns": 12},
            "params": {
                "distance": 0.144,
                "inset": 0.365,
                "output_front": True,
                "output_back": True,
                "output_side": True,
            },
        },
        {
            "input_spec": {"type": "tube", "Primitive Type": "poly", "End Caps": 1, "Rows": 2, "Columns": 12},
            "params": {
                "distance": 0.144,
                "inset": 0.0,
                "output_front": True,
                "output_back": True,
                "output_side": True,
            },
        },
    ]

    input_specs = get_extrude_input_specs()
    test_id = 0

    for input_spec in input_specs:
        for params in extrude_tests:
            run_extrude_test(input_spec, dict(params), test_id)
            test_id += 1

    for input_spec in input_specs:
        for out_params in output_toggle_tests:
            params = {"distance": 1.0, "inset": 0.0}
            params.update(out_params)
            run_extrude_test(input_spec, params, test_id)
            test_id += 1

    for case in focused_cases:
        run_extrude_test(case["input_spec"], case["params"], test_id)
        test_id += 1


if __name__ == "__main__":
    main()

