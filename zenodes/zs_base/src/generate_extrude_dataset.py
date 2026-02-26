import hou
import json
import os

OUTPUT_DIR = "extrude_dataset"
os.makedirs(OUTPUT_DIR, exist_ok=True)


# ---------------------------------------------------------
# Parameter lookup by UI label (recursive, folder-aware)
# ---------------------------------------------------------

def find_parm_by_label(node, label):
    """
    Recursively search parameter templates by UI label
    and return the corresponding hou.Parm object.
    """

    def search_templates(templates):
        for t in templates:
            # If the template is a folder, search inside it
            if isinstance(t, hou.FolderParmTemplate):
                result = search_templates(t.parmTemplates())
                if result:
                    return result
            else:
                if t.label() == label:
                    return node.parm(t.name())
        return None

    ptg = node.parmTemplateGroup()
    return search_templates(ptg.parmTemplates())


def safe_set_by_label(node, label, value):
    """
    Set a parameter using its UI label.
    Raises an error if the parameter cannot be found.
    """
    parm = find_parm_by_label(node, label)
    if parm is None:
        raise RuntimeError(
            f"Parameter with label '{label}' not found on node {node.type().name()}"
        )
    parm.set(value)


# ---------------------------------------------------------
# Geometry serialization
# ---------------------------------------------------------

def dump_geometry(geo):
    """
    Convert hou.Geometry into a serializable dictionary.
    Includes point positions and primitive topology.
    """

    data = {
        "points": [],
        "primitives": []
    }

    # Export points
    for p in geo.points():
        data["points"].append({
            "id": p.number(),
            "P": list(p.position())
        })

    # Export primitives
    for prim in geo.prims():
        data["primitives"].append({
            "id": prim.number(),
            "type": prim.type().name(),
            "vertices": [v.point().number() for v in prim.vertices()]
        })

    return data


# ---------------------------------------------------------
# Create input geometry
# ---------------------------------------------------------

def create_input_node(obj, input_spec):
    """
    Create a geometry container and a basic primitive node.

    input_spec: either a string ("grid", "box", "tube") or a dict:
        - "type": required, one of "grid", "box", "tube"
        - optional "params": dict of UI label -> value
        - any other top-level key (except "type"/"params") is also treated
          as UI label -> value for convenience
          Omitted params use Houdini defaults.
    Examples:
      {"type": "tube", "End Caps": 0, "Rows": 2, "Columns": 12}
      {"type": "tube", "params": {"End Caps": 0, "Rows": 2, "Columns": 12}}
    """

    if isinstance(input_spec, str):
        input_type = input_spec
        input_params = {}
    else:
        input_type = input_spec.get("type")
        if not input_type:
            raise ValueError('input_spec dict must have "type"')

        input_params = {}
        params_dict = input_spec.get("params")
        if isinstance(params_dict, dict):
            input_params.update(params_dict)

        # Also allow flat form: {"type":"tube", "Rows":2, ...}
        for k, v in input_spec.items():
            if k in ("type", "params"):
                continue
            input_params[k] = v

    geo_node = obj.createNode("geo", run_init_scripts=False)
    geo_node.moveToGoodPosition()

    if input_type == "grid":
        node = geo_node.createNode("grid")
    elif input_type == "box":
        node = geo_node.createNode("box")
    elif input_type == "tube":
        node = geo_node.createNode("tube")
    else:
        raise ValueError(f"Unknown input type: {input_type}")

    for label, value in input_params.items():
        parm = find_parm_by_label(node, label)
        if parm is not None:
            parm.set(value)
        # else: skip unknown label so we can add new params without breaking

    node.moveToGoodPosition()
    return geo_node, node


# ---------------------------------------------------------
# Execute a single test case
# ---------------------------------------------------------

