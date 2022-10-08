#version 460 core

layout(local_size_x = 32, local_size_y = 1, local_size_z = 32) in;

#include lbm.cs

void main(){

  uint ind = (gl_GlobalInvocationID.x*NY + gl_GlobalInvocationID.y)*NZ + gl_GlobalInvocationID.z;

  // Initialize the Boltzmann Distribution

  for(int q = 0; q < Q; q++){

    F[ind*Q + q] = equilibrium(q, 1.0, force);
    if(B[ind] > 0)
      F[ind*Q + q] = equilibrium(q, 1.0, vec3(0));

  }

  RHO[ind] = getRho(ind);
  V[ind] = vec4(getV(ind)/RHO[ind], 1.0);

}
