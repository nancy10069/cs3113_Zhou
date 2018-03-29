/*
 
 USE W, S to control the left bar
 use up, down to control the right bar
 
 
 */
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define LEVEL_HEIGHT 16
#define LEVEL_WIDTH 22
#define SPRITE_COUNT_X 16;
#define SPRITE_COUNT_Y 16;
#define TILE_SIZE 1.0f;
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
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



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
SDL_Window* displayWindow;
unsigned int levelDat[LEVEL_HEIGHT][LEVEL_WIDTH]{
    //1=ground,3=coin,2=enemy,4=player
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1},   {0,0,3,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0},
        {1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1},
        {0,0,2,0,0,0,0,0,0,0,4,0,0,0,0,0,0,2,0,0,0,0},    {0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    
};

class SheetSprite{
public:
    SheetSprite(){};
    SheetSprite(unsigned int textureID,float u,float v,float width,float height,float size){
        this->textureID = textureID;
        this->u=u;
        this->v=v;
        this->size=size;
        this->width=width;
        this->height=height;
    };
    void Draw(ShaderProgram *program){
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLfloat texCoords[]={
            u,v+height,
            u+width,v,
            u,v,
            u+width,v,
            u,v+height,
            u+width,v+height
        };
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        

        float aspect=width/height;
        float vertices[]={
        -0.5f*size*aspect,-0.5f*size,
        0.5f*size*aspect,0.5f*size,
        -0.5f*size*aspect,0.5f*size,
        0.5f*size*aspect,0.5f*size,
        -0.5f*size*aspect,-0.5f*size,
        0.5f*size*aspect,-0.5f*size,
        };
            glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
            glEnableVertexAttribArray(program->positionAttribute);

            glDrawArrays(GL_TRIANGLES,0,6);
            glDisableVertexAttribArray(program->positionAttribute);
            glDisableVertexAttribArray(program->texCoordAttribute);
    };
    float size;
    unsigned int textureID;
    float u;float v;float width;float height;
};

float lerp(float v0,float v1,float t){
    return (1.0-t)*v0+t*v1;
}

enum EntityType{ENTITY_PLAYER,ENTITY_ENEMY,ENTITY_COIN};
class Entity{
public:
    Entity(float x,float y,float width,float height){
        this->x = x;
        this->y=y;
        this->width=width;
        this->height=height;
    }
    void Update(float elapsed){
//        if (elapsed <= 3){        y+=0.1*velocity_y; }

    }
    void collisionY(){
        
    }
    void collisionX(){
       
        
    }
    void Move(float elapsed){//update
        velocity_x = lerp(velocity_x,0.0f,elapsed*friction_x);
        velocity_y = lerp(velocity_y,0.0f,elapsed*friction_y);
        
        velocity_x += acceleration_x*elapsed+gravity_x*elapsed;
        velocity_y += acceleration_y*elapsed +gravity_y*elapsed;
        x+=velocity_x*elapsed;
        collisionX();
        y+=velocity_y*elapsed;
        collisionY();
        if (y<=-3.7f){
            y=3.4f;
        }
    }
    
