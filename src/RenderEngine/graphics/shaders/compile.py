import os
import re
import pathlib
import subprocess
from typing import Iterable

GLSLC = "glslc.exe"
PATH = pathlib.Path(__file__).parent
STAGES = {".vert": "VK_SHADER_STAGE_VERTEX_BIT",
          ".frag": "VK_SHADER_STAGE_FRAGMENT_BIT",
          ".comp": "VK_SHADER_STAGE_COMPUTE_BIT"}
DESCRIPTOR_TYPE = {
                 "UBO": "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER",
                 "SSBO": "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER",
                 "sampler": "VK_DESCRIPTOR_TYPE_SAMPLER",
                 "sampler2D": "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                 "subpassInput": "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT",
                 "image2D": "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE",
                }
IMAGE_FORMATS={"SRGB": "VK_FORMAT_R8G8B8A8_SRGB",
               "SNORM": "VK_FORMAT_R8G8B8A8_SNORM",
               "UNORM": "VK_FORMAT_R8G8B8A8_UNORM"}
LAYOUT_REGEX = re.compile(r"layout *\((?:(push_constant)|(?:input_attachment_index *= *(\d+))|(?:set *= *(\d+))|(?:binding *= *(\d+))|(?:offset *= *(\d+))|(?:location *= *(\d+))|(?:std\d+)|(?:, *))+\) +(in|out|uniform|buffer) +(?:(readonly|writeonly) )?(\w+)(?:\s*{[^}]+})? +(\w+)?(?:\[(\d*)\])?;")


SHADER_SRC = """#include <RenderEngine/graphics/shaders/{shader_name}.hpp>
#include <RenderEngine/graphics/ImageFormat.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
using namespace RenderEngine;


{shader_name}::{shader_name}(const GPU* gpu) : Shader(gpu,
    {vertex_buffers},
    {input_attachments},
    {output_attachments},
    {descriptor_sets},
    {push_constants},
    {shader_stages_bytecode})
{{
}}


{shader_name}::~{shader_name}()
{{
}}
"""


SHADER_HEADER = """#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{{
    class {shader_name} : public Shader
    {{
    public:
        {shader_name}() = delete;
        {shader_name}(const {shader_name}& other) = delete;
        {shader_name}(const GPU* gpu);
        ~{shader_name}();
    }};
}}
"""


def _get_shader_stages_code(shader_prefix: str) -> dict:
    """
    compile shader and load source and byte codes, for each subpass for each stage

    Returns
    -------
    dict :
        shader stages data of the form {".vert": {"source": "...", "bytes": b"..."}}
    """
    # list shadser source files corresponding to the given shader name
    files: list[pathlib.Path] = []
    for file in sorted(os.listdir(PATH)):
        if file.startswith(shader_prefix) and (PATH / file).suffix in STAGES.keys():
            files.append(PATH / file)
    # compile with glslc to spirv format and read source and byte codes
    code = dict()
    for source in files:
        spirv: pathlib.Path = source.with_suffix(source.suffix+".spv")
        if not spirv.is_file() or (spirv.lstat().st_mtime < source.lstat().st_mtime):
            sp = subprocess.run([GLSLC, source, "-o", spirv], capture_output=True)
            if sp.stderr:
                raise RuntimeError(f"{spirv} compilation failed:\n{sp.stderr.decode()}")
            print("Compiled:", spirv.relative_to(PATH))
        with open(source, "r", encoding="utf-8") as f:
            src = f.read()
        with open(spirv, "rb") as f:
            spv = f.read()
        code[source.suffix] = {"source": src, "bytes": spv}
    return code


def _get_variables(code: dict) -> list[dict]:
    """
    return a dict describing the variables
    """
    variables = {"push_constants": [],
                 "descriptor_sets": {},
                 "input_attachments": [],
                 "output_attachments": [],
                 "vertex_inputs": []}
    for extension, data in code.items():
        stage = STAGES[extension]
        src = data["source"]
        matches = LAYOUT_REGEX.findall(src)
        for (push_constant, input_index, _set, binding, offset, location, storage, memory_access, _type, name, count) in matches:
            if push_constant != "":
                variables["push_constants"].append({"type": _type, "stage": stage, "offset": offset or "0", "name": name})
            elif binding != "" and storage in ("uniform", "buffer"):
                descriptor_set = variables["descriptor_sets"].setdefault(int(_set or "0"), [])
                descriptor = {"set": _set or "0", "binding": binding, "name": name or _type, "count": count or "1", "stage": stage,
                              "type": DESCRIPTOR_TYPE.get(_type, DESCRIPTOR_TYPE["UBO"]) if storage == "uniform" else DESCRIPTOR_TYPE["SSBO"]}
                descriptor_set.append(descriptor)
                if _type == "subpassInput":
                    assert stage == "VK_SHADER_STAGE_FRAGMENT_BIT"
                    variables["input_attachments"].append({"name": name, "input_index": input_index})
            if stage == "VK_SHADER_STAGE_FRAGMENT_BIT" and storage == "out":
                variables["output_attachments"].append({"name": name, "type": _type})
            if stage == "VK_SHADER_STAGE_VERTEX_BIT" and storage == "in":
                variables["vertex_inputs"].append({"name": name, "location": location, "binding": binding or "0", "type": _type, "offset": offset})
    return variables


