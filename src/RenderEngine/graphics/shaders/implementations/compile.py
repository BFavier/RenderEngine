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
UNIFORM_TYPES = {
                 "sampler": "VK_DESCRIPTOR_TYPE_SAMPLER",
                 "sampler2D": "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",
                 "subpassInput": "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT",
                 "image2D": "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE",
                }
FRAMEBUFFER_FORMATS = ["ERROR", "ImageFormat::GRAY", "ERROR", "ImageFormat::RGB", "ImageFormat::RGBA"]
BUFFER_TYPE = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER"
LAYOUT_REGEX = re.compile(r"layout *\((?:(push_constant)|(?:input_attachment_index *= *(\d+))|(?:set *= *(\d+))|(?:binding *= *(\d+))|(?:offset *= *(\d+))|(?:location *= *(\d+))|(?:std\d+)|(?:, *))+\) +(in|out|uniform|buffer) +(?:(readonly|writeonly) )?(\w+)(?:\s*{[^}]+})? +(\w+)?(?:\[(\d*)\])?;")


SHADER_SRC = """#include <RenderEngine/graphics/shaders/implementations/{shader_name}.hpp>
#include <RenderEngine/graphics/ImageFormat.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
using namespace RenderEngine;


{shader_name}::{shader_name}(const GPU* gpu) : Shader(gpu,
    {vertex_buffer_bindings}, 
    {vertex_buffer_attributes},
    {attachments},
    {input_attachments},
    {output_attachments},
    {descriptor_sets},
    {push_constants},
    {stages_bytecode})
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


COMPUTE_SHADER_SRC = """#include <RenderEngine/graphics/shaders/implementations/{shader_name}.hpp>
#include <RenderEngine/graphics/ImageFormat.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
using namespace RenderEngine;


{shader_name}::{shader_name}(const GPU* gpu) : ComputeShader(gpu,
    {descriptor_sets}, 
    {push_constants},
    {bytecode})
{{
}}


{shader_name}::~{shader_name}()
{{
}}
"""


COMPUTE_SHADER_HEADER = """#pragma once
#include <RenderEngine/graphics/shaders/ComputeShader.hpp>

