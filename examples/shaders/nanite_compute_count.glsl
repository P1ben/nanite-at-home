#version 430 core

struct Cluster {
    float parental_cutoff;
    float self_cutoff;

    uint faces_start;
    uint faces_length;
};

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 11) readonly buffer input_clusters {
    Cluster i_clusters[];
};

layout (std140, binding = 13) readonly buffer input_metric {
    float i_metric;
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
        atomicAdd(o_face_count, i_clusters[id].faces_length);
    }
}