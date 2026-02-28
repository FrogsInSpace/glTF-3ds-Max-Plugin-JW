Markdown
# glTF? Importer/Exporter for Autodesk 3ds MaxR

This plug-in adds glTF file access capabilities to Autodesk 3ds Max.

## Table of Contents
* [Introduction](#introduction)
* [Features](#features)
* [Getting Started](#getting-started)
* [Installation](#installation)

## Introduction
This project consists of two main plug-in build projects:
* **HSglTFImporter**: Imports glTF files into 3ds Max.
* **HSglTFExporter**: Exports 3ds Max scene data to glTF.

## Features
### Import
* **Object Types**: Geometry (Mesh), Shape, Camera, Lights
* **Format Types**: gltf + .bin, gltf (embedded), glb (binary)
* **Compression**: Draco compression supported

## Getting Started
### Build Environment
* **VS 2017**: For 3ds Max 2020-2022
* **VS 2019**: For 3ds Max 2023-2025
* **VS 2022**: For 3ds Max 2026

### 3rd Party Libraries
This project utilizes the following libraries:
* [KTX-Software](https://github.com/KhronosGroup/KTX-Software) (Apache 2.0)
* [Draco](https://github.com/google/draco) (Apache 2.0)