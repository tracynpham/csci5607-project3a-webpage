#version 300 es

/* Assignment 5: Artistic Rendering
 * Original C++ implementation by UMN CSCI 4611 Instructors, 2012+
 * GopherGfx implementation by Evan Suma Rosenberg <suma@umn.edu>, 2022-2024
 * License: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 * PUBLIC DISTRIBUTION OF SOURCE CODE OUTSIDE OF CSCI 4611 IS PROHIBITED
 */ 

precision mediump float;

// constants used to indicate the type of each light
#define POINT_LIGHT 0
#define DIRECTIONAL_LIGHT 1

// max number of simultaneous lights handled by this shader
const int MAX_LIGHTS = 8;


// INPUT FROM UNIFORMS SET WITHIN THE MAIN APPLICATION

// position of the camera in world coordinates
uniform vec3 eyePositionWorld;

// properties of the lights in the scene
uniform int numLights;
uniform int lightTypes[MAX_LIGHTS];
uniform vec3 lightPositionsWorld[MAX_LIGHTS];
uniform vec3 lightAmbientIntensities[MAX_LIGHTS];
uniform vec3 lightDiffuseIntensities[MAX_LIGHTS];
uniform vec3 lightSpecularIntensities[MAX_LIGHTS];

// material properties (coefficents of reflection)
uniform vec3 kAmbient;
uniform vec3 kDiffuse;
uniform vec3 kSpecular;
uniform float shininess;

// texture data
uniform int useTexture;
uniform sampler2D surfaceTexture;


// INPUT FROM THE VERTEX SHADER AFTER INTERPOLATION ACROSS TRIANGLES BY THE RASTERIZER

in vec3 interpPositionWorld;
in vec3 interpNormalWorld;
in vec4 interpColor;
in vec2 interpTexCoords;


// OUTPUT

out vec4 fragColor;


void main() {
    // PART 2.0: In class example

// Get position in world space
    vec4 pos = vec4(interpPositionWorld,1);

    // Get normal in world space
    vec3 normal = normalize(interpNormalWorld);
    
    // Initalize illumination
    vec3 illumination = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < numLights; i++) {
        // Get light vector
        vec3 l;
        if (lightTypes[i] == DIRECTIONAL_LIGHT) {
            l = normalize(lightPositionsWorld[i]);
        }
        else {
            l = normalize(lightPositionsWorld[i] - pos.xyz);
        }

        // calculate eye vector
        vec3 e = normalize(eyePositionWorld - pos.xyz);

        // calculate Ambient light
        illumination += kAmbient*lightAmbientIntensities[i];

        // calculate Diffuse light
        illumination += kDiffuse*lightDiffuseIntensities[i]*max(dot(normal,l), 0.0);

        // calculate specular light
        vec3 r = reflect(-l, normal);
        illumination += kSpecular*lightSpecularIntensities[i]*pow(max(dot(e, r), 0.0), shininess);
    }

    fragColor = vec4(illumination,1.0);

    if (useTexture != 0) {
        fragColor *= texture(surfaceTexture, interpTexCoords);
    }
}
