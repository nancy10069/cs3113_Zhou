
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#ifdef _WINDOWS
#define LEVEL_HEIGHT 16
#define LEVEL_WIDTH 22
#define SPRITE_COUNT_X 16;
#define SPRITE_COUNT_Y 16;
#define TILE_SIZE 1.0f;
#include <GL/glew.h>
#endif
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "SatCollision.h"
#include "Vector3.h"
#include <vector>
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include <cstring>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif



SDL_Window* displayWindow;
/*
unsigned int levelDat[LEVEL_HEIGHT][LEVEL_WIDTH]{
    //1=ground,3=coin,2=enemy,4=player
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1},   {0,0,3,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
    {1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1},
    {0,0,2,0,0,0,0,0,0,0,4,0,0,0,0,0,0,2,0,0,0,0},    {0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    
};
*/

GLuint LoadTexture(const char* filepath){
    int w,h,comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL){
        std::cout << "Unable to load image. Make sure the path is corret\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}

class SheetSprite{
public:
    SheetSprite(){};
    SheetSprite(unsigned int textureID_, float u_, float v_, float width_, float height_, float
                size_){
        textureID=textureID_;
        u=u_;
        v=v_;
        width=width_;
        height=height_;
        size=size_;
        
    }
    void Draw(ShaderProgram *program) const {
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        float vertices[] = {
            -0.5f , -0.5f,
            0.5f, 0.5f ,
            -0.5f , 0.5f ,
            0.5f, 0.5f ,
            -0.5f, -0.5f  ,
            0.5f , -0.5f };
        
        glUseProgram(program->programID);
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    
    
    
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
};

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN, ENTITY_BACKGROUND};

class Entity{
public:

    Entity(const SheetSprite& sprite, float x, float y, float sizeX, float sizeY, float velocity_x, float velocity_y, float acceleration_x, float acceleration_y, EntityType entityType):
    sprite(sprite), position(x,y,0),
    size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0),//size*sprite
    velocity(velocity_x, velocity_y,0),
    acceleration(acceleration_x, acceleration_y, 0),
    entityType(entityType){};
    
    
    void UpdateX(float elapsed){
        if(entityType != ENTITY_BACKGROUND){

            velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
            
        }
    }
    
    void UpdateY(float elapsed){
        if(entityType != ENTITY_BACKGROUND){// if not static, gravity=-2
         //   acceleration.y = -2.0f;
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            if(position.y <= -1.5f+size.y*0.5){
                position.y = -1.5f+size.y*0.5;
                collideBottom = true;
            }
            
        }
    }
    
    
    void Render(ShaderProgram* program, Entity* player){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        matrix.Identity();
        matrix.Translate(position.x, position.y, position.z);
        Matrix viewMatrix;
        glUseProgram(program->programID);
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(matrix);
        program->SetViewMatrix(viewMatrix);
        sprite.Draw(program);
    }
    
    bool SatCollision(Entity* entity){
        matrix.Identity();
        matrix.Translate(position.x, position.y, position.z);
        
        std::pair<float,float> penetration;
        std::vector<std::pair<float,float>> e1Points;
        std::vector<std::pair<float,float>> e2Points;

        std::vector<Vector3> points;
        points.push_back(Vector3(-0.5f, -0.5f, 0.0f));
        points.push_back(Vector3(0.5f, 0.5f, 0.0f));
        points.push_back(Vector3(-0.5f, 0.5f, 0.0f));
        points.push_back(Vector3(0.5f, -0.5f, 0.0f));
        

        for(int i=0; i < points.size(); i++) {
            Vector3 point = matrix * points[i];
            e1Points.push_back(std::make_pair(point.x, point.y));
        }

        for(int i=0; i < points.size(); i++) {
            Vector3 point = entity->matrix * points[i];
            e2Points.push_back(std::make_pair(point.x, point.y));
        }
    
        bool collided = CheckSATCollision(e1Points, e2Points, penetration);
        
        if(collided){
            position.x += (penetration.first * 0.5f);
            position.y += (penetration.second * 0.5f);
            entity->position.x -= (penetration.first * 0.5f);
            entity->position.y -= (penetration.second * 0.5f);
        }
    
        return collided;
    }
    
    bool CollidesWithX(Entity* entity){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
            }else if(entity->entityType == ENTITY_BACKGROUND){
                double Xpenetration = 0.0f;
                
                Xpenetration = fabs(fabs(position.x-entity->position.x) - size.x*0.5 - entity->size.x*0.5);
                
                if(position.x>entity->position.x){
                    position.x = position.x + Xpenetration + 0.00001f;
                    collideLeft = true;
                }else{
                    position.x = position.x - Xpenetration - 0.000001f;
                    collideRight = true;
                }
                
                velocity.x = 0.0f;
            }
            return true;
        }
    }
    
    bool CollidesWithY(Entity* entity){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_COIN){
                entity->position.x = -2000.0f;
            }else if(entity->entityType == ENTITY_BACKGROUND){
                double Ypenetration = 0.0f;
                
                
                Ypenetration = fabs(fabs(position.y-entity->position.y) - size.y*0.5 - entity->size.y*0.5);
                
                if(position.y>entity->position.y){
                    position.y = position.y + Ypenetration + 0.00001f;
                    collideBottom = true;
                }else{
                    position.y = position.y - Ypenetration - 0.00001f;
                    collideTop = true;
                }
                
                velocity.y = 0.0f;
                
            }
            return true;
        }
    }
    
    SheetSprite sprite;
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    Matrix matrix;
    
    //    bool isStatic;
    
    EntityType entityType;
    
    bool collideTop;
    bool collideBottom;
    bool collideLeft;
    bool collideRight;
};



