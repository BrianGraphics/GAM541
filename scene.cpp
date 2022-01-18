////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

const bool fullPolyCount = true; // Use false when emulating the graphics pipeline in software

#include "math.h"
#include <iostream>
#include <stdlib.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <glu.h>                // For gluErrorString

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
//#define REFL
#include <glm/glm.hpp>
#include <glm/ext.hpp>          // For printing GLM objects with to_string

#include "framework.h"
#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "transform.h"

//#include "rgbe.h"


const float PI = 3.14159f;
const float rad = PI/180.0f;    // Convert degrees to radians

glm::mat4 Identity;

const float grndSize = 100.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// throughout your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as after every OpenGL call.  At the very least, once per
// refresh will tell you if something is going wrong.
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line scene.cpp:%d): %s\n", __LINE__, gluErrorString(err)); exit(-1);} }

// Create an RGB color from human friendly parameters: hue, saturation, value
glm::vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return glm::vec3(v,v,v);

    int i = (int)(h*6.0) % 6;
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if      (i == 0)     return glm::vec3(v,t,p);
    else if (i == 1)  return glm::vec3(q,v,p);
    else if (i == 2)  return glm::vec3(p,v,t);
    else if (i == 3)  return glm::vec3(p,q,v);
    else if (i == 4)  return glm::vec3(t,p,v);
    else   /*i == 5*/ return glm::vec3(v,p,q);
}

////////////////////////////////////////////////////////////////////////
// Constructs a hemisphere of spheres of varying hues
Object* SphereOfSpheres(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);
    
    for (float angle=0.0;  angle<360.0;  angle+= 18.0)
        for (float row=0.075;  row<PI/2.0;  row += PI/2.0/6.0) {   
            glm::vec3 hue = HSV2RGB(angle/360.0, 1.0f-2.0f*row/PI, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId,
                                    //hue, glm::vec3(1.0, 1.0, 1.0), 120.0);
                                    hue, glm::vec3(3.0, 3.0, 3.0), 120.0);
            float s = sin(row);
            float c = cos(row);
            ob->add(sp, Rotate(2,angle)*Translate(c,0,s)*Scale(0.075*c,0.075*c,0.075*c));
        }
    return ob;
}

