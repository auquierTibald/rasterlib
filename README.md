# Rasterlib #
*A cross-platform multi-threaded software triangle rasterizing library*

### Introduction ###
First of all, rasterlib does not provide any windowing system, it is up to the user to manage the display.
The "only" thing that rasterlib provides is a frame buffer that can be retrieved by calling ``` RL_GetColorBuffer ``` .

### Usage ###
Rasterlib is centered in a drawing context type ( ``` RL_Context ```)  that can be allocated with ``` RL_CreateContext ``` by passing size of the wanted display.

Then, you can provide multiple information like vertex data, textures, and vertex/fragment "shaders" ( callbacks ) to the context before calling ```RL_Draw``` in order to initiate the pipeline that you defined, using the data you provided.
This will perform modifications to the intern frame buffer that can then be retrieved to be used.

### Extensions ###
Rasterlib also provides some 3D utilities like an asset loader for images and .obj 3d models, and a system for 3D Rendering that manages projection, and near plane clipping ( you can choose it by calling ```RL_ProjectionMode``` ).
