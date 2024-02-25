import os
import re
import pathlib
import subprocess

GLSLC = "glslc.exe"
PATH = pathlib.Path(__file__).parent
STAGES = {".vert": "VK_SHADER_STAGE_VERTEX_BIT",
          ".frag": "VK_SHADER_STAGE_FRAGMENT_BIT",
          ".comp": "VK_SHADER_STAGE_COMPUTE_BIT"}
TYPES = {"sampler": "VK_DESCRIPTOR_TYPE_SAMPLER",
         "sampler2D": "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER"}
# regex for bindings "layout([set={0}, ]binding={0}[, offset={0}]) uniform {vec3|image2D|sampler2D|...} {name};"
BINDING_REGEX = re.compile(r"layout *\((?:set *= *(\d+), *)?binding *= *(\d+)(?:, *offset *= *(\d+))?(?:, *std\d+)?\) *uniform *(\w+)?(?:\s*{[^}]+})? *(\w+)(?:\[(\d+)\])?;")


CONSTRUCTOR_SRC = """#include <RenderEngine/graphics/shaders/{shader_name}.hpp>
#include <RenderEngine/graphics/GPU.hpp>
using namespace RenderEngine;

std::vector<std::vector<std::vector<std::pair<std::string, VkDescriptorSetLayoutBinding>>>> bindings_sets = {bindings_sets};
std::vector<std::vector<std::pair<VkVertexInputBindingDescription, VkVertexInputAttributeDescription>>> vertex_inputs = {vertex_inputs};
std::vector<std::vector<std::pair<std::string, Shader::Type>>> fragment_inputs = {fragment_inputs};
std::vector<std::vector<std::pair<std::string, Shader::Type>>> fragment_outputs = {fragment_outputs};
std::vector<std::vector<std::pair<VkShaderStageFlagBits, std::vector<uint8_t>>>> stages_bytecode = {stages_bytecode};


{shader_name}::{shader_name}(const GPU* gpu) : Shader(gpu, bindings_sets, vertex_inputs, fragment_inputs, fragment_outputs, stages_bytecode)
{{
}}


{shader_name}::~{shader_name}()
{{
}}
"""

CLASS_HEADER = """#pragma once
#include <RenderEngine/graphics/shaders/Shader.hpp>

namespace RenderEngine
{{
    class {shader_name} : Shader
    {{
    public:
        {shader_name}() = delete;
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
                subprocess.call([GLSLC, source, "-o", spirv])
                print("Compiled:", spirv.relative_to(PATH))
            with open(source, "r", encoding="utf-8") as f:
                src = f.read()
            with open(spirv, "rb") as f:
                spv = f.read()
            code["source"] = src
            code["bytes"] = spv
    return subpasses


def _get_binding_descriptions(subpasses: dict) -> str:
    """
    returns the description of all bindings
    """
    subpass_descriptors = []
    for subpass, stages in subpasses.items():
        bindings = {}
        for stage in STAGES.keys():
            src = stages.get(stage, {"source": ""})["source"]
            matches = BINDING_REGEX.findall(src)
            for (bset, bind, offset, btype, name, count) in matches:
                bset = bset or "0"
                descriptor: dict = bindings.get(bset, {})
                _stages = descriptor.get(name, {"stages": set()})["stages"]
                _stages = _stages | {STAGES[stage]}
                descriptor[name] = {"binding": bind, "offset": offset or "0", "type": btype, "count": count or "1", "stages": _stages}
                bindings[bset] = descriptor
        # generate strings
        descriptors = []
        for set_index, set_descriptor in sorted(bindings.items()):  # for each bindings set
            set_bindings = []
            for name, props in set_descriptor.items():  # for each binding
                type = TYPES.get(props["type"], "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER")
                VkDescriptorSetLayoutBinding = "{"+props["binding"]+", "+type+", "+props["count"]+", "+" | ".join(props["stages"])+", "+"nullptr"+"}"
                set_bindings.append("{\""+name+"\", "+VkDescriptorSetLayoutBinding+"}")
            descriptors.append("{"+", ".join(set_bindings)+"}")
        subpass_descriptors.append("{"+", ".join(descriptors)+"}")
    return "{"+", ".join(subpass_descriptors)+"}"


def constructor(shader_path: pathlib.Path):
    """
    generate C++ constructor source code for the given shader
    """
    subpasses = _get_subpasses_code(shader_path)
    # set vertex inputs
    binding_sets_descriptions = _get_binding_descriptions(subpasses)
    vertex_inputs = ""
    fragment_inputs = ""
    fragment_outputs = ""
    stages_bytecode = ""
    code = CONSTRUCTOR_SRC.format(shader_name=shader_path.name,
                                  bindings_sets=binding_sets_descriptions,
                                  vertex_inputs=vertex_inputs,
                                  fragment_inputs=fragment_inputs,
                                  fragment_outputs=fragment_outputs,
                                  stages_bytecode=stages_bytecode)
    with open(shader_path.with_suffix(".cpp"), "w", encoding="utf-8") as f:
        f.write(code)
    header_path = pathlib.Path(shader_path.as_posix().replace("/src/", "/include/")).with_suffix(".hpp")
    header = CLASS_HEADER.format(shader_name=shader_path.name)
    if not header_path.is_file() or True:
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(header)


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