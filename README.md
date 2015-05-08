# jonsengine
Hobby DX11 game engine

# List of features (2015-05-08)

Core:
- File logging
- Basic memory management
- Basic debugging features

Resources:
- Custom binary file format for parsing game assets
- Offline asset exporter tool
- Various thirdparty libraries to parse a wide range of assets into a unified asset format
- Create basic geometric shapes at runtime

Rendering:
- Directx 11 renderer
- Deferred shading pipeline with phong shading
- Directional/Point/Ambient lights with shadowmapping
- Cascaded shadow mapping (CSM) with Sample distribution shadow maps (SDSM)
- Fast PCF shadow filtering
- Scalable Ambient Obscurance (SSAO)
- Fast approximate antialiasing (FXAA)
- Gamma corrected lighting

Scene:
- Basic hierarchical scenegraph structure
- Models, materials, cameras, directional/point/ambient lights
- Basic renderqueue with asset culling

Window:
- Basic window management
- Raw keyboard/mouse input handling