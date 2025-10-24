//CSCI 5607 HW3 - Rays & Files
//This HW has three steps:
// 1. Compile and run the program (the program takes a single command line argument)
// 2. Understand the code in this file (rayTrace_vec3.cpp), in particular be sure to understand:
//     -How ray-sphere intersection works
//     -How the rays are being generated
//     -The pipeline from rays, to intersection, to pixel color
//    After you finish this step, and understand the math, take the HW quiz on canvas
// 3. Update the file parse_vec3.h so that the function parseSceneFile() reads the passed in file
//     and sets the relevant global variables for the rest of the code to product to correct image

//To Compile: g++ -fsanitize=address -std=c++11 rayTrace_vec3.cpp

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // For fopen and sscanf
#define _USE_MATH_DEFINES 
#endif

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "image_lib.h" //Defines an image class and a color class

//#Vec3 Library
#include "vec3.h"

//High resolution timer
#include <chrono>

//Scene file parser
#include "parse_vec3.h"

//added struct to hold information about the ray intersection with the sphere
struct HitInformation {
  vec3 point; //hit point
  vec3 normal; //normal along hit point
  int sphere_num; //index of the hit sphere
  bool hit = false;
};

//Tests is the ray intersects the sphere
bool raySphereIntersect(vec3 start, vec3 dir, vec3 center, float radius){
  float a = dot(dir,dir); //TODO - Understand: What do we know about "a" if "dir" is normalized on creation?
  vec3 toStart = (start - center);
  float b = 2 * dot(dir,toStart);
  float c = dot(toStart,toStart) - radius*radius;
  float discr = b*b - 4*a*c;
  if (discr < 0) return false;
  else{
    float t0 = (-b + sqrt(discr))/(2*a);
    float t1 = (-b - sqrt(discr))/(2*a);
    if (t0 > 0 || t1 > 0) return true;
  }
  return false;
}

float whereRaySphereIntersect(vec3 start, vec3 dir, vec3 center, float radius){
  float a = dot(dir,dir); 
  vec3 toStart = (start - center);
  float b = 2 * dot(dir,toStart);
  float c = dot(toStart,toStart) - radius*radius;
  float discr = b*b - 4*a*c;
  if (discr < 0) return -1;
  else{
    float t0 = (-b + sqrt(discr))/(2*a);
    float t1 = (-b - sqrt(discr))/(2*a);
    if (t0 > 0 && t1 > 0) 
      return (t0 < t1) ? t0 : t1; //only return the smaller distance
    else if (t0 > 0) //t1 was negative 
      return t0;
    else if (t1 > 0) //t0 was negative
      return t1;
    else
      return -1; //no intersection
  }
}
bool FindIntersection(vec3 start, vec3 dir, HitInformation& hitInfo) {
  float closest_dist = -1;
  for (int s = 0; s < sphere_count; s++) {
    vec3 spherePos = vec3(sphere_x[s], sphere_y[s], sphere_z[s]);
    float radius = sphere_r[s];
    float dist = whereRaySphereIntersect(start, dir, spherePos, radius);
    if (dist > 0 && (closest_dist < 0 || dist < closest_dist)) {
      closest_dist = dist;
      hitInfo.sphere_num = s;
      hitInfo.hit = true;
      hitInfo.point = start + dir * dist;
      hitInfo.normal = (hitInfo.point - spherePos).normalized();
    }
  }
  return hitInfo.hit;
}

Color ApplyLightingModel(vec3 start, vec3 dir, HitInformation& hitInfo) {
  Color contribution = Color(0,0,0); //black
  for (int i = 0; i < num_lights; i++) {
    //light logic goes here
  }
  //more light logic after
  return contribution;
}

Color evaluateRayTree(vec3 start, vec3 dir) {
  bool hit_something = false;
  HitInformation hit;
  hit_something = FindIntersection(start, dir, hit);
  if (hit_something) {
    return ApplyLightingModel(start, dir, hit);
  } else {
    Color color = Color(background.x, background.y, background.z);
  }
}

int main(int argc, char** argv){

  //Read command line paramaters to get scene file
  if (argc != 2){
     std::cout << "Usage: ./a.out scenefile\n";
     return(0);
  }
  std::string secenFileName = argv[1];

  //Parse Scene File
  parseSceneFile(secenFileName);

  float imgW = img_width, imgH = img_height;
  float halfW = imgW/2, halfH = imgH/2;
  float d = halfH / tanf(halfAngleVFOV * (M_PI / 180.0f));

  Image outputImg = Image(img_width,img_height);
  auto t_start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < img_width; i++){
    for (int j = 0; j < img_height; j++){
      float u = (halfW - (imgW)*((i+0.5)/imgW));
      float v = (halfH - (imgH)*((j+0.5)/imgH));
      vec3 p = eye - d*forward + u*right + v*up;
      vec3 rayDir = (p - eye).normalized();
      Color color = evaluateRayTree(eye, rayDir);
      outputImg.setPixel(i, j, color);
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  printf("Rendering took %.2f ms\n",std::chrono::duration<double, std::milli>(t_end-t_start).count());

  outputImg.write(imgName.c_str());
  return 0;
}