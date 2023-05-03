#version 330 core

in vec2 uv;

uniform float time;

out vec4 color_out;

// Constantes
const float MIN_HIT_DISTANCE = 0.01;
const float MAX_TRACE_DISTANCE = 50.0;

// Materiales
const vec3 materiales[4] = vec3[](
    vec3(0.5, 0.9, 0.4),
    vec3(0.8, 0.3, 0.9),
    vec3(0.9, 0.4, 0.4),
    vec3(0.3, 0.3, 0.8)
);

// Variables
vec3 camera_pos = vec3(0., 0.0, -2.);
vec3 luz_pos = vec3(2.0, 5.0, 2.0);

// ---
// Signed distance functions

// - Sphere
float sdf_sphere(vec3 p, vec3 c, float r) {
    return length(p - c) - r;
}
// - Cube
float sdf_cube(vec3 p, vec3 c, vec3 b, float r) {
    vec3 q = abs(p - c) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}
// - Plane
float sdf_plane(vec3 p, vec3 n, float d) {
    return dot(p, n) + d;
}
// - Torus
float sdf_torus(vec3 p, vec3 c, vec2 t) {
  vec2 q = vec2(length(p.xz - c.xz) - t.x, p.y - c.y);
  return length(q) - t.y;
}
// - Mandelbulb
float sdf_mandelbulb(vec3 p, float power, float iterations) {
    vec3 z = p;
    float dr = 1.0;
    float r = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        r = length(z);
        
        if (r > 2.0) break;
        
        // Convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);
        
        dr = pow(r, power - 1.0) * power * dr + 1.0;
        
        // Scale and rotate the point
        float zr = pow(r, power);
        theta = theta * power;
        phi = phi * power;
        
        // Convert back to cartesian coordinates
        z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        z += p;
    }
    
    return 0.5 * log(r) * r / dr;
}

// ---
// Escena

struct Object {
    float dist;
    int material;
};

Object min_(Object a, Object b) {
    return (a.dist <= b.dist) ? a : b;
}

Object scene(vec3 p) {
    /*Object s = Object(sdf_sphere(p, vec3(1.0, 0.3 * sin(time * 2.0), 1.3), 1.3), -1); // Sphere mirror
    s = min_(s, Object(sdf_sphere(p, vec3(0.0, 0.3 + 0.1 * cos(time * 1.5), -0.5), 0.3), 1)); // Sphere
    s = min_(s, Object(sdf_cube(p, vec3(2.0, 1.02, 0.1), vec3(0.3), 0.1), 2)); // Cube
    s = min_(s, Object(sdf_plane(p, vec3(0.0, -1.0, 0.0), 1.5), 3)); // Ground*/

    Object s = Object(sdf_mandelbulb(p, 8.0 + 2. * sin(time), 8.0), 0);
    
    return s;
}

// ---
// Calcular color

float sombra(vec3 ray_origin, vec3 ray_dir, float k) {
    float res = 1.0;
    float ph = 1e20;
    float trace_distance = 0.0;
    
    for (int i = 0; i < 32; i++) {
        vec3 current_pos = ray_origin + trace_distance * ray_dir;
        Object closest = scene(current_pos);
        
        if (closest.dist < MIN_HIT_DISTANCE) {
            return 0.0;
        }
        
        float y = closest.dist*closest.dist/(2.0*ph);
        float d = sqrt(closest.dist*closest.dist-y*y);
        res = min(res, k*d/max(0.0,trace_distance-y));
        ph = closest.dist;
        trace_distance += closest.dist;
    }
    
    return res;
}

vec3 calculate_color(int material, vec3 pos, vec3 norm) {
    vec3 color = materiales[material];
    
    // Luz difusa
    vec3 luz_dir = normalize(pos - luz_pos);
    vec3 luz_direccional = vec3(max(0.0, dot(norm, luz_dir)));
    vec3 luz_ambiente = vec3(0.02);
    
    color = max(color * (luz_direccional + luz_ambiente), luz_ambiente);
    
    // Sombras
    vec3 sombra_ray_origin = pos + norm * 0.2;
    float sombra_luz = sombra(sombra_ray_origin, luz_dir, 4);
    color = mix(color, color*0.2, 1.0 - sombra_luz);
    
    return color;
}

// ---
// Calculos

vec3 normal(vec3 p) {
    const vec3 small_step = vec3(0.001, 0.0, 0.0);

    float gradient_x = scene(p + small_step.xyy).dist - scene(p - small_step.xyy).dist;
    float gradient_y = scene(p + small_step.yxy).dist - scene(p - small_step.yxy).dist;
    float gradient_z = scene(p + small_step.yyx).dist - scene(p - small_step.yyx).dist;

    vec3 n = vec3(gradient_x, gradient_y, gradient_z);
    
    return normalize(n);
}

float rand(vec2 coord) {
    return fract(sin(dot(coord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// ---
// Ray marching

Object ray_march(vec3 ray_origin, vec3 ray_dir, int max_steps) {
    float trace_distance = 0.0;
    
    for (int i = 0; i < max_steps; i++) {
        vec3 current_pos = ray_origin + trace_distance * ray_dir;
        Object closest = scene(current_pos);
        
        if (closest.dist < MIN_HIT_DISTANCE) { // Hit!
            return Object(trace_distance, closest.material);
        }
        
        if (trace_distance > MAX_TRACE_DISTANCE)
            break;
        
        trace_distance += closest.dist;
    }
    
    return Object(-1.0, 0); // Miss
}

// ---
// Render

vec3 render(vec3 ray_origin, vec3 ray_dir) {
    vec3 color;
    Object ray = ray_march(ray_origin, ray_dir, 256);
    
    if (ray.dist == -1.0) { // Cielo
        color = vec3(0.4, 0.4, 0.8) - (ray_dir.y * 0.2);
    } else { // Objeto
        vec3 pos = ray_origin + ray.dist * ray_dir;
        vec3 pos_normal = normal(pos);
        
        // Color
        if (ray.material >= 0) {
            color = calculate_color(ray.material, pos, pos_normal);
        } else {
            vec3 r_ray_origin = pos + pos_normal * 0.1;
            Object r_ray = ray_march(r_ray_origin, pos_normal, 32);
            vec3 r_pos = r_ray_origin + r_ray.dist * pos_normal;
            vec3 r_norm = normal(r_pos);
            
            if (r_ray.dist == -1)
                color = vec3(0.4, 0.4, 0.8) * 0.7 + color * 0.5;
            else
                color = calculate_color(r_ray.material, r_pos, r_norm) * 0.7 + color * 0.5;
        }
    }
    
    return color;
}

// ---

void main() {
    vec3 ro = camera_pos;
    vec3 rd = vec3(uv, 1.0);
    
    color_out = vec4(render(ro, rd), 1.0);
}
