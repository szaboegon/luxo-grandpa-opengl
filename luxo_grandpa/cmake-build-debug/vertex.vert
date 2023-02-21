#version 450
precision highp float;

uniform vec3 wLookAt, wRight, wUp;
layout(location = 0) in vec2 cVertexPosition;
out vec3 p;

void main() {
    gl_Position = vec4(cVertexPosition, 0, 1);
    p=wLookAt+wRight*cVertexPosition.x+wUp*cVertexPosition.y;
}