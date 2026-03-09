# glTF? Importer/Exporter for Autodesk 3ds MaxR

This project adds glTF? (2.0) file access capabilities to Autodesk 3ds MaxR, providing a seamless workflow for importing and exporting 3D content in the glTF format.

---

## Table of Contents
1. [Introduction](#introduction)
2. [Features](#features)
3. [Getting Started](#getting-started)
4. [Building the Project](#building-the-project)
5. [Installation](#installation)
6. [Usage](#usage)
7. [Advanced](#advanced)
8. [W.I.P.](#wip)

---

## Introduction
This project consists of two main plug-in build projects:
* **HSglTFImporter**: A plug-in to import glTF files into 3ds Max scenes.
* **HSglTFExporter**: A plug-in to export 3ds Max scene data to glTF files.

---

## Features

### Import
* **Object Types**: Geometry (Mesh), Shape, Camera, Lights
* **Format Types**: `*.gltf` + `*.bin`, `*.gltf` (embedded), `*.glb` (binary)
* **Compression**: Draco compression supported
* **Materials**: 
    * 3ds Max Standard Materials (Scanline, PBR, Physical, glTF, Arnold)
    * V-RayR and Chaos Corona materials
* **Animation**: Object TRS, Morph weight, `KHR_animation_pointer`
* **Vertex Deformation**: Skin, Morph
* **Custom Attributes**: Scene, Node, and Material data (stored in `extras`)

### Export
* **Object Types**: Geometry, Shape, Camera, Light
* **Format Types**: `*.gltf` + `*.bin`, `*.gltf` (embedded), `*.glb` (binary)
* **Compression**: Draco compression supported
* **Supported Materials**:
    * 3ds Max Standard Materials (Scanline, PBR, Physical, glTF, Arnold)
    * V-RayR and Chaos Corona materials
* **Animation**: Object TRS, Morph weight, `KHR_animation_pointer`
* **Vertex Deformation**: Skin, Morph
* **Custom Attributes**: Exports Scene, Node, and Material custom attributes (stored in `extras`)

---

## Getting Started

To build or use this project, the following environment is required:

### Build Environment (Compiler)
* **Microsoft Visual Studio 2017**: For 3ds Max 2020, 2021, and 2022
* **Microsoft Visual Studio 2019**: For 3ds Max 2023, 2024, and 2025
* **Microsoft Visual Studio 2022**: For 3ds Max 2026

### Required SDK
* **Autodesk 3ds Max SDK**: Provided by Autodesk.
Environment VariablesYou must set an environment variable pointing to the installation path of the SDK.
Use the variable name corresponding to your specific 3ds Max version:

3ds Max Version    Environment Variable Name

  2020 -------------- ADSK_3DSMAX_SDK_2020  
  2021 -------------- ADSK_3DSMAX_SDK_2021  
  2022 -------------- ADSK_3DSMAX_SDK_2022  
  2023 -------------- ADSK_3DSMAX_SDK_2023  
  2024 -------------- ADSK_3DSMAX_SDK_2024  
  2025 -------------- ADSK_3DSMAX_SDK_2025  
  2026 -------------- ADSK_3DSMAX_SDK_2026  

### 3rd Party Libraries
This project incorporates the following open-source libraries:
* [KTX-Software](https://github.com/KhronosGroup/KTX-Software) (Apache 2.0)
* [Draco](https://github.com/google/draco) (Apache 2.0)
* [libwebp](https://chromium.googlesource.com/webm/libwebp) (BSD 3-Clause)
* [JsonCpp](https://github.com/open-source-parsers/jsoncpp) (MIT)
* [TinyGLTF](https://github.com/syoyo/tinygltf) (MIT)
* [cgltf](https://github.com/jkuhlmann/cgltf) (MIT)

*Note: TinyGLTF and cgltf may require modifications for specific glTF extension support.*

---

## Building the Project
The solution includes two primary build configurations:
* **Release**: Production-ready build.
* **Hybrid**: Optimized build with debug symbols (for development).

---

## Installation
Once built, the following plug-in files are generated:
1. `HSglTFImporter.dli`
2. `HSglTFExporter.dlo`

Copy these files into the `Plugins` folder of your 3ds Max installation directory.

Khronos PBR Neutral Tone Mapping
This importer includes a feature to switch the color map to Khronos PBR Neutral Tone when loading glTF files.
To enable this feature (3ds Max 2024 and later):
Copy the tone map file located in the Packages folder into your 3dsMax installation directory.

---

## Usage
*(Detailed documentation on settings and workflows will be added soon.)*

## Advanced
*(Scripting and advanced configuration details to be added.)*

## W.I.P.
* Enhancing support for `KHR_physics_rigid_bodies` and `KHR_collision_shapes`.
* khr_interactivity

---

### Trademarks
* 3ds Max is a registered trademark of Autodesk, Inc.
* glTF is a registered trademark of Khronos Group Inc.
* V-Ray and Chaos Corona are registered trademarks of Chaos Software Ltd.