void processGameInput(SDL_Event* event){
   
}


int main(int argc, char *argv[])
{
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("Space Invader", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
        glewInit();
#endif
        SDL_GL_SwapWindow(displayWindow);
        float lastFrameTicks = 0.0f;
        glClearColor(0.1f, 0.5f, 0.3f, 1);
        glViewport(0, 0, 640, 360);
        ShaderProgram program;
        float accumulator = 0.0f;
        program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
       float overalltime = 0.0;
    
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    Matrix player_M;
    Matrix Enemy_M;
    
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

//    std::vector<Entity> entities;
  //  GLuint tex = LoadTexture(RESOURCE_FOLDER"sheet.png");
//    player.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"front.png"),211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);
  //  enemy.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
    
    
    
    /*for(size_t i =0;i<3;i++){
        Entity enm(-3+i*0.5,0,0.3,0.3);
        enm.isStatic=false;
        enm.sprite =SheetSprite(tex,423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);//enemyblackship
        
        entities.push_back(enm);
    }
    
     for(size_t i =0;i<3;i++){
     Entity coin(-3+i*0.5,1,0.3,0.3); coin.isStatic=false;
     coin.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"coin_gold.png"),211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);
     
     entities.push_back(coin);
     }
     for(size_t i =0;i<5;i++){
     Entity ground(-3+i*0.1,-1,0.3,0.3);
     ground.isStatic=true;
     ground.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"ground_dirt.png"),211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);
     
     entities.push_back(ground);
     }
     */
    
    Entity player(SheetSprite(LoadTexture(RESOURCE_FOLDER"front.png"), 1,1,1,1, 1), 1, -0.5f, 1, 1, 0,0,0, -1, ENTITY_PLAYER);
    Entity enm(SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"), 1,1,1,1, 1), -2, -0.5f,1,1,0,0,0,0, ENTITY_ENEMY);
    Entity ground (SheetSprite(LoadTexture(RESOURCE_FOLDER"ground_dirt.png"), 1,1,1,1, 1), 2, -0.5f, 1,1,0,0,0,0, ENTITY_BACKGROUND);
    
    std::vector<Entity> entities;
    entities={ground,enm};
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    SDL_GL_SwapWindow(displayWindow);
    
    
    
    float elapsed = 0.0f;
    int x=0;
    SDL_Event event;
    bool done = false;bool lose = true;
    while (!done) {
        //while (lose)
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program.programID);
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        program.SetColor(0,0,0, 1);
        //GLint tex =LoadTexture(RESOURCE_FOLDER"font1.png");
        //DrawText(&program2, tex, "PRESS SPACE TO PLAY", 0.2, 0);
        
       // SDL_GL_SwapWindow(displayWindow);

        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        
        
        while(elapsed >= FIXED_TIMESTEP) {
//            Update(elapsed);
            player.collideBottom = false;
            player.collideTop = false;
            player.collideRight = false;
            player.collideLeft = false;
            player.UpdateY(elapsed);
            player.UpdateX(elapsed);
            
            for (Entity entity:entities){
                player.SatCollision(&entity);
            }
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        //render

        player.Render(&program, &player);
        for (Entity entity : entities){
            entity.Render(&program, &player);
        }
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_W){
                    player.velocity.y = 2;
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_S){
                    player.velocity.y = -2;}
                else if(event.key.keysym.scancode == SDL_SCANCODE_D){
                    player.acceleration.x = 2;
                }else if(event.key.keysym.scancode == SDL_SCANCODE_A){
                    player.acceleration.x = -2;
                }else{
                    player.acceleration.x = 0;
                }
                
            }
        }
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
}

