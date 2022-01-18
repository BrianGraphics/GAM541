////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "Camera.h"
#include "fbo.h"

enum ObjectIds {
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    roomId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
    floorId     = 11,
    thirdPicId  = 12
};

class Shader;


class Scene
{
public:
    GLFWwindow* window;

    // @@ Declare interactive viewing variables here. (spin, tilt, ry, front back, ...)
	float spin, tilt, 
		  tx, ty, zoom,
	      rx, ry,
	      front, back;

    glm::vec3 eye;
    float speed;
    float time_last_frame;

    // Light parameters
    float lightSpin, lightTilt, lightDist;
    float lightRx, lightRy;
    glm::vec3 lightPos;
    glm::vec3 Light;
    glm::vec3 Ambient;

    // @@ Perhaps declare additional scene lighting values here. (lightVal, lightAmb)
    glm::vec3 lookAtPos;
    glm::vec3 lookAtV;
    glm::vec3 lookAtA;
    glm::vec3 lookAtB;


    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    bool transformaiton_mode;
    bool w_down;
    bool a_down;
    bool s_down;
    bool d_down;
    
    // Viewport
    int width, height;

    // Transformations
    glm::mat4 WorldProj, WorldView, WorldInverse;

    // Transformations from light's POV
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::mat4 ShadowMatrix;

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
    Object *central, *anim, *room, *floor, *teapot, *podium, *sky,
            *ground, *sea, *spheres, *leftFrame, *rightFrame, *thirdFrame,
            *box1, *box2;

    std::vector<Object*> animated;
    ProceduralGround* proceduralground;

    // Shader programs
    ShaderProgram* lightingProgram;
    ShaderProgram* shadowProgram;
    // @@ Declare additional shaders if necessary

    //FBO
    FBO fbo;
    FBO top, bot;

    // Options menu stuff
    bool show_demo_window;

    //hdr
    std::string inName, outName;
    int inWidth, inHeight, outWidth, outHeight;
    std::vector<float> inImage;
    Texture* irr;
    float e;

    Camera* camera;

    //void readHDR(const std::string inName, std::vector<float>& image, int& width, int& height);
    //void writeHDR(const std::string outName, std::vector<float>& image, const int width, const int height);
    void InitializeScene();
    void BuildTransforms();
    void DrawMenu();
    void DrawScene();
};
