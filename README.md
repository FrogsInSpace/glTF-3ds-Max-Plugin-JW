# glTF 2.0 Importer/Exporter for Autodesk 3ds Max

This project adds glTF™ (2.0) file access capabilities to Autodesk 3ds Max®, providing seamless round-trip workflow for importing, editing, and exporting 3D content in glTF format.

## Table of Contents

1. [Introduction](#introduction)
1. [User Documentation](#user-documentation)
1. [Build Requirements](#build-requirements)
1. [Installation](#installation)
1. [Trademarks](#trademarks)

## Introduction

This project consists of two main plug-in build projects:
* **HSglTFImporter**: A plug-in to import glTF files into 3ds Max scenes.
* **HSglTFExporter**: A plug-in to export 3ds Max scene data to glTF files.

## User Documentation

**[End-User Documentation](./User_Documentation/README.md)** is divided into sections:
- **glTF Importer** options for importing and formatting the scene for use in 3ds Max.
- **glTF Editing** tools within 3ds Max for editing and preparing glTF content.
- **glTF Exporter** options for exporting the scene out from 3ds Max into glTF format. 

## Build Requirements

To build or use this project, the following environment is required:

### Build Environment (Compiler)

* **Microsoft Visual Studio 2017**: For 3ds Max 2020, 2021, and 2022
* **Microsoft Visual Studio 2019**: For 3ds Max 2023, 2024, and 2025
* **Microsoft Visual Studio 2022**: For 3ds Max 2026, and 2027

### Required SDK

* **Autodesk 3ds Max SDK**: Provided by Autodesk. 

### Environment Variables 

You must set an environment variable pointing to the installation path of the SDK. Use the variable name corresponding to your specific 3ds Max version:

| 3ds Max Version | Environment Variable Name |
| --- | --- |
| 2020 | ADSK_3DSMAX_SDK_2020 |
| 2021 | ADSK_3DSMAX_SDK_2021 |
| 2022 | ADSK_3DSMAX_SDK_2022 |
| 2023 | ADSK_3DSMAX_SDK_2023 |
| 2024 | ADSK_3DSMAX_SDK_2024 |
| 2025 | ADSK_3DSMAX_SDK_2025 |
| 2026 | ADSK_3DSMAX_SDK_2026 |

### Building the Project

The solution includes two primary build configurations:
* **Release**: Production-ready build.
* **Hybrid**: Optimized build with debug symbols (for development).

## Installation

Once built, the following plug-in files are generated:
1. `HSglTFImporter.dli`
2. `HSglTFExporter.dlo`

Copy these files into the `Plugins` folder of your 3ds Max installation directory.

### Khronos PBR Neutral Tone Mapping

This importer includes a feature to switch the Color Management to use Khronos PBR Neutral Tone when loading glTF files.

To enable this feature (3ds Max 2024 and later): Copy the tone map file located in the Packages folder into your 3ds Max installation directory.

## Trademarks

* 3ds Max® is a registered trademark of Autodesk, Inc.
* glTF™ is a registered trademark of Khronos Group Inc.
* V-Ray® and Chaos Corona® are registered trademarks of Chaos Software EOOD.
* Pencil+® is a registered trademark of P SOFTHOUSE CO., LTD.

### 3rd Party Libraries

This project incorporates the following open-source libraries. We acknowledge and appreciate the contributions of these projects:

* [KTX-Software](https://github.com/KhronosGroup/KTX-Software): Licensed under the Apache License 2.0. Copyright © 2013-2020 Mark Callow, The Khronos Group Inc.
* [Draco](https://github.com/google/draco): Licensed under the Apache License 2.0. Copyright © 2016 Google Inc.
* [libwebp](https://chromium.googlesource.com/webm/libwebp) / libsharpyuv: Licensed under the BSD 3-Clause License. Copyright (c) 2010, Google Inc. All rights reserved.
* [JsonCpp](https://github.com/open-source-parsers/jsoncpp): Licensed under the MIT License or Public Domain. Copyright © 2007-2010 Baptiste Lepilleur and The JsonCpp Authors.
* [TinyGLTF](https://github.com/syoyo/tinygltf): Licensed under the MIT License. Copyright © 2015-Present Syoyo Fujita. 
* [cgltf](https://github.com/jkuhlmann/cgltf): Licensed under the MIT License. Copyright © 2018 Johannes Kuhlmann.

*Note: TinyGLTF and cgltf may require modifications for specific glTF extension support.*

For more information regarding these licenses, please refer to the documentation provided within the source repository.

=========================================

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
