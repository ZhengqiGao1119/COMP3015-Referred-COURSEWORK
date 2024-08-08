# COMP3015-Referred-COURSEWORK

## Github:
https://github.com/ZhengqiGao1119/COMP3015-Referred-COURSEWORK

## Youtube:
https://youtu.be/--xvbjq0mPQ

## Environment
- **Visual Studio Version:** [17.7.4]
- **Operating System:** [Windows 11 Home Chinese Version]

## How Does It Work?
- Use WASD for forward, backward, left, and right movement, and mouse rotation to change direction.

## How does the program code work?

### PBR rendering:
- Use `ObjMesh::Load` to load the model
- Use the function `loadTexture` to load model textures
- Use the class `GLSLProgram` to create the PBR rendering shader
- Render the model in `renderscene`

### Shadow rendering:
- Create a framebuffer for shadows, only containing the depth attachment
- Use the function `renderShadow` to render the scene from the light source perspective and generate the shadow map
- Pass the shadow map to the PBR shader to calculate PCF shadows

## What makes your shader program special?
- Implemented PBR rendering, which provides a more realistic scene effect compared to traditional Lambertian rendering models.
- Implemented PCF shadows, which offer softer and more realistic shadow edges compared to hard shadows.
