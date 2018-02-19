/*
 
 USE W, S to control the left bar
 use up, down to control the right bar
 
 
 */


#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
SDL_Window* displayWindow;

class Entity{
public:
    Entity(float x,float y,float width,float height){
        this->x = x;
        this->y=y;
        this->width=width;
        this->height=height;
    }
    void Draw(ShaderProgram &program){
        
        float vertices[]={x,y,x+width,y,x+width,y+height,x,y,x+width,y+height,x,y+height};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        /*
        float texCoords[] = {0.0,1.0,1.0,1.0,1.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        */
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisableVertexAttribArray(program.positionAttribute);
//        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
    }
    float x;
    float y;
    float rotation;
    int textureID;
    float width;
    float height;
    float velocity_x;
    float velocity_y;
};

GLuint LoadTexture(const char *filePath){
    int w,h,comp;
    unsigned char* image = stbi_load(filePath,&w, &h, &comp, STBI_rgb_alpha);
    if (image == NULL){
        std::cout<<"unable to load";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1,&retTexture);
    glBindTexture(GL_TEXTURE_2D,retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
      SDL_GL_SwapWindow(displayWindow);
    float lastFrameTicks = 0.0f;
    glClearColor(0.1f, 0.2f, 0.3f, 1);
    glViewport(0, 0, 640, 360);
    ShaderProgram program;
    ShaderProgram program2;
    
    
    
    
    
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    program2.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
/*
    GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"alienBeige_square.png");
    GLuint emojiTexture_round = LoadTexture(RESOURCE_FOLDER"alienBeige_round.png");
    GLuint emojiTexture_suit = LoadTexture(RESOURCE_FOLDER"alienBeige_suit.png");
 */
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    Matrix modelMatrix1;
    
    Matrix leftbar;
    Matrix rightbar;
    
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

    Entity circle(-0.2,0,0.2,0.2);
    Entity bar1(-3.5,0,0.2,1);
    Entity bar2(3,0,0.2,1);
    
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    SDL_GL_SwapWindow(displayWindow);
    
 //   float angle = rand() % 180;
    float angle = 315;
    SDL_Event event;
    bool done = false;bool lose = false;
    while (!done) {
        while (!lose){
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        
        
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program2.programID);
        circle.x +=cos(angle/180*3.14)*elapsed;
        circle.y += sin(angle/180*3.14)*elapsed;
        
        if (circle.y <=-1.9 || circle.y>=2.0){
        //    std::cout<<angle;
            if (circle.y<=1.9){circle.y=-1.89;}
            if (circle.y>=2.0){circle.y=1.99;}
            if (angle<90){angle-=90;}
            else if (angle<180){angle+=90;}
            else if (angle<270){angle-=90;}
            else if (angle<360){angle+=90;}
            if (angle>360){angle-=360;}
            if (angle<0){angle+=360;}
        }
            
        if (circle.x <=-3.3f){
            if (circle.y>=bar1.y && circle.y <=bar1.y+bar1.height){
                circle.x = bar1.x+bar1.width+0.1;
                if (angle<180){angle-=90;}
                else if (angle<270){angle+=90;}
                if (angle>360){angle-=360;}
                if (angle<0){angle+=360;}
            }else{
                SDL_Quit();
            }
        }
        if (circle.x>=2.8f ){
            std::cout<<circle.y<<" "<<bar2.y<<std::endl;
            if(circle.y>=bar2.y && circle.y <=bar2.y+bar2.height){
            circle.x = 2.7f;

            if (angle<90){angle+=90;}
            else if (angle<360){angle-=90;std::cout<<angle;}
            if (angle>360){angle-=360;}
            if (angle<0){angle+=360;}
            }else{
                SDL_Quit();
            }
        }
        
        program2.SetColor(0.5f, 0.8f, 0.6f, 1);
        program2.SetModelMatrix(modelMatrix1);
        program2.SetProjectionMatrix(projectionMatrix);
        program2.SetViewMatrix(viewMatrix);
        circle.Draw(program2);

        
        
        glUseProgram(program2.programID);
        program2.SetModelMatrix(leftbar);
        program2.SetColor(1, 1, 1, 1);
//        leftbar.Translate(0,bar1_translate,0);
        bar1.Draw(program2);

        program2.SetModelMatrix(rightbar);
    //    rightbar.Translate(0,bar2_translate,0);
        bar2.Draw(program2);
        
        
        
        /*
        float vertices1[]={0.5f,-0.5f,0.0f,0.5f,-0.5f,-0.5f};
        glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
        glEnableVertexAttribArray(program2.positionAttribute);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(program2.positionAttribute);

        //--------------square
        
        
        
      //  program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
        glUseProgram(program.programID);
        
        modelMatrix.Identity();
        //modelMatrix.Rotate(elapsed);
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        
        glBindTexture(GL_TEXTURE_2D, emojiTexture);
        float vertices[]={-0.5,-1.5,0.5,-1.5,0.5,-0.5,-0.5,-1.5,0.5,-0.5,-0.5,-0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords[] = {0.0,1.0,1.0,1.0,1.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
       //--------------------circle------------
        
        
        modelMatrix.Identity();
        
        program.SetModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, emojiTexture_round);
        float vertices2[]={-1.5,-0.5,-0.5,-0.5,-0.5,0.5,-1.5,-0.5,-0.5,0.5,-1.5,0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        
        //--------------suit-------------
        modelMatrix.Identity();
        program.SetModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D, emojiTexture_suit);
        float vertices3[]={0.5,-0.5,1.5,-0.5,1.5,0.5,0.5,-0.5,1.5,0.5,0.5,0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
*/
        SDL_GL_SwapWindow(displayWindow);
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if (event.type == SDL_KEYDOWN){
                if (event.key.keysym.scancode == SDL_SCANCODE_S){
                    //bar1_translate-=0.1;
                    //bar1_y-=0.1;
                    bar1.y-=0.1;
                    //down
                }else if(event.key.keysym.scancode == SDL_SCANCODE_W){
//                    bar1_translate+=0.1;
                    bar1.y+=0.1;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_UP){
                    //bar2_translate+=0.1;
                    bar2.y+=0.1;
                }else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN){
                    //bar2_translate-=0.1;
                    bar2.y-=0.1;
                }
            }else if (event.type ==SDL_KEYUP){
                if (event.key.keysym.scancode == SDL_SCANCODE_S){
                    bar1.y=bar1.y;
                    //bar1_translate = 0;
                }if (event.key.keysym.scancode == SDL_SCANCODE_W){
                    bar1.y=bar1.y;

                    //bar1_translate = 0;
                }if (event.key.keysym.scancode == SDL_SCANCODE_UP){
                    bar2.y=bar2.y;

                    //bar2_translate = 0;
                }if (event.key.keysym.scancode == SDL_SCANCODE_DOWN){
                    bar2.y=bar2.y;

                    //bar2_translate = 0;
                }
            }
        }
            glClear(GL_COLOR_BUFFER_BIT);
        }
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
