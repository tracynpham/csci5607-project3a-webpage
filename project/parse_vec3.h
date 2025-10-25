
//Set the global scene parameter variables

#ifndef PARSE_VEC3_H
#define PARSE_VEC3_H
#define MAX_INPUT 256

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>


//Camera & Scene Parameters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parameters
int img_width = 800, img_height = 600;
std::string imgName = "raytraced.png";

//Camera Parameters
vec3 eye = vec3(0,0,0); 
vec3 forward = vec3(0,0,-1).normalized();
vec3 up = vec3(0,1,0).normalized();
vec3 right;
float halfAngleVFOV = 35; 

//Scene (Sphere) Parameters
vec3 spherePos = vec3(0,0,2);
float sphereRadius = 1; 

//store multiple spheres from a file
int sphere_material_index[MAX_INPUT];
float sphere_x[MAX_INPUT], sphere_y[MAX_INPUT], sphere_z[MAX_INPUT];
float sphere_r[MAX_INPUT];
int sphere_count = 0;

//Background Parameters
vec3 background = vec3(0,0,0);

//Material Paramters

//material defaults
vec3 ambient = vec3(0,0,0);
vec3 diffuse = vec3(1,1,1);
vec3 specular = vec3(0,0,0);
float curr_phong_cos = 5; //ns
vec3 transmissive = vec3(0,0,0);
float curr_ior = 1; //index of reflection

//store multiple materials from a file
float ambient_r[MAX_INPUT], ambient_g[MAX_INPUT], ambient_b[MAX_INPUT];
float diffuse_r[MAX_INPUT], diffuse_g[MAX_INPUT], diffuse_b[MAX_INPUT];
float specular_r[MAX_INPUT], specular_g[MAX_INPUT], specular_b[MAX_INPUT];
float phong_cos[MAX_INPUT];
float trans_r[MAX_INPUT], trans_g[MAX_INPUT], trans_b[MAX_INPUT];
float ior[MAX_INPUT];
int mat_count = 0;


//Light Parameters
vec3 ambient_light = vec3(0,0,0);
int max_depth = 5;

float point_light_r[MAX_INPUT], point_light_g[MAX_INPUT], point_light_b[MAX_INPUT];
float point_light_x[MAX_INPUT], point_light_y[MAX_INPUT], point_light_z[MAX_INPUT];
int num_lights = 0;

float dir_light_r[MAX_INPUT], dir_light_g[MAX_INPUT], dir_light_b[MAX_INPUT];
float dir_light_x[MAX_INPUT], dir_light_y[MAX_INPUT], dir_light_z[MAX_INPUT];
int num_dir_lights = 0;

