from testdata_common import try_set_by_label


def normalize_input_spec(input_spec):
    """
    input_spec:
      - str: "grid"/"box"/"tube"
      - dict:
          {"type":"tube", "Rows":2}
          {"type":"tube", "params":{"Rows":2}}
    """
    if isinstance(input_spec, str):
        return input_spec, {}

    input_type = input_spec.get("type")
    if not input_type:
        raise ValueError('input_spec dict must have "type"')

    input_params = {}
    params_dict = input_spec.get("params")
    if isinstance(params_dict, dict):
        input_params.update(params_dict)

    for k, v in input_spec.items():
        if k in ("type", "params"):
            continue
        input_params[k] = v

    return input_type, input_params


def create_input_node(obj, input_spec):
    """
    Create geometry container + primitive source node from input_spec.
    """
    input_type, input_params = normalize_input_spec(input_spec)

    geo_node = obj.createNode("geo", run_init_scripts=False)
    geo_node.moveToGoodPosition()

    if input_type == "grid":
        node = geo_node.createNode("grid")
    elif input_type == "box":
        node = geo_node.createNode("box")
    elif input_type == "tube":
        node = geo_node.createNode("tube")
    elif input_type == "line":
        node = geo_node.createNode("line")
    elif input_type == "circle":
        node = geo_node.createNode("circle")
    else:
        raise ValueError("Unknown input type: {}".format(input_type))

    for label, value in input_params.items():
        try_set_by_label(node, label, value)

    node.moveToGoodPosition()
    return geo_node, node


def get_extrude_input_specs():
    """
    Centralized input-geometry presets for Extrude tests.
    """
    return [
        # Grid
        {"type": "grid", "Primitive Type": "poly"},
        {"type": "grid", "Primitive Type": "poly", "Rows": 2, "Columns": 2, "Size": (2.0, 2.0)},
        {"type": "grid", "Primitive Type": "poly", "Rows": 6, "Columns": 6, "Center": (1.0, 0.5, -2.0), "Rotate": (15.0, 35.0, 10.0)},
        # Box
        {"type": "box", "Primitive Type": "poly"},
        {"type": "box", "Primitive Type": "poly", "Size": (2.0, 1.0, 0.5)},
        {"type": "box", "Primitive Type": "poly", "Use Divisions": 1, "Divisions": (2, 3, 4)},
        # Tube
        {"type": "tube", "Primitive Type": "poly"},
        {"type": "tube", "Primitive Type": "poly", "End Caps": 0, "Rows": 2, "Columns": 12},
        {"type": "tube", "Primitive Type": "poly", "End Caps": 1, "Rows": 2, "Columns": 12},
        {"type": "tube", "Primitive Type": "poly", "End Caps": 1, "Rows": 4, "Columns": 10, "Radius": (0.6, 1.4), "Radius Scale": 0.85},
    ]