    void Render(ShaderProgram *program){
        
    }
    bool CollidersWith(const Entity &entity){
        return false;
    }
    void Draw(ShaderProgram &program,Matrix &modelmatrix){
        if (exist){
            glBindTexture(GL_TEXTURE_2D, sprite.textureID);
            float vertices[]={x,y,x+width,y,x+width,y+height,x,y,x+width,y+height,x,y+height};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float texCoords[] = {0.0,1.0,1.0,1.0,1.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        }
        
    }
    //basic
    float x;
    float y;
    float gravity_x=0;
    float gravity_y=-5.5f;
    int textureID;
    float width;
    float height;
    float velocity_x=0;
    float velocity_y=0;
    float acceleration_x=0;
    float acceleration_y=0;
    float friction_x=1.5f;
    float friction_y=1.5f;
    SheetSprite sprite;
    bool isStatic=false;
    EntityType entityType;
    bool onground=false;
    
    
    
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
    

    
    //updated
    /*
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;*/
    
    
    //not needed
    float rotation;
    float timeAlive;
    bool exist=true;
};

void DrawText(ShaderProgram *program,int fontTexture,std::string text,float size,float spacing){
    float texture_size=1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;

    for (int i=0;i<text.size();i++){
        int spriteIndex = (int)text[i];
        float texture_x=(float)(spriteIndex%16)/16.0f;
        float texture_y=(float)(spriteIndex/16)/16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size+spacing)*i)+(-0.5f*size),0.5f*size,
            ((size+spacing)*i)+(-0.5f*size),-0.5f*size,
            ((size+spacing)*i)+(0.5f*size),0.5f*size,
            ((size+spacing)*i)+(0.5f*size),-0.5f*size,
            ((size+spacing)*i)+(0.5f*size),0.5f*size,
            ((size+spacing)*i)+(-0.5f*size),-0.5f*size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x,texture_y,
            texture_x,texture_y+texture_size,
            texture_x+texture_size,texture_y,
            texture_x+texture_size,texture_y+texture_size,
            texture_x+texture_size,texture_y,
            texture_x,texture_y+texture_size,
            
            
        });
    }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    
    for (int i=0;i<text.size()*12;i+=12)
    {
        float texCoord[]={
                texCoordData[i],texCoordData[i+1],texCoordData[i+2],
                texCoordData[i+3],texCoordData[i+4],texCoordData[i+5],
            texCoordData[i+6],texCoordData[i+7],texCoordData[i+8],
            texCoordData[i+9],texCoordData[i+10],texCoordData[i+11]
            };
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        float vertices[]={
            vertexData[i],vertexData[i+1],vertexData[i+2],
            vertexData[i+3],vertexData[i+4],vertexData[i+5],
            vertexData[i+6],vertexData[i+7],vertexData[i+8],
            vertexData[i+9],vertexData[i+10],vertexData[i+11]
        };
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);

    }
}

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

std::vector<Entity> bullets;

void shootBullet(float x,float y){
    Entity newBullet(x,y,0.1f,0.1f);
    newBullet.velocity_y=1;
    bullets.push_back(newBullet);
}
void dropbullet(float x,float y){
    Entity newBullet(x,y,0.1f,0.1f);
    newBullet.velocity_y=-1;
    bullets.push_back(newBullet);

}
bool shouldremovebullet(Entity bullet){
    if (bullet.timeAlive>0.4){
        return true;
    }else{
        return false;
    }
}

enum GameMode{STATE_MAIN_MENU,STATE_GAME_LEVEL};
class GameState{
public:
    GameState();
    Entity player;
    Entity enemies[12];
    Entity bullets[20];
    int score;
};

GameMode mode;

//GameState state;
void Render(){
    switch(mode){
        case STATE_MAIN_MENU:
    //mainmenu;
            break;
        case STATE_GAME_LEVEL:
    //mainlevel;
            break;
    }
}
void Update(float elapsed){
        switch(mode){
        case STATE_MAIN_MENU:
    break;
        case STATE_GAME_LEVEL:
            break;
    }
    
}
void ProcessInput(){
    switch(mode){
        case STATE_MAIN_MENU:
            //processmainmenuinput();
            break;
        case STATE_GAME_LEVEL:
            //processgamelevelinput(state);
            break;
    }
}
void RenderGame(const GameState &game){

}
void UpdateGame(const GameState &game,float elapsed){}
void ProcessInput(const GameState &game){}





/*
void testcollide(Entity &one, Entity &two){
    if (one.y<two.y+two.height && one.y+one.height>two.y
        && one.x < two.x+two.width && one.x+one.width > two.x){
        one.exist=false;
        two.exist=false;
        std::cout<<"collided";
    }
}
*/