void parseSceneFile(std::string fileName){
  //TODO: Override the default values with new data from the file "fileName"
  FILE* myFile = fopen(fileName.c_str(), "r");
  if (!myFile) {
    printf("ERROR: Could not open %s\n", fileName.c_str());
    exit(1);
  }
  char line[MAX_INPUT];
  while (fgets(line, sizeof(line), myFile)) {
    if (line[0] == '#' || line[0] == '\n') {
      continue;
    }
    if (strncmp(line, "sphere:", 7) == 0) {
      float x, y, z, r;
      if (sscanf(line + 7, "%f %f %f %f", &x, &y, &z, &r) == 4) {
        sphere_x[sphere_count] = x;
        sphere_y[sphere_count] = y;
        sphere_z[sphere_count] = z;
        sphere_r[sphere_count] = r;
        sphere_material_index[sphere_count] = (mat_count > 0) ? (mat_count - 1) : 0; //avoid invalid indexing
        sphere_count++;
        printf("Sphere: (%f, %f, %f), r = %f\n", x, y, z, r);
      } else {
				std::cerr << "invalid sphere position" << line << std::endl;
			}
    }
    if (strncmp(line, "image_resolution:", 17) == 0) {
      int w, h;
      if (sscanf(line + 17, "%d %d", &w, &h) == 2) {
        img_width = w;
        img_height = h;
        printf("Image resloution: (%d, %d)\n", w, h);
      } else {
				std::cerr << "invalid image resolution" << line << std::endl;
			}
    }
    if (strncmp(line, "output_image:", 13) == 0) {
      char outputFile[MAX_INPUT];
      if (sscanf(line + 13, "%255s", outputFile) == 1) {
        imgName = std::string(outputFile);
        printf("Output image name: %s\n", imgName.c_str());
      } else {
				std::cerr << "invalid output name" << line << std::endl;
			}
    }
    if (strncmp(line, "camera_pos:", 11) == 0) {
      float camPos_x, camPos_y, camPos_z;
      if (sscanf(line + 11, "%f %f %f", &camPos_x, &camPos_y, &camPos_z) == 3) {
        eye = vec3(camPos_x, camPos_y, camPos_z);
        printf("Camera position: (%f, %f, %f)\n", camPos_x, camPos_y, camPos_z);
      } else {
				std::cerr << "invalid camera position" << line << std::endl;
			}
    }
    if (strncmp(line, "camera_fwd:", 11) == 0) {
      float fx, fy, fz;
      if (sscanf(line + 11, "%f %f %f", &fx, &fy, &fz) == 3) {
        forward = vec3(fx, fy, fz).normalized();
        printf("Camera forward direction: (%f, %f, %f)\n", fx, fy, fz);
			}
			else {
				std::cerr << "invalid camera forward direction" << line << std::endl;
      }
    }
    if (strncmp(line, "camera_up:", 10) == 0) {
      float ux, uy, uz;
      if (sscanf(line + 10, "%f %f %f", &ux, &uy, &uz) == 3) {
        up = vec3(ux, uy, uz).normalized();
        printf("Camera up alignment: (%f, %f, %f)\n", ux, uy, uz);
			}
			else {
				std::cerr << "invalid camera up alignment" << line << std::endl;
      }
    }
    if (strncmp(line, "camera_fov_ha:", 14) == 0) {
      float camFov;
      if (sscanf(line + 14, "%f", &camFov) == 1) {
        halfAngleVFOV = camFov;
        printf("Camera half-angle (vertical FOV): %f\n", camFov);
			}
			else {
				std::cerr << "invalid camera half-angle " << line << std::endl;
      }
    }
    if (strncmp(line, "background:", 11) == 0) {
      float backR, backG, backB;
      if (sscanf(line + 11, "%f %f %f", &backR, &backG, &backB) == 3) {
        background = vec3(backR, backG, backB);
        printf("Background: %f %f %f\n",  backR, backG, backB);
      } else {
				std::cerr << "invalid " << line << std::endl;
      }
    }
    if (strncmp(line, "material:", 9) == 0) {
      float ar, ag, ab;
      float dr, dg, db;
      float sr, sg, sb, ns;
      float tr, tg, tb, new_ior;
      if (sscanf(line + 9, 
        "%f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
        &ar, &ag, &ab, &dr, &dg, &db, &sr, &sg, &sb, &ns, &tr, &tg, &tb, &new_ior) == 14) {
      
        //storing each value into an array to deal with multiple materials in a .txt file
        ambient_r[mat_count] = ar;
        ambient_g[mat_count] = ag;
        ambient_b[mat_count] = ab;

        diffuse_r[mat_count] = dr;
        diffuse_g[mat_count] = dg;
        diffuse_b[mat_count] = db;

        specular_r[mat_count] = sr;
        specular_g[mat_count] = sg;
        specular_b[mat_count] = sb;
        phong_cos[mat_count] = ns;

        trans_r[mat_count] = tr;
        trans_g[mat_count] = tg;
        trans_b[mat_count] = tb;
        ior[mat_count] = new_ior;

        mat_count++; //up the material count

        //print statement for testing
        printf("Material: %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", ar, ag, ab, dr, dg, db, sr, sg, sb, ns, tr, tg, tb, new_ior);
      } else {
				std::cerr << "invalid " << line << std::endl;
      }
    }
    if (strncmp(line, "directional_light:", 18) == 0) {
      float dirR, dirG, dirB;
      float dirX, dirY, dirZ;
      if (sscanf(line + 18, "%f %f %f %f %f %f", &dirR, &dirG, &dirB, &dirX, &dirY, &dirZ) == 6) {
        // vec3 dir_light_color = vec3(dirR, dirG, dirB);
        // vec3 dir_light_direction = vec3(dirX, dirY, dirZ);
        dir_light_r[num_dir_lights] = dirR;
        dir_light_g[num_dir_lights] = dirG;
        dir_light_b[num_dir_lights] = dirB;

        dir_light_x[num_dir_lights] = dirX;
        dir_light_y[num_dir_lights] = dirY;
        dir_light_z[num_dir_lights] = dirZ;
        num_dir_lights++;
        printf("Directional light: %f %f %f %f %f %f\n", dirR, dirG, dirB, dirX, dirY, dirZ);
      } else {
				std::cerr << "invalid directional light" << line << std::endl;
      }
    }
    if (strncmp(line, "point_light:", 12) == 0) { // set position light color + pos
      float r, g, b;
      float x, y, z;
      if (sscanf(line + 12, "%f %f %f %f %f %f", &r, &g, &b, &x, &y, &z) == 6) {
        //Point light color
        point_light_r[num_lights] = r;
        point_light_g[num_lights] = g;
        point_light_b[num_lights] = b;

        //Point light position
        point_light_x[num_lights] = x;
        point_light_y[num_lights] = y;
        point_light_z[num_lights] = z;

        num_lights++;
        printf("Point light color: (%f, %f, %f)\n", r, g, b);
        printf("Point light position: (%f, %f, %f)\n", x, y, z);
      }
      else {
        std::cerr << "invalid point light settings" << line << std::endl;
      }
    }
    if (strncmp(line, "spot_light:", 11) == 0) {
      float r, g, b;
      float px, py, pz;
      float dx, dy, dz;
      float ang1, ang2;
      if (sscanf(line + 11, "%f %f %f %f %f %f %f %f %f %f %f",
        &r, &g, &b, &px, &py, &pz, &dx, &dy, &dz, &ang1, &ang2) == 11) {
        vec3 spotLightColor = vec3(r, g, b);
        vec3 spotLightPos = vec3(px, py, pz);
        vec3 spotLightDir = vec3(dx, dy, dz);
        float angle1 = ang1;
        float angle2 = ang2;
        printf("Spot light color: (%f, %f, %f)\n", r, g, b);
        printf("Spot light position: (%f, %f, %f)\n", px, py, pz);
        printf("Spot light direciton: (%f, %f, %f)\n", dx, dy, dz);
        printf("Angle 1: %f\n", ang1);
        printf("Angle2: %f\n", ang2);
      } else {
        std::cerr << "invalid spot light settings" << line << std::endl;
      }
    }
    if (strncmp(line, "ambient_light:", 14) == 0) {
      float r, g, b;
      if (sscanf(line + 14, "%f %f %f", &r, &g, &b) == 3) {
        ambient_light = vec3(r, g, b);
        printf("Ambient light color: (%f, %f, %f)\n", r, g, b);
      } else {
        std::cerr << "invalid ambient light settings" << line << std::endl;
      }
    }
    if (strncmp(line, "max_depth:", 10) == 0) {
      int mx_dpth;
      if (sscanf(line + 10, "%d", &mx_dpth) == 1) {
        max_depth = mx_dpth;
        printf("Max depth: %d\n", mx_dpth);
      } else {
        std::cerr << "invalid max depth" << line << std::endl;
      }
    }
  }
  printf("Orthogonal Camera Basis:\n");
  forward = forward.normalized();
  right = cross(up, forward).normalized();
  up = cross(forward, right).normalized();

  printf("forward: %f,%f,%f\n",forward.x,forward.y,forward.z);
  printf("right: %f,%f,%f\n",right.x,right.y,right.z);
  printf("up: %f,%f,%f\n",up.x,up.y,up.z);

  fclose(myFile);
}

#endif