////////////////////////////////////////////////////////////////////////
// Constructs a -1...+1  quad (canvas) framed by four (elongated) boxes
Object* FramedPicture(const glm::mat4& modelTr, const int objectId, 
                      Shape* BoxPolygons, Shape* QuadPolygons)
{
    // This draws the frame as four (elongated) boxes of size +-1.0
    float w = 0.05;             // Width of frame boards.
    
    Object* frame = new Object(NULL, nullId);
    Object* ob;
    
    glm::vec3 woodColor(87.0/255.0,51.0/255.0,35.0/255.0);
    ob = new Object(BoxPolygons, frameId,
                    woodColor, glm::vec3(0.2, 0.2, 0.2), 10.0);
    frame->add(ob, Translate(0.0, 0.0, 1.0+w)*Scale(1.0, w, w));
    frame->add(ob, Translate(0.0, 0.0, -1.0-w)*Scale(1.0, w, w));
    frame->add(ob, Translate(1.0+w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));
    frame->add(ob, Translate(-1.0-w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));

    ob = new Object(QuadPolygons, objectId,
                    woodColor, glm::vec3(0.0, 0.0, 0.0), 10.0);
    frame->add(ob, Rotate(0,90));

    return frame;
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as setting a
// number of other parameters.
void Scene::InitializeScene()
{
    glEnable(GL_DEPTH_TEST);  
    CHECKERROR;

    // @@ Initialize interactive viewing variables here. (spin, tilt, ry, front back, ...)
	spin = 0.0f;
	tilt = 30.0f;

	tx = 0.0f;
	ty = 0.0f;
	zoom = 25.0f;

    width = 400.0f;
    height = 300.0f;

	ry = 0.4f;
    rx = ry * width / height;
	
	front = 0.5f;
	back = 5000.0f;

    eye = glm::vec3(0.0f, -20.0f, 0.0f);
    speed = 10.0f;
    time_last_frame = (float)glfwGetTime();

    transformaiton_mode = false;
    w_down = false;
    a_down = false;
    s_down = false;
    d_down = false;

    // Set initial light parameters
    lightSpin = 150.0;
    lightTilt = -45.0;
    lightDist = 100.0;

    // @@ Perhaps initialize additional scene lighting values here. (lightVal, lightAmb)
    lightRx = 40.0 / lightDist;
    lightRy = lightRx;

    CHECKERROR;
    objectRoot = new Object(NULL, nullId);

    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);

    // Create the lighting shader program from source code files.
    // @@ Initialize additional shaders if necessary
    lightingProgram = new ShaderProgram();
    lightingProgram->AddShader("final.vert", GL_VERTEX_SHADER);
    lightingProgram->AddShader("final.frag", GL_FRAGMENT_SHADER);
    lightingProgram->AddShader("lighting.vert", GL_VERTEX_SHADER);
    lightingProgram->AddShader("lighting.frag", GL_FRAGMENT_SHADER);

    shadowProgram = new ShaderProgram();
    shadowProgram->AddShader("shadow.vert", GL_VERTEX_SHADER);
    shadowProgram->AddShader("shadow.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(shadowProgram->programId,   0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");

    lightingProgram->LinkProgram();
    shadowProgram->LinkProgram();
    


    
    // Create all the Polygon shapes
    proceduralground = new ProceduralGround(grndSize, 400,
                                     grndOctaves, grndFreq, grndPersistence,
                                     grndLow, grndHigh);
    
    Shape* TeapotPolygons =  new Teapot(fullPolyCount?12:2);
    Shape* BoxPolygons = new Box();
    Shape* SpherePolygons = new Sphere(32);
    Shape* RoomPolygons = new Ply("room.ply");
    Shape* FloorPolygons = new Plane(10.0, 10);
    Shape* QuadPolygons = new Quad();
    Shape* SeaPolygons = new Plane(2000.0, 50);
    Shape* GroundPolygons = proceduralground;

    // Various colors used in the subsequent models
    glm::vec3 woodColor(87.0/255.0, 51.0/255.0, 35.0/255.0);
    glm::vec3 brickColor(134.0/255.0, 60.0/255.0, 56.0/255.0);
    glm::vec3 floorColor(6*16/255.0, 5.5*16/255.0, 3*16/255.0);
    glm::vec3 brassColor(0.5, 0.5, 0.1);
    glm::vec3 grassColor(62.0/255.0, 102.0/255.0, 38.0/255.0);
    glm::vec3 waterColor(0.3, 0.3, 1.0);

    glm::vec3 black(0.0, 0.0, 0.0);
    glm::vec3 brightSpec(0.03, 0.03, 0.03);
    //glm::vec3 brightSpec(0.5, 0.5, 0.5);
    glm::vec3 polishedSpec(0.018, 0.018, 0.018);
    //glm::vec3 polishedSpec(0.3, 0.3, 0.3);

    //light color & brightness
    Light = glm::vec3(3, 3, 3);
    Ambient = glm::vec3(0.1, 0.1, 0.1);

 
    // Creates all the models from which the scene is composed.  Each
    // is created with a polygon shape (possibly NULL), a
    // transformation, and the surface lighting parameters Kd, Ks, and
    // alpha.

    // @@ This is where you could read in all the textures and
    // associate them with the various objects being created in the
    // next dozen lines of code.

    // @@ To change an object's surface parameters (Kd, Ks, or alpha),
    // modify the following lines.
    
    //camera = new Camera(width, height, glm::vec3(tx, ty, -0.1f), spin, tilt, zoom);

    central = new Object(NULL, nullId);
    box1 = new Object(BoxPolygons, boxId, glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), 10);
    box2 = new Object(BoxPolygons, boxId, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), 10);
    anim       = new Object(NULL, nullId);
    //room       = new Object(RoomPolygons, roomId, brickColor, black, 1);
    //floor      = new Object(FloorPolygons, floorId, floorColor, black, 1);
    teapot     = new Object(TeapotPolygons, teapotId, brassColor, brightSpec, 120);
    //podium     = new Object(BoxPolygons, boxId, glm::vec3(woodColor), polishedSpec, 10); 
    sky        = new Object(SpherePolygons, skyId, black, black, 0);
    //ground     = new Object(GroundPolygons, groundId, grassColor, black, 1);
    //sea        = new Object(SeaPolygons, seaId, waterColor, brightSpec, 120);
    //leftFrame  = FramedPicture(Identity, lPicId, BoxPolygons, QuadPolygons);
    //rightFrame = FramedPicture(Identity, rPicId, BoxPolygons, QuadPolygons); 
    spheres    = SphereOfSpheres(SpherePolygons);
    //thirdFrame = FramedPicture(Identity, thirdPicId, BoxPolygons, QuadPolygons);
