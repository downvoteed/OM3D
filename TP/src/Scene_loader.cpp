#include "Scene.h"
#include "StaticMesh.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils.h>

#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION
#include <tinygltf/tiny_gltf.h>

bool is_true = true;

namespace OM3D
{
    struct Mesh
    {
        std::unordered_map<int, std::shared_ptr<Texture>> textures;
        std::unordered_map<int, std::shared_ptr<Material>> materials;
    };

    std::shared_ptr<Material> loadMesh(tinygltf::Model gltf,
                                       const tinygltf::Primitive& prim);

    void loadSkeleton(const tinygltf::Model& gltf, const tinygltf::Node& node,
                      const std::shared_ptr<StaticMesh>& static_mesh,
                      std::unordered_map<int, glm::mat4> node_transforms,
                      int j);

    std::vector<AnimationChannel> loadAnimation(const tinygltf::Model& gltf);

    bool display_gltf_loading_warnings = false;

    static size_t component_count(int type)
    {
        switch (type)
        {
        case TINYGLTF_TYPE_SCALAR:
            return 1;
        case TINYGLTF_TYPE_VEC2:
            return 2;
        case TINYGLTF_TYPE_VEC3:
            return 3;
        case TINYGLTF_TYPE_VEC4:
            return 4;
        case TINYGLTF_TYPE_MAT2:
            return 4;
        case TINYGLTF_TYPE_MAT3:
            return 9;
        case TINYGLTF_TYPE_MAT4:
            return 16;
        default:
            return 0;
        }
    }

    static bool decode_index_buffer(const tinygltf::Model& gltf,
                                    const tinygltf::Accessor& accessor,
                                    Span<u32> indices)
    {
        const tinygltf::BufferView& buffer =
            gltf.bufferViews[accessor.bufferView];

        auto decode_indices = [&](u32 elem_size, auto convert_index) {
            const u8* in_buffer = gltf.buffers[buffer.buffer].data.data()
                + buffer.byteOffset + accessor.byteOffset;
            const size_t input_stride =
                buffer.byteStride ? buffer.byteStride : elem_size;

            for (size_t i = 0; i != accessor.count; ++i)
            {
                indices[i] = convert_index(in_buffer + i * input_stride);
            }
        };

        switch (accessor.componentType)
        {
        case TINYGLTF_PARAMETER_TYPE_BYTE:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
            decode_indices(1, [](const u8* data) -> u32 { return *data; });
            break;

        case TINYGLTF_PARAMETER_TYPE_SHORT:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
            decode_indices(2, [](const u8* data) -> u32 {
                return *reinterpret_cast<const u16*>(data);
            });
            break;

        case TINYGLTF_PARAMETER_TYPE_INT:
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
            decode_indices(4, [](const u8* data) -> u32 {
                return *reinterpret_cast<const u32*>(data);
            });
            break;

        default:
            std::cerr << "Index component type not supported" << std::endl;
            return false;
        }

        return true;
    }

