#version 430 core

struct Cluster {
    float parental_cutoff;
    float self_cutoff;

    uint faces_start;
    uint faces_length;
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 11) readonly buffer input_clusters {
    Cluster i_clusters[];
};

layout (std430, binding = 12) readonly buffer input_faces {
    uint i_faces[];
};

layout (std140, binding = 13) readonly buffer input_metric {
    float i_metric;
};

layout (std430, binding = 14) buffer output_faces {
    uint o_faces[];
};

layout (std140, binding = 15) buffer output_face_count {
    uint o_face_count;
};

	
void main() {
    uint id = gl_GlobalInvocationID.x;
    if (id >= i_clusters.length()) {
        return;
    }
    if (i_clusters[id].self_cutoff < i_metric && i_clusters[id].parental_cutoff >= i_metric) {
        uint start_pos = atomicAdd(o_face_count, i_clusters[id].faces_length);
        for (uint i = 0; i < i_clusters[id].faces_length; i++) {
            uint face_start = 3 * (i_clusters[id].faces_start + i);
            uint position   = 3 * (start_pos + i);
            for (int j = 0; j < 3; j++) {
                o_faces[position + j] = i_faces[face_start + j];
            }
        }
    }
}