#ifdef REFL
    spheres->drawMe = true;
#else
    spheres->drawMe = false;
#endif

    //Add texture path to object if it has one
    //sky->texture = new Texture("skys\\Newport_Loft_Ref.hdr");
    //irr = new Texture("skys\\Newport_Loft_Ref.irr.hdr");
    //sea->texture = new Texture("textures\\ripples_normalmap.png");
    //sea->normalMap = new Texture("textures\\ripples_normalmap.png");
    //room->texture = new Texture("textures\\Standard_red_pxr128.png");
    //room->normalMap = new Texture("textures\\Standard_red_pxr128_normal.png");
    //floor->texture = new Texture("textures\\6670-diffuse.jpg");
    //floor->normalMap = new Texture("textures\\6670-normal.jpg");
    //teapot->texture = new Texture("textures\\cracks.png");
    //podium->texture = new Texture("textures\\Brazilian_rosewood_pxr128.png");
    //podium->normalMap = new Texture("textures\\Brazilian_rosewood_pxr128_normal.png");
    //rightFrame->texture = new Texture("textures\\cat.png");
    //thirdFrame->texture = new Texture("textures\\women.png");
    //ground->texture = new Texture("textures\\grass.jpg");


    // @@ To change the scene hierarchy, examine the hierarchy created
    // by the following object->add() calls and adjust as you wish.
    // The objects being manipulated and their polygon shapes are
    // created above here.

    // Scene is composed of sky, ground, sea, room and some central models
    if (fullPolyCount) {
        objectRoot->add(sky, Scale(2000.0, 2000.0, 2000.0));
        //objectRoot->add(sea); 
        //objectRoot->add(ground); 
    }
    objectRoot->add(central);
#ifndef REFL
    //objectRoot->add(room,  Translate(0.0, 0.0, 0.02));
#endif
    //objectRoot->add(floor, Translate(0.0, 0.0, 0.0) * Scale(0.0, 0.0, 2.0));

    // Central model has a rudimentary animation (constant rotation on Z)
    //animated.push_back(anim);

    // Central contains a teapot on a podium and an external sphere of spheres
    //central->add(podium, Translate(0.0, 0.0, 1.5));
    central->add(box1, Translate(0.0, -1.0, 1.5));
    central->add(box2, Translate(0.0, 30.0, 0.0)*Scale(30.0f, 30.0f, 0.5f));
    //central->add(box1, Translate(1.0, 0,0));
    //central->add(box2, Translate(-1.0, 0,0));
    central->add(anim, Translate(0.0, 0,0));
    anim->add(teapot, Translate(0.1, 0.0, 1.5)*TeapotPolygons->modelTr);
    //if (fullPolyCount)
        //anim->add(spheres, Translate(0.0, 0.0, 0.0)*Scale(16, 16, 16));
    
    //Room contains three framed pictures
    //if (fullPolyCount) {
    //    room->add(leftFrame,  Translate(-4.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));
    //    room->add(rightFrame, Translate( 1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8)); 
    //    room->add(thirdFrame, Translate(-1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));
    //}

    CHECKERROR;

    // Options menu stuff
    show_demo_window = false;

    //Create
    fbo.CreateFBO(1000, 1000);
    //top.CreateFBO(1000, 1000);
    //bot.CreateFBO(1000, 1000);
    glActiveTexture(GL_TEXTURE2); // Activate texture unit 2
    glBindTexture(GL_TEXTURE_2D, fbo.textureID); // Load texture into it

    camera = new Camera(width, height, glm::vec3(tx, ty - 25.0f, 0.0f), glm::vec3(spin, tilt, 0.0f));
}

