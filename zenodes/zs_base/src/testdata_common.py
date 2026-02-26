import json
import os


def ensure_output_dir(output_dir):
    os.makedirs(output_dir, exist_ok=True)


def clear_output_json(output_dir):
    ensure_output_dir(output_dir)
    for name in os.listdir(output_dir):
        if name.endswith(".json"):
            os.remove(os.path.join(output_dir, name))


def find_parm_by_label(node, label):
    """
    Recursively search parameter templates by UI label and return hou.Parm.
    """

    def search_templates(templates):
        for t in templates:
            if isinstance(t, __import__("hou").FolderParmTemplate):
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
    parm = find_parm_by_label(node, label)
    if parm is None:
        raise RuntimeError(
            "Parameter with label '{}' not found on node {}".format(
                label, node.type().name()
            )
        )
    parm.set(value)


def try_set_by_label(node, label, value):
    parm = find_parm_by_label(node, label)
    if parm is not None:
        parm.set(value)
        return True
    return False


def dump_geometry(geo):
    data = {"points": [], "primitives": []}

    for p in geo.points():
        data["points"].append({"id": p.number(), "P": list(p.position())})

    for prim in geo.prims():
        data["primitives"].append(
            {
                "id": prim.number(),
                "type": prim.type().name(),
                "vertices": [v.point().number() for v in prim.vertices()],
            }
        )

    return data


def write_dataset_json(output_dir, filename, dataset):
    ensure_output_dir(output_dir)
    path = os.path.join(output_dir, filename)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(dataset, f, indent=2)
    return path

