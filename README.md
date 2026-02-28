glTF Importer/Exporter for Autodesk 3ds Max
This README describes the plug-in projects that add glTF file access capabilities to Autodesk 3ds Max, a premier DCC (Digital Content Creation) tool.

Table of Contents
Introduction

Features

Getting Started

Building the Project

Installation

Usage

Advanced

W.I.P.

Introduction
This project consists of two main plug-in build projects:

HSglTFImporter: A build project for the plug-in that imports glTF files into 3ds Max.

HSglTFExporter: A build project for the plug-in that exports 3ds Max scene data to glTF format.

Features
Import
Object Types: Geometry (Mesh), Shape, Camera, Lights

Format Types: gltf + .bin, gltf (embedded), glb (binary)

Compression: Draco compression supported

Materials (*1):

3ds Max Standard Materials (Scanline, PBR, Physical, glTF, Arnold)

V-Ray and Corona materials

Animation: Object TRS (Translation/Rotation/Scale), Morph weight, KHR_animation_pointer

Vertex Deformation: Skin, Morph

Custom Attributes: Scene, Node, and Material data (stored in extras)

Export
Object Types: Geometry, Shape, Camera, Light

Format Types: gltf + .bin, gltf (embedded), glb (binary)

Compression: Draco compression supported

Supported Materials (*1):

3ds Max Standard Materials (Scanline, PBR, Physical, glTF, Arnold)

V-Ray and Corona materials

Animation: Object TRS, Morph weight, KHR_animation_pointer

Vertex Deformation: Skin, Morph

Custom Attributes: Exports Scene, Node, and Material custom attributes (stored in extras)

Getting Started
To get started with this project, the following environment is required:

Build Environment (Compiler)
Microsoft Visual Studio 2017: For 3ds Max 2020, 2021, and 2022

Microsoft Visual Studio 2019: For 3ds Max 2023, 2024, and 2025

Microsoft Visual Studio 2022: For 3ds Max 2026

Required SDK
Autodesk 3ds Max SDK: Provided by Autodesk.

3rd Party Libraries
This project utilizes the following libraries:

KTX-Software (ktx.lib): Apache License 2.0. c 2013-2020 Mark Callow, The Khronos Group Inc.

Draco (draco.lib): Apache License 2.0. c 2016 Google Inc.

Webp (libwebp.lib / libsharpyuv.lib): BSD 3-Clause License. c 2010, Google Inc.

JsonCpp (JsonCpp_static.lib): MIT License or Public Domain. c 2007-2010 Baptiste Lepilleur and The JsonCpp Authors.

TinyGLTF: MIT License. c 2015-Present Syoyo Fujita.

cgltf: MIT License. c 2018 Johannes Kuhlmann.

Note: TinyGLTF and cgltf may require modifications to support specific glTF extensions.

Building the Project
The project includes two primary build configurations:

Release: Production build.

Hybrid: Debug-ready build (optimized but with debug symbols).

Installation
Upon successfully building each project, two plug-in files will be generated:

HSglTFImporter.dli

HSglTFExporter.dlo

To install, copy these files into the Plugins folder within your 3ds Max installation directory.

Usage
(To be added)

Advanced
(To be added)

W.I.P. (Work In Progress)
(To be added)