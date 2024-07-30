#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 9) out;

layout(location = 0) in vec3 fragColor[];
layout(location = 0) out vec3 outColor;

void main() {
    vec4 positions[3];
    for (int i = 0; i < 3; i++) {
        positions[i] = gl_in[i].gl_Position;
    }

    bool wrapLeft = false;
    bool wrapRight = false;

    for (int i = 0; i < 3; i++) {
        if (positions[i].x < -1.0) {
            wrapLeft = true;
        }
        if (positions[i].x > 1.0) {
            wrapRight = true;
        }
    }

    // Original triangle
    for (int i = 0; i < 3; i++) {
        gl_Position = positions[i];
        outColor = fragColor[i];
        EmitVertex();
    }
    EndPrimitive();

    if (wrapLeft || wrapRight) {
        for (int i = 0; i < 3; i++) {
            vec4 pos = positions[i];
            if (wrapLeft) {
                if (pos.x < -1.0) pos.x += 2.0;
                gl_Position = pos;
                outColor = fragColor[i];
                EmitVertex();
            }
            if (wrapRight) {
                if (pos.x > 1.0) pos.x -= 2.0;
                gl_Position = pos;
                outColor = fragColor[i];
                EmitVertex();
            }
        }
        EndPrimitive();
    }
}