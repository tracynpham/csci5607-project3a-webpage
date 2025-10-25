
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
#include <algorithm>

//added struct to hold information about the ray intersection with the sphere
struct HitInformation {
  vec3 point; //hit point
  vec3 normal; //normal along hit point
  int sphere_num; //index of the hit sphere
  bool hit = false;
  float dist;
};

Color ApplyLightingModel(vec3 start, vec3 dir, HitInformation& hitInfo, int depth = 0);
Color evaluateRayTree(vec3 start, vec3 dir, int depth = 0);


//Tests is the ray intersects the sphere
bool raySphereIntersect(vec3 start, vec3 dir, vec3 center, float radius){
  float a = dot(dir,dir);
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

//returns were exactly the ray intersected the sphere
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
//if the sphere was intersected, then save the information about it in the HitInformation struct
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
      hitInfo.dist = dist;
    }
  }
  return hitInfo.hit;
}
Color evaluateRayTree(vec3 start, vec3 dir, int depth) {
  //base case
  if (depth >= max_depth) {
    return Color(0,0,0);
  }
  bool hit_something = false;
  HitInformation hit;
  hit_something = FindIntersection(start, dir, hit);
  if (hit_something) {
    return ApplyLightingModel(start, dir, hit, depth);
  } else {
    return Color(background.x, background.y, background.z);
  }
}

vec3 reflect(vec3 d, vec3 n) {
  return (d-2.0f*dot(d, n)*n);
}

bool refract(vec3 d, vec3 n, float r, vec3& t) {
  d = d.normalized();
  n = n.normalized();
  float cos_theta = -dot(n, d);
  float k = 1.0f - r * r * (1.0f - cos_theta * cos_theta);
  if (k < 0.0f) return false; 
  t = (r * d) + ((r * cos_theta) - std::sqrt(k)) * n;
  return true;
}


Color ApplyLightingModel(vec3 start, vec3 dir, HitInformation& hitInfo, int depth) {
  vec3 contribution = vec3(0, 0, 0);
  int mat_index = sphere_material_index[hitInfo.sphere_num];

  // getting diffuse, specular, and ambient coefficients from the arrays we parsed
  vec3 kd = vec3(diffuse_r[mat_index], diffuse_g[mat_index], diffuse_b[mat_index]);
  vec3 ks = vec3(specular_r[mat_index], specular_g[mat_index], specular_b[mat_index]);
  vec3 ka = vec3(ambient_r[mat_index], ambient_g[mat_index], ambient_b[mat_index]);
  vec3 kt = vec3(trans_r[mat_index], trans_g[mat_index], trans_b[mat_index]);
  float ns = phong_cos[mat_index]; // phong exponent

  // adding the contribution from the ambient lighting once , no need to loop through the lights to get it 
  contribution = contribution + (ka * ambient_light);

  // compute view vector pointing from hitpoint to the camera
  vec3 V = (eye - hitInfo.point).normalized();

  // loop through all lights in the scene
  for (int i = 0; i < num_lights; i++) {
    // get light position and color, compute light vector pointing from hitpoint towards the light 
    vec3 lightPos = vec3(point_light_x[i], point_light_y[i], point_light_z[i]);
    vec3 lightColor = vec3(point_light_r[i], point_light_g[i], point_light_b[i]);
    vec3 L = (lightPos - hitInfo.point).normalized();

    // shadow ray, small offset to avoid shadows intersectiong the surface
    vec3 shadow = hitInfo.point + hitInfo.normal * 0.001f;
    HitInformation shadow_hit;
    // check if light is blocked by an object
    bool blocked = FindIntersection(shadow, L, shadow_hit);
    float light_distance = (lightPos - hitInfo.point).length(); 
    if (blocked && (shadow_hit.point - shadow).length() < light_distance){
      // this means light was blocked, skip adding diffuse and specular contributions from this light
      continue;
    }
    
    // diffuse lighting
    float diff = std::max(0.0f, dot(hitInfo.normal, L)); // n dot L
    vec3 diffuse = kd * diff;

    //specular lighting (blinn-phong)
    vec3 H = (L + V).normalized();
    float spec_angle = std::max(0.0f, dot(hitInfo.normal, H));
    vec3 specular = ks * pow(spec_angle, ns);

    // combine diffuse + specular contributions
    float attenuation = 1.0f / (light_distance * light_distance);
    vec3 totalLight = (diffuse + specular) * lightColor * attenuation;

    // add this light's contribution to the running total
    contribution = contribution + totalLight * (1.0f / num_lights);
  }
  //more light logic after
  //Following the pseudocode from lecture slides 13
  if (depth < max_depth) {
      float mat_ior = ior[mat_index];
      bool entering = dot(dir, hitInfo.normal) < 0.0f; // determine if ray is entering or exiting the object
      
      // set surface normal to point in correct direction for refractio
      // flip it when exiting 
      vec3 n = hitInfo.normal;
      vec3 nRefract = entering ? n : vec3(-n.x, -n.y, -n.z);

      // set refractive indices for snell's law
      float n1 = entering ? 1.0f : mat_ior;
      float n2 = entering ? mat_ior : 1.0f;
      float snell_ratio = n1 / n2;

      // reflect ray computation (Fresnell effect)
      vec3 r = reflect(dir, nRefract).normalized();
      vec3 reflect_start = hitInfo.point + nRefract * 0.001f;
      Color reflect_color = evaluateRayTree(reflect_start, r, depth + 1);
      vec3 reflect_contrib = vec3(reflect_color.r, reflect_color.g, reflect_color.b);

      // refraction ray (only if refract is true)
      vec3 t;
      bool canRefract = refract(dir, nRefract, snell_ratio, t); //snell's law
      vec3 k = vec3(0.0f, 0.0f, 0.0f);
      if (canRefract) {
          vec3 t_dir = t.normalized();
          vec3 refract_start = hitInfo.point + t_dir * 0.001f;
          Color refract_color = evaluateRayTree(refract_start, t.normalized(), depth + 1);
          k = vec3(refract_color.r, refract_color.g, refract_color.b);
          // beer-lambert attenuation when exiting
          if (!entering) {
            float distance = hitInfo.dist;
            k.x *= exp(-kt.x * distance);
            k.y *= exp(-kt.y * distance);
            k.z *= exp(-kt.z * distance);
          }
      }
      // fresnel effect
      float R0 = powf((n1 - n2) / (n1 + n2), 2.0f); //Schlick approximation
      float c = entering ? std::fabs(dot(dir * -1, nRefract)) : std::fabs(dot(t.normalized(), nRefract));
      float R = canRefract ? (R0 + (1.0f - R0) * powf(1.0f - c, 5.0f)) : 1.0f;
      contribution = contribution + (R * reflect_contrib + (1.0f - R) * (k));
    }
  contribution.clampTo1(); // clamp so none of the exponents exceed 1
  return Color(contribution.x, contribution.y, contribution.z); // this is where i converted it to a color
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
      Color color = evaluateRayTree(eye, rayDir, 0);
      outputImg.setPixel(i, j, color);
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  printf("Rendering took %.2f ms\n",std::chrono::duration<double, std::milli>(t_end-t_start).count());

  outputImg.write(imgName.c_str());
  return 0;
}