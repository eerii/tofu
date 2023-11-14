# tofu 🌸

tofu is a tiny opengl 3.3 renderer written in c++.
it is very barebones and it is meant as a learning project.

![Solar system](https://github.com/eerii/tofu/assets/22449369/9eaa9679-6cbf-4849-a67e-2085535907f4)

## features

- basic glfw and opengl setup
- shader loading
- multiple objects per buffer
- instanced rendering
- optional deferred rendering and framebuffer support
- pseudo compute shaders (using transform feedback)
- basic camera
- basic material support
- imgui customizable interface

## examples

- [solar system](ejemplos/sistema_solar) 🪐: a complete example with multiple objects, instanced rendering, compute shaders, occlusion culling, deferred rendering and more
- [crane](ejemplos/grua) 🏗️: a simple example with a single object that can be controlled with the keyboard
- [raymarching](ejemplos/raymarching) 🌈: shader only rendering with raymarching

## building

you will need make and a c++ compiler that supports c++17.

```bash
git clone https://github.com/eerii/tofu.git
cd tofu/examples/[example]
make
```
