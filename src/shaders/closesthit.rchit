#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 payload;
hitAttributeEXT vec3 attribs;

void main()
{
    vec3 baryCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    payload = baryCoords;
}