void Scene::DrawMenu()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    float min = 0.1f, max = 100.0f;
    if (ImGui::BeginMainMenuBar()) {
        // This menu demonstrates how to provide the user a list of toggleable settings.
        /*if (ImGui::BeginMenu("IBL")) {
            ImGui::SliderScalar("e", ImGuiDataType_Float, &e, &min, &max, "%f", ImGuiSliderFlags_None);
            ImGui::EndMenu(); }*/

        if (ImGui::BeginMenu("Objects")) {
            if (ImGui::MenuItem("Draw spheres", "", spheres->drawMe))  {spheres->drawMe ^= true; }            
            if (ImGui::MenuItem("Draw walls", "", room->drawMe))       {room->drawMe ^= true; }
            if (ImGui::MenuItem("Draw ground/sea", "", ground->drawMe)){ground->drawMe ^= true;}
            ImGui::EndMenu(); }
                	
        // This menu demonstrates how to provide the user a choice
        // among a set of choices.  The current choice is stored in a
        // variable named "mode" in the application, and sent to the
        // shader to be used as you wish.
        if (ImGui::BeginMenu("Menu ")) {
            if (ImGui::MenuItem("<sample menu of choices>", "",	false, false)) {}
            if (ImGui::MenuItem("Draw Sky", "",		sky->drawMe)) { sky->drawMe ^= true; }
            if (ImGui::MenuItem("Draw Ground", "",  ground->drawMe)) { ground->drawMe ^= true; }
            if (ImGui::MenuItem("Do nothing 2", "",		mode==2)) { mode=2; }
            ImGui::EndMenu(); }
        
        ImGui::EndMainMenuBar(); }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene::BuildTransforms()
{
    // @@ When you are ready to try interactive viewing, replace the
    // following hard coded values for WorldProj and WorldView with
    // transformation matrices calculated from variables such as spin,
    // tilt, tr, ry, front, and back.

    if (transformaiton_mode)
    {
        WorldView = Rotate(2, spin) * Translate((-1.0f) * eye.x, (-1.0f) * eye.y, (-1.0f) * eye.z);
        WorldView = Rotate(0, tilt - 90.0f) * WorldView;
    }
    else
    {
        WorldView = Rotate(0, tilt - 90.0f) * Rotate(2, spin);
        WorldView = Translate(tx, ty, zoom * (-1.0f)) * WorldView;
    }

    rx = ry * width / height;
    WorldProj = Perspective(rx, ry, front, back);
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be
// drawn. (Which is often: 30 to 60 times per second are the common
// goals.)
void Scene::DrawScene()
{
    // Set the viewport
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    CHECKERROR;
    lightPos = glm::vec3(lightDist * cos(lightSpin * rad) * sin(lightTilt * rad),
        lightDist * sin(lightSpin * rad) * sin(lightTilt * rad),
        lightDist * cos(lightTilt * rad));

    // Update position of any continuously animating objects
    double atime = 360.0*glfwGetTime()/36;
    for (std::vector<Object*>::iterator m=animated.begin();  m<animated.end();  m++)
        (*m)->animTr = Rotate(2, atime);

    BuildTransforms();

    // The lighting algorithm needs the inverse of the WorldView matrix
    WorldInverse = glm::inverse(WorldView);
    

    ////////////////////////////////////////////////////////////////////////////////
    // Anatomy of a pass:
    //   Choose a shader  (create the shader in InitializeScene above)
    //   Choose and FBO/Render-Target (if needed; create the FBO in InitializeScene above)
    //   Set the viewport (to the pixel size of the screen or FBO)
    //   Clear the screen.
    //   Set the uniform variables required by the shader
    //   Draw the geometry
    //   Unset the FBO (if one was used)
    //   Unset the shader
    ////////////////////////////////////////////////////////////////////////////////

    CHECKERROR;
    int loc, programId;

    ////////////////////////////////////////////////////////////////////////////////
    // Lighting pass
    ////////////////////////////////////////////////////////////////////////////////
    
    // Choose the lighting shader
    lightingProgram->Use();
    programId = lightingProgram->programId;


    // Set the viewport, and clear the screen
    glViewport(0, 0, width, height);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    camera->drawCam(rx, ry, front, back, programId, "Camera");
    camera->Input(window);
    
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));
    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lightPos[0]));
    loc = glGetUniformLocation(programId, "eyePos");
    glUniform3fv(loc, 1, &(eye[0]));
    loc = glGetUniformLocation(programId, "Light");
    glUniform3fv(loc, 1, &(Light[0]));
    loc = glGetUniformLocation(programId, "Ambient");
    glUniform3fv(loc, 1, &(Ambient[0]));
    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);
    CHECKERROR;


    // Draw all objects (This recursively traverses the object hierarchy.)
    objectRoot->Draw(lightingProgram, Identity);
    CHECKERROR; 
    
    // Turn off the shader
    lightingProgram->Unuse();

    ////////////////////////////////////////////////////////////////////////////////
    // End of Lighting pass
    ////////////////////////////////////////////////////////////////////////////////
}