namespace RenderEngine
{{
    class {shader_name} : public ComputeShader
    {{
    public:
        {shader_name}() = delete;
        {shader_name}(const {shader_name}& other) = delete;
        {shader_name}(const GPU* gpu);
        ~{shader_name}();
    }};
}}
"""


def _get_subpasses_code(shader_path: pathlib.Path) -> dict:
    """
    compile shader and load source and byte codes, for each subpass for each stage

    Returns
    -------
    dict :
        subpass data of the form {"subpass0": {".vert": {"source": "...", "bytes": b"..."}}}
    """
    # list files in the folder
    files: list[pathlib.Path] = []
    for file in sorted(os.listdir(shader_path)):
        file_path = shader_path / file
        if file_path.is_file():
            files.append(file_path)
    # list subpasses and stages
    subpasses = {file.name[:-len("".join(file.suffixes))]: {} for file in files if file.suffix in STAGES.keys()}
    for subpass in subpasses.keys():
        for file in files:
            if file.name.startswith(subpass) and file.suffix in STAGES.keys():
                subpasses[subpass][file.suffix] = {}
    # compile with glslc to spirv format and read source and byte codes
    for subpass, stages in subpasses.items():
        for stage, code in stages.items():
            source: pathlib.Path = shader_path / (subpass+stage)
            spirv: pathlib.Path = shader_path / (subpass+stage+".spv")
            if not spirv.is_file() or (spirv.lstat().st_mtime < source.lstat().st_mtime):
                sp = subprocess.run([GLSLC, source, "-o", spirv], capture_output=True)
                if sp.stderr:
                    raise RuntimeError(f"{spirv} compilation failed:\n{sp.stderr.decode()}")
                print("Compiled:", spirv.relative_to(PATH))
            with open(source, "r", encoding="utf-8") as f:
                src = f.read()
            with open(spirv, "rb") as f:
                spv = f.read()
            code["source"] = src
            code["bytes"] = spv
    return subpasses


def _get_subpasses_layout(subpasses: dict) -> list[dict]:
    """
    return for each subpass a dict describing the layout
    """
    subpasses_layouts = []
    for subpass, stages_code in subpasses.items():
        descriptors = []
        vertex_inputs = []
        input_attachments = []
        output_attachments = []
        push_constants = []
        for extension, stage in STAGES.items():
            src = stages_code.get(extension, {"source": ""})["source"]
            matches = LAYOUT_REGEX.findall(src)
            for (push_constant, input_index, _set, binding, offset, location, storage, memory_access, _type, name, count) in matches:
                if push_constant != "":
                    push_constants.append({"type": _type, "stage": stage, "offset": offset or "0", "name": name})
                elif binding != "" and storage in ("uniform", "buffer"):
                    default_type = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER" if storage == "uniform" else "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER"
                    descriptors.append({"set": _set or "0", "binding": binding, "name": name or _type, "count": count or "1", "stage": stage,
                                        "type": UNIFORM_TYPES.get(_type, default_type) if storage == "uniform" else BUFFER_TYPE})
                    if _type == "subpassInput":
                        assert stage == "VK_SHADER_STAGE_FRAGMENT_BIT"
                        input_attachments.append({"name": name, "input_index": input_index})
                if stage == "VK_SHADER_STAGE_FRAGMENT_BIT" and storage == "out":
                    output_attachments.append({"name": name, "type": _type})
                if stage == "VK_SHADER_STAGE_VERTEX_BIT" and storage == "in":
                    vertex_inputs.append({"name": name, "location": location, "binding": binding or "0", "type": _type, "offset": offset})
        subpasses_layouts.append({"descriptors": descriptors, "vertex_inputs": vertex_inputs, "input_attachments": input_attachments, "output_attachments": output_attachments, "push_constants": push_constants})
    return subpasses_layouts


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
    

def save_shader(shader_path: pathlib.Path, subpasses: list, layouts: list):
    """
    generate the C++ code for a renderpass shader
    """
    # order unique attachments :
    # input attachment are identified by a name and an index, so we have to make sure they at at the correct index in the list of unique attachments
    all_input_attachments = {att["name"]: int(att["input_index"]) for subpass in layouts for att in subpass["input_attachments"]}
    att_types = {att["name"]: FRAMEBUFFER_FORMATS[int(att["type"][-1])].encode() for subpass in layouts for att in subpass["output_attachments"]}
    att_indexes = {name: min((i, j) for i, subpass in enumerate(layouts) for j, att in enumerate(subpass["output_attachments"]) if att["name"] == name)
                   for name in att_types.keys() if name not in all_input_attachments.keys()}  # non-input attachments by first (subpass, attachment) apparition indexes
    attachments = [name for name, index in sorted(att_indexes.items(), key=lambda x: x[1])]
    for name, index in sorted(all_input_attachments.items(), key=lambda x: x[1]):
        attachments.insert(index, name)
    for att in attachments:
        if att not in att_indexes.keys():
            raise RuntimeError(f"'{att}' is referenced as a subpass input attachment but not as an output attachment in the shader.")
    attachments = _nested((name, att_types[name]) for name in attachments)
    # list inputs attachments and output attachments at each stage
    input_attachments = _nested((att["name"] for att in subpass["input_attachments"]) for subpass in layouts)
    output_attachments = _nested((att["name"] for att in subpass["output_attachments"]) for subpass in layouts)
    # for each stage, for each set, the list of descriptor sets
    descriptors = _nested(({descriptor["set"]: [(desc["name"], (desc["binding"], desc["type"].encode(), desc["count"], desc["stage"].encode()))
                                                for desc in subpass["descriptors"] if descriptor["set"] == desc["set"]]}.values()
                           for descriptor in subpass["descriptors"]) for subpass in layouts)
    # For each subpass the vertex attributes
    bindings = [{vi["binding"]: vi["name"].split("_")[0] for vi in subpass["vertex_inputs"]} for subpass in layouts]
    vertex_buffer_bindings = _nested([(name, (binding, f"sizeof({name.title()})".encode(), "VK_VERTEX_INPUT_RATE_VERTEX".encode() if name == "vertex" else "VK_VERTEX_INPUT_RATE_INSTANCE".encode()))
                                      for binding, name in sorted(subpass.items())]
                                     for subpass in bindings)
    vertex_buffer_attributes = _nested([(vi["name"].split("_")[-1], (vi["location"], vi["binding"], f"static_cast<VkFormat>(Type::{vi['type'].upper()})".encode(), f"offsetof({vi['name'].split('_')[0].title()}, {vi['name'].split('_')[-1]})".encode()))
                                        for vi in subpass["vertex_inputs"]]
                                       for subpass in layouts)
    # for each subpass the push constants
    push_constants = _nested([(pc["type"], (pc["stage"].encode(), int(pc["offset"]), f"sizeof({pc['type']})".encode())) for pc in subpass["push_constants"]] for subpass in layouts)
    # shader stage bytecodes
    stages_bytecode = _nested([(STAGES[k].encode(), list(v["bytes"])) for k, v in subpass.items()] for subpass_name, subpass in subpasses.items())
    # assembling and writing
    code = SHADER_SRC.format(shader_name=shader_path.name,
                                  vertex_buffer_bindings=vertex_buffer_bindings,
                                  vertex_buffer_attributes=vertex_buffer_attributes,
                                  attachments=attachments,
                                  input_attachments=input_attachments,
                                  output_attachments=output_attachments,
                                  push_constants=push_constants,
                                  descriptor_sets=descriptors,
                                  stages_bytecode=stages_bytecode)
    with open(shader_path.with_suffix(".cpp"), "w", encoding="utf-8") as f:
        f.write(code)
    header_path = pathlib.Path(shader_path.as_posix().replace("/src/", "/include/")).with_suffix(".hpp")
    header = SHADER_HEADER.format(shader_name=shader_path.name)
    if not header_path.is_file() or True:
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(header)


def save_compute_shader(shader_path: pathlib.Path, code: dict, layout: dict):
    """
    generates the c++ code for a compute shader
    """
    descriptor_sets = sorted({desc["set"] for desc in layout["descriptors"]})
    descriptors = _nested([[(desc["name"], (desc["binding"], desc["type"].encode(), desc["count"], desc["stage"].encode())) for desc in layout["descriptors"] if desc["set"] == dset] for dset in descriptor_sets])
    push_constants = _nested((pc["name"], (pc["stage"].encode(), pc["offset"], f"sizeof({pc['type']})".encode())) for pc in layout["push_constants"])
    bytecode = _nested([b for b in list(code["bytes"])])
    code = COMPUTE_SHADER_SRC.format(shader_name=shader_path.name,
                                     descriptor_sets=descriptors,
                                     push_constants=push_constants,
                                     bytecode=bytecode)
    with open(shader_path.with_suffix(".cpp"), "w", encoding="utf-8") as f:
        f.write(code)
    header_path = pathlib.Path(shader_path.as_posix().replace("/src/", "/include/")).with_suffix(".hpp")
    header = COMPUTE_SHADER_HEADER.format(shader_name=shader_path.name)
    if not header_path.is_file() or True:
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(header)

def constructor(shader_path: pathlib.Path):
    """
    generate C++ constructor source code for the given shader
    """
    subpasses = _get_subpasses_code(shader_path)
    layouts = _get_subpasses_layout(subpasses)
    if len(subpasses) == 1 and ".comp" in next(iter(subpasses.values())).keys():
        save_compute_shader(shader_path, next(iter(subpasses.values()))[".comp"], layouts[0])
    else:
        save_shader(shader_path, subpasses, layouts)


def all_constructors():
    """
    generate
    """
    global PATH
    for dir in os.listdir(PATH):
        path = PATH / dir
        if path.is_dir():
            constructor(path)


if __name__ == "__main__":
    all_constructors()