//testcollide = static
void testcollide(Entity &one, Entity &two){//two is static
    if (one.y<two.y+two.height && one.y+one.height>two.y
        && one.x < two.x+two.width && one.x+one.width > two.x){
        // check x
/*        if (one.x<two.x && one.x+one.width > two.x){
            one.x = two.x-one.width;
            
        }
        if (one.x>two.x && two.x+two.width > one.x){
            one.x = two.x+one.width;
        }*/
        
        
        //check y
        if (one.y<two.y && one.y+one.height > two.y){
            one.y = two.y-one.height;
        }
        if (one.y>two.y && two.y+two.height > one.y){
            one.y = two.y+one.height;
            one.onground = true;
        }
    }
    if (one.y>two.y+one.height){one.onground=false;}
}
//testcoin
void testcoin(Entity &one, Entity &two){//two is coin
    if (one.y<two.y+two.height && one.y+one.height>two.y
        && one.x < two.x+two.width && one.x+one.width > two.x){
        if (two.entityType==ENTITY_COIN){
            two.exist=false;
        }
        if (two.entityType==ENTITY_ENEMY){
            one.exist=false;
        }
    }
    
}

int main(int argc, char *argv[])
{
    float accumulator = 0.0f;
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
    ShaderProgram program2;
    
    float overalltime = 0.0;
    
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    program2.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;

    Matrix player_M;
    Matrix Enemy_M;
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    std::vector<Entity> entities;
    Entity player(0,-1,0.3,0.3);
    Entity enemy(0,1,0.3,0.3);
    GLuint tex = LoadTexture(RESOURCE_FOLDER"sheet.png");
    player.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"front.png"),211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);
    enemy.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
    
    
    for(size_t i =0;i<3;i++){
        Entity enm(-3+i*0.5,0,0.3,0.3);
        enm.isStatic=false;
            enm.sprite =SheetSprite(tex,423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);//enemyblackship
            
            entities.push_back(enm);
    }
    
    /*
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    SDL_GL_SwapWindow(displayWindow);

    
    float elapsed = 0.0f;
    int x=0;
    SDL_Event event;
    bool done = false;bool lose = true;
    while (!done) {
        if (lose){
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(program2.programID);
            program2.SetModelMatrix(modelMatrix);
            program2.SetProjectionMatrix(projectionMatrix);
            program2.SetViewMatrix(viewMatrix);
            program2.SetColor(0,0,0, 1);
            GLint tex =LoadTexture(RESOURCE_FOLDER"font1.png");
            DrawText(&program2, tex, "PRESS SPACE TO PLAY", 0.2, 0);
            if (x!=-2){
            x-=2;
                modelMatrix.Translate(-2, 0, 0);}

            SDL_GL_SwapWindow(displayWindow);

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }
                else if (event.type == SDL_KEYUP){
                    if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){

                        //load new game
                        x=0;
                        lose=false;
                        entities.clear();

                        
                        std::vector<float>vertexData;
                        std::vector<float>texCoordData;
                        float tile_size=0.3f;
                        for (int y=0;y<LEVEL_HEIGHT;y++){
                            for(int x=0;x<LEVEL_WIDTH;x++){
                                if (levelDat[y][x] == 0){
                                    //this is empty
                                }
                                if (levelDat[y][x] == 1){
                                    //this is ground
                                    Entity enm(tile_size*x,tile_size*y,tile_size,tile_size);
                                    enm.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"ground_dirt.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
                                    enm.isStatic=true;
                                    entities.push_back(enm);
                                }
                                if (levelDat[y][x] == 2){
                                    //this is coin
 
                                    Entity enm(tile_size*x,tile_size*y,tile_size,tile_size);
                         enm.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"coin_gold.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
                                    enm.isStatic=false;
                                    enm.acceleration_y=0;enm.gravity_y=0;
                                    enm.entityType=ENTITY_COIN;
                                    entities.push_back(enm);
                                }
                                if (levelDat[y][x] == 3){
                                    //this is enemy
  
                                    Entity enm(tile_size*x,tile_size*y,tile_size,tile_size);
                                  enm.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
                                    enm.velocity_x=1;
                                    enm.entityType=ENTITY_ENEMY;
                                    entities.push_back(enm);
                                }
                                if (levelDat[y][x] == 4){
                                    //this is player
                                    player.x = tile_size*x;
                                    player.y=tile_size*y;
                                    player.velocity_x=0;
                                    player.exist=true;
                                    player.x=0;
                                    player.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"front.png"),211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);
                                    
                                }

                            }
                        }
                        
                        
                        /*
                        for(size_t i =0;i<1;i++){
                                Entity enm(-3+i*0.5,0,0.3,0.3);
                                enm.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
                                enm.velocity_x=1;
                            enm.entityType=ENTITY_ENEMY;
                                entities.push_back(enm);
                        }
                        for(size_t i =0;i<50;i++){
                            Entity enm(-3.7+i*0.3,-1.5,0.3,0.3);
                            enm.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"ground_dirt.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
                            enm.isStatic=true;
                            entities.push_back(enm);
                        }
                        for(size_t i =0;i<3;i++){
                            Entity enm(-1+i*0.5,-0.5,0.3,0.3);
                            enm.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"coin_gold.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
                            enm.isStatic=false;
                            enm.acceleration_y=0;enm.gravity_y=0;
                            enm.entityType=ENTITY_COIN;
                            entities.push_back(enm);
                        }*/
                        overalltime=0;
                      
                        SDL_GL_SwapWindow(displayWindow);
    
                    }}}}
        else{
            
            
            
            
            
            float ticks = (float)SDL_GetTicks()/1000.0f;
            float elapsed = ticks - lastFrameTicks;
            elapsed += accumulator;
            if (elapsed < FIXED_TIMESTEP){
                accumulator=elapsed;
                continue;
            }
            while (elapsed>=FIXED_TIMESTEP){
                Update(FIXED_TIMESTEP);
                elapsed-=FIXED_TIMESTEP;
            }
            accumulator=elapsed;

                        lastFrameTicks = ticks;
            overalltime+=elapsed;
            
            
            
            // HERE IS WHAT UPDATED-----
            
            
            
            
            
            
/*            bool all_dead=false;
            for (Entity ett:entities){
                if (ett.exist){
                    all_dead=true;
                }
            }*/
            
            //or the player is dead
            
            if (!player.exist){
                lose=true;
            }
            glClear(GL_COLOR_BUFFER_BIT);
            
            
            //RENDERING THE PLAYER
            
            program2.SetColor(0.5f, 0.8f, 0.6f, 1);
            program2.SetModelMatrix(player_M);
            program2.SetProjectionMatrix(projectionMatrix);
            program2.SetViewMatrix(viewMatrix);
            player.Draw(program2,player_M);
            player.Move(elapsed);
            viewMatrix.Identity();
            viewMatrix.Translate(-player.x, -player.y, 0);
            
            for (int i=0;i<entities.size();i++){
                if (entities[i].isStatic){
                testcollide(player, entities[i]);
                }
                else{
                testcoin(player, entities[i]);
                }
                if (entities[i].entityType==ENTITY_ENEMY){
                    if (player.x>entities[i].x){
                    entities[i].velocity_x=0.5f;
                    }else{
                        entities[i].velocity_x=-0.5f;
                        
                    }                }
            }
            
            
            //RENDERING THE ENEMIES
            program2.SetModelMatrix(Enemy_M);
            for(int i=0;i<=entities.size()-1;i++){
                //entities[i].x+=elapsed*0.1;
                entities[i].Draw(program2, Enemy_M);
                if (!entities[i].isStatic){
                entities[i].Move(elapsed);
                    for (int ji=0;ji<entities.size();ji++){
                        if (entities[ji].isStatic){
                            testcollide(entities[i], entities[ji]);
                        }
                    }
                    
                    
                }
            }
            
            
            SDL_GL_SwapWindow(displayWindow);
            
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }else if (event.type == SDL_KEYDOWN){
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                        player.velocity_x+=0.5f;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                        player.velocity_x-=0.5f;
                    }
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP){
                        if (player.onground){
                        player.velocity_y+=5.5f;
                        }
                    }if (event.key.keysym.scancode == SDL_SCANCODE_DOWN){
                        if (!player.onground){
                            player.velocity_y-=5.5f;
                        }
                    }
                    
                
                
            }
            
            
            
        }
    glClear(GL_COLOR_BUFFER_BIT);
  //  SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
    }
    