    static bool decode_attrib_buffer(const tinygltf::Model& gltf,
                                     const std::string& name,
                                     tinygltf::Accessor& accessor,
                                     Span<Vertex> vertices)
    {
        const tinygltf::BufferView& buffer =
            gltf.bufferViews[accessor.bufferView];

        [[maybe_unused]] const size_t vertex_count = vertices.size();

        auto decode_attribs = [&](auto* vertex_elems, bool isInt = false) {
            using attrib_type =
                std::remove_reference_t<decltype(vertex_elems[0])>;
            using value_type = typename attrib_type::value_type;
            static constexpr size_t size =
                sizeof(attrib_type) / sizeof(value_type);

            const size_t components = component_count(accessor.type);
            const bool normalize = accessor.normalized;

            DEBUG_ASSERT(accessor.count == vertex_count);

            if (components != size)
            {
                if (display_gltf_loading_warnings)
                {
                    std::cerr << "Expected VEC" << size << " attribute, got VEC"
                              << components << std::endl;
                }
            }

            const size_t min_size = std::min(size, components);
            auto convert = [=](const u8* data) {
                attrib_type vec;
                for (size_t i = 0; i != min_size; ++i)
                {
                    if (isInt)
                        vec[int(i)] = data[i];
                    else
                        vec[int(i)] =
                            reinterpret_cast<const value_type*>(data)[i];
                }
                if (normalize)
                {
                    if constexpr (size == 4)
                    {
                        const glm::vec3 n = glm::normalize(glm::vec3(vec));
                        vec[0] = n[0];
                        vec[1] = n[1];
                        vec[2] = n[2];
                    }
                    else
                    {
                        vec = glm::normalize(vec);
                    }
                }
                return vec;
            };
            {
                u8* out_begin = reinterpret_cast<u8*>(vertex_elems);

                const auto& in_buffer = gltf.buffers[buffer.buffer].data;
                const u8* in_begin =
                    in_buffer.data() + buffer.byteOffset + accessor.byteOffset;

                size_t attrib_size = components * sizeof(value_type);
                const size_t input_stride =
                    buffer.byteStride ? buffer.byteStride : attrib_size;

                for (size_t i = 0; i != accessor.count; ++i)
                {
                    /* if (name == "JOINT_0")
                    {
                        glm::vec<4, int> joints = ReadIntegerElement(in_begin +
                    i * input_stride , accessor, components);
                        WriteIntegerElement(joints, components, out_begin + i *
                    sizeof(Vertex));
                    }
                    else */
                    const u8* attrib = in_begin + i * input_stride;
                    DEBUG_ASSERT(attrib < in_buffer.data() + in_buffer.size());
                    *reinterpret_cast<attrib_type*>(
                        out_begin + i * sizeof(Vertex)) = convert(attrib);
                }
            }
        };

        if (name == "POSITION")
        {
            decode_attribs(&vertices[0].position);
        }
        else if (name == "NORMAL")
        {
            decode_attribs(&vertices[0].normal);
        }
        else if (name == "TANGENT")
        {
            decode_attribs(&vertices[0].tangent_bitangent_sign);
        }
        else if (name == "TEXCOORD_0")
        {
            decode_attribs(&vertices[0].uv);
        }
        else if (name == "COLOR_0")
        {
            decode_attribs(&vertices[0].color);
        }
        else if (name == "JOINTS_0")
        {
            decode_attribs(&vertices[0].joints_0, true);
        }
        else if (name == "WEIGHTS_0")
        {
            decode_attribs(&vertices[0].weights_0);
        }
        else
        {
            if (display_gltf_loading_warnings)
            {
                std::cerr << "Attribute \"" << name << "\" is not supported"
                          << std::endl;
            }
        }
        return true;
    }

    static Result<MeshData> build_mesh_data(const tinygltf::Model& gltf,
                                            const tinygltf::Primitive& prim)
    {
        std::vector<Vertex> vertices;
        for (auto&& [name, id] : prim.attributes)
        {
            tinygltf::Accessor accessor = gltf.accessors[id];
            if (!accessor.count)
            {
                continue;
            }

            if (accessor.sparse.isSparse)
            {
                return { false, {} };
            }

            if (!vertices.size())
            {
                std::fill_n(std::back_inserter(vertices), accessor.count,
                            Vertex{});
            }
            else if (vertices.size() != accessor.count)
            {
                return { false, {} };
            }

            if (!decode_attrib_buffer(gltf, name, accessor, vertices))
            {
                return { false, {} };
            }
        }

        std::vector<u32> indices;
        {
            tinygltf::Accessor accessor = gltf.accessors[prim.indices];
            if (!accessor.count || accessor.sparse.isSparse)
            {
                return { false, {} };
            }

            if (!indices.size())
            {
                std::fill_n(std::back_inserter(indices), accessor.count,
                            u32(0));
            }
            else if (indices.size() != accessor.count)
            {
                return { false, {} };
            }

            if (!decode_index_buffer(gltf, accessor, indices))
            {
                return { false, {} };
            }
        }

        return { true, MeshData{ std::move(vertices), std::move(indices) } };
    }

