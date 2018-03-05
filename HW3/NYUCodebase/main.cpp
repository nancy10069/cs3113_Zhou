/*
 
 USE W, S to control the left bar
 use up, down to control the right bar
 
 
 */

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

class Entity{
public:
    Entity(float x,float y,float width,float height){
        this->x = x;
        this->y=y;
        this->width=width;
        this->height=height;
    }
    void Update(float elapsed){//if bullet
        if (elapsed <= 3){        y+=0.1*velocity_y;        }

    }

    
    void Draw(ShaderProgram &program,Matrix &modelmatrix){
        if (exist){
   //         sprite.Draw(&program);
            
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
    float x;
    float y;
    float rotation;
    int textureID;
    float width;
    float height;
    float velocity_x;
    float velocity_y;
    SheetSprite sprite;
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






void testcollide(Entity &one, Entity &two){
    if (one.y<two.y+two.height && one.y+one.height>two.y
        && one.x < two.x+two.width && one.x+one.width > two.x){
        one.exist=false;
        two.exist=false;
        std::cout<<"collided";
    }
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
    player.velocity_x=3;
    Entity enemy(0,1,0.3,0.3);
    GLuint tex = LoadTexture(RESOURCE_FOLDER"sheet.png");
    player.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"playerShip1_blue.png"),211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);
    enemy.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"enemyBlack1.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
    
//    player.sprite=SheetSprite(tex,211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);//playership
//    enemy.sprite=SheetSprite(tex,423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);//enemyship
    
    for(size_t i =0;i<20;i++){
        if (i<10){
        Entity enm(-3+i*0.5,0,0.3,0.3);
            enm.sprite =SheetSprite(tex,423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);//enemyblackship
            
            entities.push_back(enm);
        }
        if (i>=10&&i<20){
            Entity enm(-3+(i-10)*0.5,1,0.3,0.3);
            enm.sprite =SheetSprite(tex,423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);//enemyredship
            entities.push_back(enm);
        
        }
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    SDL_GL_SwapWindow(displayWindow);

    
    
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
            DrawText(&program2, tex, "PRESS SPACE TO SHOOT", 0.2, 0);
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
                        for(size_t i =0;i<20;i++){
                            if (i<10){
                                
                                Entity enm(-3+i*0.5,0,0.3,0.3);
                                enm.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"enemyBlack1.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);
                                entities.push_back(enm);
                            }
                            if (i>=10&&i<20){
                                Entity enm(-3+(i-10)*0.5,1,0.3,0.3);
                                enm.sprite =SheetSprite(LoadTexture(RESOURCE_FOLDER"enemyGreen1.png"),423.0f/1024.0f,728.0f/1024.0f,93.0f/1024.0f,84.0f/1024.0f,0.3);//enemyblackship
                                entities.push_back(enm);
                            }
                        }
                        overalltime=0;
                        //player = Entity(0,-1,0.3,0.3);
                        player.exist=true;
                        player.x=0;
                        player.sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"playerShip1_blue.png"),211.0f/1024.0f,941.0f/1024.0f,99.0f/1024.0f,75.0f/1024.0f,0.3f);
                        
                        bullets.clear();
                        SDL_GL_SwapWindow(displayWindow);
                    }}}}
        else{
            float ticks = (float)SDL_GetTicks()/1000.0f;
            float elapsed = ticks - lastFrameTicks;
            lastFrameTicks = ticks;
            
            overalltime+=elapsed;
            //test if all of the ships are dead
            bool all_dead=false;
            for (Entity ett:entities){
                if (ett.exist){
                    all_dead=true;
                }
            }
            //or the player is dead
            if (!all_dead||!player.exist){
                lose=true;
            }
            glClear(GL_COLOR_BUFFER_BIT);
            
            
            program2.SetColor(0.5f, 0.8f, 0.6f, 1);
            program2.SetModelMatrix(player_M);
            program2.SetProjectionMatrix(projectionMatrix);
            program2.SetViewMatrix(viewMatrix);
            player.Draw(program2,player_M);
            //        player_M.Translate(player.x, player.y, 0);
            
            program2.SetModelMatrix(Enemy_M);
            for(int i=0;i<=entities.size()-1;i++){
                //         entities[i].Update_enemy(elapsed,Enemy_M);
                entities[i].x+=elapsed*0.1;
                entities[i].Draw(program2, Enemy_M);

            }
            if ((int)overalltime % 2 <=0.05){
                
              /*  bool first_row_all_dead=false;
                for (int i=0;i<entities.size()/2;i++){
                    if (entities[i].exist){first_row_all_dead=true;}
                }
                int anumber;
                if (first_row_all_dead){
                    anumber =0 + (rand() % static_cast<int>(10));
                }
                else{
                    anumber =10 + (rand() % static_cast<int>(10));
                }overalltime+=0.5;
                            std::cout<<anumber<<std::endl;
          //      std::cout<<entities[anumber].x<<std::endl;
                dropbullet(entities[anumber].x, entities[anumber].y);
                */
                
//the front spaceship will shoot
                int i=0;
 
            while (i<=entities.size()-1){
                if (entities[i].exist){
                    dropbullet(entities[i].x, entities[i].y);
                    overalltime+=0.5;
                    i=100;
                }
                i++;
            }
            }
            
            
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), shouldremovebullet),bullets.end());
            
            for (int i=0;i<bullets.size();i++){
                bullets[i].Update(elapsed);
                bullets[i].Draw(program2,player_M);
                testcollide(bullets[i], player);//player will die
                for(int j=0;j<=entities.size()-1;j++){
                    if (bullets[i].exist && entities[j].exist){
                        testcollide(bullets[i], entities[j]);
                    }
                }
            }
            SDL_GL_SwapWindow(displayWindow);
            
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    done = true;
                }else if (event.type == SDL_KEYDOWN){
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                        //                   player_M.Translate(elapsed*player.x, 0, 0);
                        player.x+=player.velocity_x*0.1;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                        //                    player_M.Translate((-1)*elapsed*player.x, 0, 0);
                        player.x-=player.velocity_x*0.1;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                        
                        shootBullet(player.x, player.y+0.2);
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
    

