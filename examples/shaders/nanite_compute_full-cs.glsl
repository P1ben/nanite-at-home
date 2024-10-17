#version 430 core

struct Cluster {
    float parental_cutoff;
    float self_cutoff;

    uint faces_start;
    uint faces_length;
    uint vertex_start;
    uint vertex_length;
};

struct Vertex {
    float position[3];
    float normal[3];
    float color[3];
    float uv[2];
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 11) readonly buffer input_clusters {
    Cluster i_clusters[];
};

layout (std140, binding = 12) readonly buffer input_metric {
    float i_metric;
};

layout (std430, binding = 13) readonly buffer input_faces {
    uint i_faces[];
};

layout (std430, binding = 14) writeonly buffer output_faces {
    uint o_faces[];
};

layout (std140, binding = 15) buffer output_face_count {
    uint o_face_count;
};

layout (std430, binding = 16) readonly buffer input_vertices {
    Vertex i_vertices[];
};

layout (std430, binding = 17) writeonly buffer output_vertices {
    Vertex o_vertices[];
};

layout (std140, binding = 18) buffer output_vertex_count {
    uint o_vertex_count;
};

	
void main() {
    uint id = gl_GlobalInvocationID.x;
    if (id >= i_clusters.length()) {
        return;
    }
    if (i_clusters[id].self_cutoff < i_metric && i_clusters[id].parental_cutoff >= i_metric) {
        uint vertex_start_pos = atomicAdd(o_vertex_count, i_clusters[id].vertex_length);
        uint face_start_pos   = atomicAdd(o_face_count,   i_clusters[id].faces_length );

        for (uint i = 0; i < i_clusters[id].vertex_length; i++) {
            uint vert_start = (i_clusters[id].vertex_start + i);
            uint position   = (vertex_start_pos + i);
            o_vertices[position].position = i_vertices[vert_start].position;
            o_vertices[position].normal = i_vertices[vert_start].normal;
            o_vertices[position].color = i_vertices[vert_start].color;
            o_vertices[position].uv = i_vertices[vert_start].uv;
        }

        for (uint i = 0; i < i_clusters[id].faces_length; i++) {
            uint face_start = 3 * (i_clusters[id].faces_start + i);
            uint position   = 3 * (face_start_pos + i);
            for (int j = 0; j < 3; j++) {
                o_faces[position + j] = i_faces[face_start + j] + vertex_start_pos;
            }
        }
    }
}