    static Result<TextureData> build_texture_data(const tinygltf::Image& image,
                                                  bool as_sRGB)
    {
        if (image.bits != 8 && image.pixel_type != TINYGLTF_COMPONENT_TYPE_BYTE
            && image.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
        {
            std::cerr << "Unsupported image format (pixel type)" << std::endl;
            return { false, {} };
        }

        ImageFormat format = ImageFormat::RGBA8_UNORM;
        switch (image.component)
        {
        case 3:
            format = as_sRGB ? ImageFormat::RGB8_sRGB : ImageFormat::RGB8_UNORM;
            break;

        case 4:
            format =
                as_sRGB ? ImageFormat::RGBA8_sRGB : ImageFormat::RGBA8_UNORM;
            break;

        default:
            std::cerr << "Unsupported image format (components)" << std::endl;
            return { false, {} };
        }

        auto data = std::make_unique<u8[]>(image.image.size());
        std::copy(image.image.begin(), image.image.end(), data.get());

        return { true,
                 TextureData{ std::move(data),
                              glm::uvec2(image.width, image.height), format } };
    }

    static glm::mat4 parse_node_matrix(const tinygltf::Node& node)
    {
        glm::vec3 translation(0.0f, 0.0f, 0.0f);
        for (u32 k = 0; k != node.translation.size(); ++k)
        {
            translation[k] = float(node.translation[k]);
        }

        glm::vec3 scale(1.0f, 1.0f, 1.0f);
        for (u32 k = 0; k != node.scale.size(); ++k)
        {
            scale[k] = float(node.scale[k]);
        }

        glm::vec4 rotation(0.0f, 0.0f, 0.0f, 1.0f);
        for (u32 k = 0; k != node.rotation.size(); ++k)
        {
            rotation[k] = float(node.rotation[k]);
        }

        const glm::tquat<float> q(rotation.w, rotation.x, rotation.y,
                                  rotation.z);
        return glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(q)
            * glm::scale(glm::mat4(1.0f), scale);
    }

    static glm::mat4 base_transform()
    {
        return glm::mat4(1.0f);
    }

    static void
    parse_node_transforms(int node_index, const tinygltf::Model& gltf,
                          std::unordered_map<int, glm::mat4>& node_transforms,
                          const glm::mat4& parent_transform = base_transform())
    {
        const tinygltf::Node& node = gltf.nodes[node_index];
        const glm::mat4 transform = parent_transform * parse_node_matrix(node);
        node_transforms[node_index] = transform;
        for (int child : node.children)
        {
            parse_node_transforms(child, gltf, node_transforms, transform);
        }
    }

    static void compute_tangents(MeshData& mesh)
    {
        for (Vertex& vert : mesh.vertices)
        {
            vert.tangent_bitangent_sign = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }

        for (size_t i = 0; i < mesh.indices.size(); i += 3)
        {
            const u32 tri[] = { mesh.indices[i + 0], mesh.indices[i + 1],
                                mesh.indices[i + 2] };

            const glm::vec3 edges[] = {
                mesh.vertices[tri[1]].position - mesh.vertices[tri[0]].position,
                mesh.vertices[tri[2]].position - mesh.vertices[tri[0]].position
            };

            const glm::vec2 uvs[] = { mesh.vertices[tri[0]].uv,
                                      mesh.vertices[tri[1]].uv,
                                      mesh.vertices[tri[2]].uv };

            const float dt[] = { uvs[1].y - uvs[0].y, uvs[2].y - uvs[0].y };

            const glm::vec3 tangent =
                -glm::normalize((edges[0] * dt[1]) - (edges[1] * dt[0]));
            mesh.vertices[tri[0]].tangent_bitangent_sign +=
                glm::vec4(tangent, 0.0f);
            mesh.vertices[tri[1]].tangent_bitangent_sign +=
                glm::vec4(tangent, 0.0f);
            mesh.vertices[tri[2]].tangent_bitangent_sign +=
                glm::vec4(tangent, 0.0f);
        }

        for (Vertex& vert : mesh.vertices)
        {
            const glm::vec3 tangent = vert.tangent_bitangent_sign;
            vert.tangent_bitangent_sign =
                glm::vec4(glm::normalize(tangent), 1.0f);
        }
    }

    Result<std::unique_ptr<Scene>>
    Scene::from_gltf(const std::string& file_name)
    {
        const double time = program_time();
        DEFER(std::cout << file_name << " loaded in "
                        << std::round((program_time() - time) * 100.0) / 100.0
                        << "s" << std::endl);

        tinygltf::TinyGLTF ctx;
        tinygltf::Model gltf;
        {
            std::string err;
            std::string warn;

            const bool is_ascii = ends_with(file_name, ".gltf");
            const bool ok = is_ascii
                ? ctx.LoadASCIIFromFile(&gltf, &err, &warn, file_name)
                : ctx.LoadBinaryFromFile(&gltf, &err, &warn, file_name);

            if (!err.empty())
            {
                std::cerr << "Error while loading gltf: " << err << std::endl;
            }
            if (!warn.empty())
            {
                std::cerr << "Warning while loading gltf: " << warn
                          << std::endl;
            }

            if (!ok)
            {
                std::cout << "Failed to parse glTF\n";
                return { false, {} };
            }
        }

        std::cout << file_name << " parsed in "
                  << std::round((program_time() - time) * 100.0) / 100.0 << "s"
                  << std::endl;

        auto scene = std::make_unique<Scene>();

        // map node index and SceneObjet it is

        Mesh mesh;

        std::unordered_map<int, glm::mat4> node_transforms;
        std::vector<std::pair<int, int>> light_nodes;
        {
            std::vector<int> node_indices;
            if (gltf.defaultScene >= 0)
            {
                node_indices = gltf.scenes[gltf.defaultScene].nodes;
            }
            else
            {
                for (u32 i = 0; i != gltf.nodes.size(); ++i)
                {
                    node_indices.push_back(i);
                    node_transforms[i] = base_transform();
                }
            }

            for (const int node_index : node_indices)
            {
                parse_node_transforms(node_index, gltf, node_transforms);
            }

            for (const int node_index : node_indices)
            {
                const auto& node = gltf.nodes[node_index];
                if (const auto it = node.extensions.find("KHR_lights_punctual");
                    it != node.extensions.end())
                {
                    const int light_index = it->second.Get("light").Get<int>();
                    if (light_index < 0 || light_index >= gltf.lights.size())
                    {
                        continue;
                    }
                    light_nodes.emplace_back(
                        std::pair{ node_index, light_index });
                }
            }
        }
        std::map<int, SceneObject> scene_objects;

        for (auto [node_index, node_transform] : node_transforms)
        {
            const tinygltf::Node& node = gltf.nodes[node_index];

            if (node.mesh < 0)
            {
                continue;
            }
            // std::cout << "obj name: " << node.name << std::endl;

            const tinygltf::Mesh& mesh = gltf.meshes[node.mesh];

            for (size_t j = 0; j != mesh.primitives.size(); ++j)
            {
                const tinygltf::Primitive& prim = mesh.primitives[j];

                // print obj name

                if (prim.mode != TINYGLTF_MODE_TRIANGLES)
                {
                    continue;
                }

                auto mesh = build_mesh_data(gltf, prim);
                if (!mesh.is_ok)
                {
                    return { false, {} };
                }

                if (mesh.value.vertices[0].tangent_bitangent_sign
                    == glm::vec4(0.0f))
                {
                    compute_tangents(mesh.value);
                }

                std::shared_ptr<Material> material = loadMesh(gltf, prim);
                std::shared_ptr<StaticMesh> static_mesh =
                    std::make_shared<StaticMesh>(mesh.value);

                // Skeleton

                loadSkeleton(gltf, node, static_mesh, node_transforms, j);

                // Animation

                auto scene_object =
                    SceneObject(static_mesh, std::move(material));

                // TODO
                scene_object.set_transform(node_transform);
                scene->add_object(std::move(scene_object));
                scene->_obj_name_to_index[node.name] = node_index;
                scene_objects.insert(std::pair(node_index, scene_object));
            }
        }
        std::vector<AnimationChannel> animchannels = loadAnimation(gltf);
        scene->set_animators(animchannels);

        // Mesh. Load sphere.glb and add it to the scene

        std::string err;
        std::string warn;

        for (auto [node_index, light_index] : light_nodes)
        {
            const auto& gltf_light = gltf.lights[light_index];

            tinygltf::Model sphereModel;
            if (!ctx.LoadBinaryFromFile(&sphereModel, &err, &warn,
                                        "../../data/sphere.glb"))
            {
                std::cerr << "Failed to load sphere model: " << err
                          << std::endl;
                return { false, {} };
            }

            // Assurez-vous que le modèle de la sphère a au moins une primitive
            if (sphereModel.meshes.empty()
                || sphereModel.meshes[0].primitives.empty())
            {
                std::cerr << "Sphere model is not valid" << std::endl;
                return { false, {} };
            }

            const tinygltf::Primitive& spherePrim =
                sphereModel.meshes[0].primitives[0];
            auto sphereMesh = build_mesh_data(sphereModel, spherePrim);
            if (!sphereMesh.is_ok)
            {
                return { false, {} };
            }
            Material material = Material::textured_lights();
            material.set_blend_mode(BlendMode::Additive);
            material.set_depth_test_mode(DepthTestMode::Standard);
            material.set_cull_mode(CullMode::Front);

            auto scene_object =
                SceneObject(std::make_shared<StaticMesh>(sphereMesh.value),
                            std::make_shared<Material>(material));

            scene_object.set_transform(node_transforms[node_index]);
            scene->add_object(std::move(scene_object));

            is_true = false;

            const glm::vec3 color = glm::vec3(float(gltf_light.color[0]),
                                              float(gltf_light.color[1]),
                                              float(gltf_light.color[2]))
                * float(gltf_light.intensity);
            ;

            PointLight light;
            light.set_position(node_transforms[node_index][3]);
            light.set_color(color);
            if (gltf_light.range > 0.0)
            {
                light.set_radius(float(gltf_light.range));
            }
            else
            {
                const float intensity = glm::dot(color, glm::vec3(1.0f));
                light.set_radius(std::sqrt(
                    intensity * 1000.0f)); // Put radius where lum < 0.1%
            }
            scene->add_light(light);

            // create Light sphere
        }

        return { true, std::move(scene) };
    }

    std::shared_ptr<Material> loadMesh(tinygltf::Model gltf,
                                       const tinygltf::Primitive& prim)
    {
        Mesh mesh;
        auto materials = mesh.materials;
        auto textures = mesh.textures;
        std::shared_ptr<Material> material;
        if (prim.material >= 0)
        {
            auto& mat = materials[prim.material];

            if (!mat)
            {
                const auto& albedo_info =
                    gltf.materials[prim.material]
                        .pbrMetallicRoughness.baseColorTexture;
                const auto& normal_info =
                    gltf.materials[prim.material].normalTexture;

                auto load_texture =
                    [&](auto texture_info,
                        bool as_sRGB) -> std::shared_ptr<Texture> {
                    if (texture_info.texCoord != 0)
                    {
                        std::cerr << "Unsupported texture coordinate "
                                     "channel ("
                                  << texture_info.texCoord << ")" << std::endl;
                        return nullptr;
                    }

                    if (texture_info.index < 0)
                    {
                        return nullptr;
                    }

                    const int index = gltf.textures[texture_info.index].source;
                    if (index < 0)
                    {
                        return nullptr;
                    }

                    auto& texture = textures[index];
                    if (!texture)
                    {
                        if (const auto r =
                                build_texture_data(gltf.images[index], as_sRGB);
                            r.is_ok)
                        {
                            texture = std::make_shared<Texture>(r.value);
                        }
                    }
                    return texture;
                };

                auto albedo = load_texture(albedo_info, true);
                auto normal = load_texture(normal_info, false);

                if (!albedo)
                {
                    mat = Material::empty_material();
                }
                else if (!normal)
                {
                    mat = std::make_shared<Material>(
                        Material::textured_material());
                    mat->set_texture(0u, albedo);
                }
                else
                {
                    mat = std::make_shared<Material>(
                        Material::textured_normal_mapped_material());
                    mat->set_texture(0u, albedo);
                    mat->set_texture(1u, normal);
                }
                mat->set_cull_mode(OM3D::CullMode::Back);
            }
            material = mat;
        }
        return material;
    }

    Node fillNode(tinygltf::Model gltf, tinygltf::Skin skin, int i)
    {
        Node joint_node = Node();
        joint_node.index = skin.joints[i];
        if (gltf.nodes[skin.joints[i]].translation.size() > 0)
        {
            joint_node.translation = glm::vec3(
                    gltf.nodes[skin.joints[i]].translation[0],
                    gltf.nodes[skin.joints[i]].translation[1],
                    gltf.nodes[skin.joints[i]].translation[2]);
        }
        if (gltf.nodes[skin.joints[i]].rotation.size() > 0)
        {
            joint_node.rotation = glm::vec4(
                    gltf.nodes[skin.joints[i]].rotation[0],
                    gltf.nodes[skin.joints[i]].rotation[1],
                    gltf.nodes[skin.joints[i]].rotation[2],
                    gltf.nodes[skin.joints[i]].rotation[3]);
        }
        if (gltf.nodes[skin.joints[i]].scale.size() > 0)
        {
            joint_node.scale = glm::vec3(
                    gltf.nodes[skin.joints[i]].scale[0],
                    gltf.nodes[skin.joints[i]].scale[1],
                    gltf.nodes[skin.joints[i]].scale[2]);
        }

        return joint_node;
    }

    void loadSkeleton(const tinygltf::Model& gltf, const tinygltf::Node& node,
                      const std::shared_ptr<StaticMesh>& static_mesh,
                      std::unordered_map<int, glm::mat4> node_transforms,
                      const int j)
    {
        if (gltf.skins.size() > 0)
        {
            const tinygltf::Skin& skin = gltf.skins[node.skin];
            if (skin.joints.size() > 0)
            {
                const tinygltf::Accessor& accessor =
                    gltf.accessors[skin.inverseBindMatrices];
                assert(accessor.type == TINYGLTF_TYPE_MAT4);

                const tinygltf::BufferView& bufferView =
                    gltf.bufferViews[accessor.bufferView];

                const tinygltf::Buffer& buffer =
                    gltf.buffers[bufferView.buffer];

                const float* ptr = reinterpret_cast<const float*>(
                    buffer.data.data() + accessor.byteOffset
                    + bufferView.byteOffset);

                glm::mat4 inverse_bind_matrices;

                glm::mat4 m;
                //memcpy(glm::value_ptr(m), ptr + j * 16, 16 * sizeof(float));
                memcpy(glm::value_ptr(m), ptr, 16 * sizeof(float));
                inverse_bind_matrices = m;

                // print inverse_bind_matrices
                std::cout << "inverse_bind_matrices = " << std::endl;
                for (int i = 0; i < 4; i++)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        std::cout << inverse_bind_matrices[i][j] << " ";
                    }
                    std::cout << std::endl;
                }

                std::cout << "num joints = " << skin.joints.size() << std::endl;
                // each joint is an index which is the index of the node
                // in the gltf.nodes array
                std::map<int, glm::mat4> joint_matrices;
                std::map<int, Node> joint_nodes;
                for (int i = 0; i < skin.joints.size(); i++)
                {
                    const glm::mat4 global_node_tranform =
                        node_transforms[skin.joints[i]];
                    glm::mat4 inverse_joint_node_tranform =
                        glm::inverse(global_node_tranform);
                    glm::mat4 joint_transform =
                        global_node_tranform * inverse_bind_matrices;
                    joint_matrices.insert(
                        { skin.joints[i],
                          inverse_joint_node_tranform * joint_transform
                              * inverse_bind_matrices });
                    // * inverse_bind_matrices ?

                    Node joint_node = fillNode(gltf, skin, i);
                    joint_nodes.insert({ skin.joints[i], joint_node });
                }

                const auto skeleton = std::make_shared<Skeleton>(
                    inverse_bind_matrices, joint_matrices);
                skeleton->set_nodes(joint_nodes);
                static_mesh->set_skeleton(skeleton);
            }
        }
    }

    std::vector<AnimationChannel> loadAnimation(const tinygltf::Model& gltf)
    {
        if (gltf.animations.size() > 0)
        {
            const tinygltf::Animation& animation = gltf.animations[0];
            std::cout << "anim name = " << animation.name << std::endl;
            std::cout << "num channels = " << animation.channels.size()
                    << std::endl;
            std::cout << "num samplers = " << animation.samplers.size()
                    << std::endl;
            std::cout << "num nodes = " << gltf.nodes.size() << std::endl;
                
            /* std::vector<std::vector<glm::mat4>> node_matrices;
            for (int i = 0; i < gltf.nodes.size(); i++)
            {
                node_matrices.push_back(std::vector<glm::mat4>());
            } */

            std::vector<AnimationChannel> animChannels;

            for (int i = 0; i < animation.channels.size(); i++)
            {

                const tinygltf::AnimationChannel& channel =
                    animation.channels[i];
                const tinygltf::AnimationSampler& sampler =
                    animation.samplers[channel.sampler];

                const tinygltf::Accessor& input_accessor =
                    gltf.accessors[sampler.input];
                const tinygltf::Accessor& output_accessor =
                    gltf.accessors[sampler.output];

                const tinygltf::BufferView& input_bufferView =
                    gltf.bufferViews[input_accessor.bufferView];
                const tinygltf::BufferView& output_bufferView =
                    gltf.bufferViews[output_accessor.bufferView];

                const tinygltf::Buffer& input_buffer =
                    gltf.buffers[input_bufferView.buffer];

                const auto* input_ptr = reinterpret_cast<const float*>(
                    input_buffer.data.data() + input_accessor.byteOffset
                    + input_bufferView.byteOffset);

                std::vector<float> input_v;

                DEBUG_ASSERT(input_accessor.componentType == TINYGLTF_PARAMETER_TYPE_FLOAT);
                for (int x = 0; x < input_accessor.count; x++)
                {
                    float temp;
                    memcpy(&temp, input_ptr + x, sizeof(float));

                    input_v.push_back(temp);
                }

                std::vector<glm::vec4> output_v;

                const tinygltf::Buffer& output_buffer =
                    gltf.buffers[output_bufferView.buffer];
                const auto* output_ptr = reinterpret_cast<const float*>(
                    output_buffer.data.data() + output_accessor.byteOffset
                    + output_bufferView.byteOffset);

                DEBUG_ASSERT(output_accessor.componentType == TINYGLTF_PARAMETER_TYPE_FLOAT);
                for (int x = 0; x < output_accessor.count; x++)
                {
                    if(output_accessor.type == TINYGLTF_TYPE_VEC4)  {
                        glm::vec4 temp;
                        memcpy(&temp, output_ptr + (x * 4), sizeof(glm::vec4));
                        output_v.push_back(temp);
                    } else if(output_accessor.type == TINYGLTF_TYPE_VEC3) {
                        glm::vec3 temp;
                        memcpy(&temp, output_ptr + (x * 3), sizeof(glm::vec3));
                        output_v.push_back(glm::vec4(temp, 0.0f));
                    } else {
                        DEBUG_ASSERT(false);
                    }
                }

                /* Node target = Node();
                target.index = channel.target_node;
                if (gltf.nodes[channel.target_node].translation.size() > 0)
                {
                    target.translation = glm::vec3(
                        gltf.nodes[channel.target_node].translation[0],
                        gltf.nodes[channel.target_node].translation[1],
                        gltf.nodes[channel.target_node].translation[2]);
                }
                if (gltf.nodes[channel.target_node].rotation.size() > 0)
                {
                    target.rotation = glm::vec4(
                        gltf.nodes[channel.target_node].rotation[0],
                        gltf.nodes[channel.target_node].rotation[1],
                        gltf.nodes[channel.target_node].rotation[2],
                        gltf.nodes[channel.target_node].rotation[3]);
                }
                if (gltf.nodes[channel.target_node].scale.size() > 0)
                {
                    target.scale = glm::vec3(
                        gltf.nodes[channel.target_node].scale[0],
                        gltf.nodes[channel.target_node].scale[1],
                        gltf.nodes[channel.target_node].scale[2]);
                } */

                PathType path_type = PathType::TRANSLATION;
                if (channel.target_path == "translation")
                    path_type = PathType::TRANSLATION;
                else if (channel.target_path == "rotation")
                    path_type = PathType::ROTATION;
                else if (channel.target_path == "scale")
                    path_type = PathType::SCALE;
                else
                    std::cerr << "Animation channel without path type" << std::endl;

                InterpolationType::Type interpolation_type = InterpolationType::Type::LINEAR;
                if (sampler.interpolation == "LINEAR")
                    interpolation_type = InterpolationType::Type::LINEAR;
                else if (sampler.interpolation == "STEP")
                    interpolation_type = InterpolationType::Type::STEP;
                else if (sampler.interpolation == "CUBICSPLINE")
                    interpolation_type = InterpolationType::Type::CUBICSPLINE;
                else
                    std::cerr << "Animation sampler without interpolation type" << std::endl;

                AnimationSampler animSampler = AnimationSampler(input_v, output_v, interpolation_type);

                AnimationChannel animChannel = AnimationChannel(animSampler, channel.target_node, path_type);

                animChannels.push_back(animChannel);
            }
            return animChannels;
        }
        return {};
    }
}