def _nested(nested: Iterable) -> str:
    """
    recursively creates a string representation of the nested iterables

    Example
    -------
    >>> _nested([('A', 'B'), [3, 4, 5, 6]])
    "{{\"A\", \"B\"}, {3, 4, 5, 6}}"
    """
    if isinstance(nested, str) and not nested.isdigit():
        return f"\"{nested}\""
    elif isinstance(nested, bytes):
        return nested.decode()
    elif isinstance(nested, str) or not hasattr(nested, "__iter__"):
        return f"{nested}"
    else:
        return "{" + ", ".join(_nested(x) for x in nested) + "}"


def _image_format_spliter(name: str) -> tuple[str, str]:
    """
    Split the image format from the name of the image and return a (name, format) tuple

    Example
    -------
    >>> _image_format_spliter("color_srgb")
    ("color", b"VK_FORMAT_R8G8B8A8_SRGB")
    >>> _image_format_spliter("new_material_UNORM")
    ("new_material", b"VK_FORMAT_R8G8B8A8_UNORM")
    >>> # returns a SRGB image type by default
    >>> _image_format_spliter("random_image")
    ("random_image", b"VK_FORMAT_R8G8B8A8_SRGB")
    """
    suffix = name.split("_")[-1].upper()
    if suffix in IMAGE_FORMATS.keys():
        prefix = "_".join(name.split("_")[:-1])
        return (prefix, IMAGE_FORMATS[suffix].encode())
    else:
        return (name, IMAGE_FORMATS["SRGB"].encode())


def save_shader(shader_prefix: str, code: dict, variables: dict):
    """
    generate the C++ code for a renderpass shader
    """
    input_attachments = _nested((_image_format_spliter(att["name"]) for att in variables["input_attachments"]))
    output_attachments = _nested((_image_format_spliter(att["name"]) for att in variables["output_attachments"]))
    # for each stage, for each set, the list of descriptor sets
    descriptors = _nested([(desc["name"], (desc["binding"], desc["type"].encode(), desc["count"], desc["stage"].encode())) for desc in descriptor_set]
                           for i, descriptor_set in variables["descriptor_sets"].items())
    # For each subpass the vertex attributes
    vertex_buffers = _nested((vi["name"].split("_")[-1],
                             (vi["location"], vi["binding"], f"static_cast<VkFormat>(Type::{vi['type'].upper()})".encode(), f"offsetof({vi['name'].split('_')[0].title()}, {vi['name'].split('_')[-1]})".encode()))
                             for vi in variables["vertex_inputs"])
    # for each subpass the push constants
    push_constants = _nested((pc["name"], (pc["stage"].encode(), int(pc["offset"]), f"sizeof({pc['type']})".encode())) for pc in variables["push_constants"])
    # shader stage bytecodes
    shader_stages_bytecode = _nested([(STAGES[k].encode(), list(v["bytes"])) for k, v in code.items()])
    # writing c++ source file
    src = SHADER_SRC.format(shader_name=shader_prefix,
                            vertex_buffers=vertex_buffers,
                            input_attachments=input_attachments,
                            output_attachments=output_attachments,
                            push_constants=push_constants,
                            descriptor_sets=descriptors,
                            shader_stages_bytecode=shader_stages_bytecode)
    src_path = (PATH / shader_prefix).with_suffix(".cpp")
    with open(src_path, "w", encoding="utf-8") as f:
        f.write(src)
    # writing c++ header file
    header = SHADER_HEADER.format(shader_name=shader_prefix)
    header_path = pathlib.Path(src_path.as_posix().replace("/src/", "/include/")).with_suffix(".hpp")
    if not header_path.is_file() or True:
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(header)

def all_constructors():
    """
    generate
    """
    global PATH
    files = set(pathlib.Path(file).stem for file in os.listdir(PATH)
                if pathlib.Path(file).suffix.lower() in STAGES.keys())
    for shader_prefix in files:
        code = _get_shader_stages_code(shader_prefix)
        variables = _get_variables(code)
        save_shader(shader_prefix, code, variables)


if __name__ == "__main__":
    all_constructors()