def run_test(input_spec, params, test_id):
    """
    Build the network:
    input primitive -> PolyExtrude
    Then cook and export input/output geometry.

    input_spec: str ("grid"/"box"/"tube") or dict with "type" and optional param overrides.
    """
    input_type = input_spec if isinstance(input_spec, str) else input_spec["type"]
    obj = hou.node("/obj")

    geo_container, input_node = create_input_node(obj, input_spec)

    extrude = geo_container.createNode("polyextrude")
    extrude.setInput(0, input_node)

    # Set parameters using UI labels
    safe_set_by_label(extrude, "Distance", params.get("distance", 1.0))
    safe_set_by_label(extrude, "Inset", params.get("inset", 0.0))
    safe_set_by_label(extrude, "Divisions", params.get("divisions", 1))
    # Output Geometry and Groups (Houdini PolyExtrude: Output Front, Output Back, Output Side)
    for label, key in [
        ("Output Front", "output_front"),
        ("Output Back", "output_back"),
        ("Output Side", "output_side"),
    ]:
        if key in params:
            parm = find_parm_by_label(extrude, label)
            if parm is not None:
                parm.set(1 if params[key] else 0)

    extrude.moveToGoodPosition()
    extrude.setDisplayFlag(True)
    extrude.setRenderFlag(True)

    # Force cook
    extrude.cook(force=True)

    input_geo = input_node.geometry()
    output_geo = extrude.geometry()

    # Include input_spec in dataset so we know how input was built
    input_spec_serial = (
        input_spec
        if isinstance(input_spec, str)
        else {k: v for k, v in input_spec.items()}
    )
    dataset = {
        "input_type": input_type,
        "input_spec": input_spec_serial,
        "parameters": params,
        "input_geometry": dump_geometry(input_geo),
        "output_geometry": dump_geometry(output_geo)
    }

    filename = f"{input_type}_{test_id}.json"
    with open(os.path.join(OUTPUT_DIR, filename), "w") as f:
        json.dump(dataset, f, indent=2)

    # Clean up the created geometry container
    geo_container.destroy()


# ---------------------------------------------------------
# Main entry
# ---------------------------------------------------------

def main():
    """
    Define parameter variations and input primitives.
    Includes Output Front / Output Back / Output Side to match Houdini PolyExtrude.
    """

    # Base extrusion tests (all outputs on)
    tests = [
        {"distance": 1.0, "inset": 0.0},
        {"distance": 0.5, "inset": 0.0},
        {"distance": -0.5, "inset": 0.0},
        {"distance": 1.0, "inset": 0.2},
        {"distance": 0.0, "inset": 0.2},
    ]

    # Output geometry flags (Output Front, Output Back, Output Side)
    output_tests = [
        {"output_front": True, "output_back": True, "output_side": True},
        {"output_front": True, "output_back": False, "output_side": True},
        {"output_front": False, "output_back": True, "output_side": True},
        {"output_front": True, "output_back": True, "output_side": False},
        {"output_front": True, "output_back": False, "output_side": False},
        {"output_front": False, "output_back": True, "output_side": False},
        {"output_front": False, "output_back": False, "output_side": True},
    ]

    # input_types: str = type only (defaults), or dict = type + param overrides by UI label
    input_types = [
        "grid",
        "box",
        "tube",
        {"type": "tube", "Primitive Type": "poly", "End Caps": 0, "Rows": 2, "Columns": 12},
    ]

    # Explicit regression cases for reported mismatches.
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
    ]

    test_id = 0

    # Run base tests (all three outputs on)
    for input_spec in input_types:
        for params in tests:
            run_test(input_spec, params.copy(), test_id)
            test_id += 1

    # Run output-flag tests (distance=1, inset=0) to capture Front/Back/Side behavior
    for input_spec in input_types:
        for out_params in output_tests:
            params = {"distance": 1.0, "inset": 0.0, **out_params}
            run_test(input_spec, params, test_id)
            test_id += 1

    # Run focused mismatch-repro cases.
    for case in focused_cases:
        run_test(case["input_spec"], case["params"], test_id)
        test_id += 1


if __name__ == "__main__":
    main()