#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"

#include<SDL_mixer.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

// 60 FPS (1.0f/60.0f) (update sixty times a second)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

SDL_Window* displayWindow;

int score=0;int hp=100;
bool unread = true;
bool hurt=false;
float hurt_time=0;
int mission = 0;
void mission_complete(){
    hp=100;
    mission++;
    unread=true;
}
Mix_Chunk *someSound;
std::vector<std::string> missions = {"Your school is occupied and you are surrounded. Meet your team at Library for further actions. Watch out for slimes.","Your team8 is badly injured. Go buy a Cheese burger for him. Watch Out for slimes.","Escape from this place. Watch out, enemies are going to attack you."};
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_LEVEL_2,STATE_GAME_LEVEL_3,STATE_GAME_OVER, STATE_WIN};


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
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size):textureID(textureID), u(u), v(v), width(width), height(height), size(size){ }
    
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


class Vector3{
public:
    Vector3(float x, float y, float z):x(x), y(y), z(z){};
    
    float x;
    float y;
    float z;
};
float Distance(const Vector3& point1,const Vector3& point2)
{
    float distance = sqrt( (point1.x - point2.x) * (point1.x - point2.x) +
                          (point1.y - point2.y) * (point1.y - point2.y) +
                          (point1.z - point2.z) * (point1.z - point2.z) );
    return distance;
}
float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_BULLET,ENTITY_COIN, ENTITY_STATIC,ENTITY_OBSTACLE};

class Entity{
public:
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType){
        
    
        
    };
    
    
    void DrawText(ShaderProgram *program,int fontTexture,std::string text,float size,float spacing,float x,float y){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-14.2f, 14.2f, -8.0f, 8.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        Matrix viewMatrix;
        glUseProgram(program->programID);
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        modelMatrix.Translate(-4, 0, 0);
        float texture_size=1.0/16.0f;
        std::vector<float> vertexData;
        std::vector<float> texCoordData;
        int row=0;
        int line =20;
        for (int i=0;i<text.size();i++){
            
            int spriteIndex = (int)text[i];
            float texture_x=(float)(spriteIndex%16)/16.0f;
            float texture_y=(float)(spriteIndex/16)/16.0f;
            if (((size+spacing)*i)+(-0.5f*size)-row*line > line){
                y-=1;
                row++;
            }
            vertexData.insert(vertexData.end(), {
                (x+(size+spacing)*i)+(-0.5f*size)-row*line,0.5f*size+y,
                (x+(size+spacing)*i)+(-0.5f*size)-row*line,-0.5f*size+y,
                (x+(size+spacing)*i)+(0.5f*size)-row*line,0.5f*size+y,
                (x+(size+spacing)*i)+(0.5f*size)-row*line,-0.5f*size+y,
                (x+(size+spacing)*i)+(0.5f*size)-row*line,0.5f*size+y,
                (x+(size+spacing)*i)+(-0.5f*size)-row*line,-0.5f*size+y,
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
    };
    void UpdateX(float elapsed){
        if(entityType == ENTITY_BULLET ||entityType==ENTITY_ENEMY){
            if(elapsed<3){ position.x += velocity.x *elapsed;}
        }
        if(entityType == ENTITY_PLAYER){
    //        const Uint8 *keys = SDL_GetKeyboardState(NULL);

            velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
            
            if(position.x <= -2.0f+size.x*0.5){
                position.x = -2.0f+size.x*0.5;
                collideLeft = true;
            }
            if(position.x >= 70.0f+size.x*0.5){
                position.x = 70.0f+size.x*0.5;
                collideRight = true;
            }
            
        }
        
    }
    
    void UpdateY(float elapsed){
        if(entityType == ENTITY_BULLET ||entityType==ENTITY_ENEMY){
            if (elapsed<3){position.y += velocity.y *elapsed;}
        }
        if(entityType == ENTITY_PLAYER){
           // const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            velocity.y = lerp(velocity.y, 0.0f, elapsed * 1.5f);
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            /*
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            */
            if(position.y <= -4.0f+size.y*0.5){
                position.y = -4.0f+size.y*0.5;
                collideBottom = true;
            }
            if(position.y >= 30.0f+size.y*0.5){
                position.y = 30.0f+size.y*0.5;
                collideTop = true;
            }
            
        }
    }
    
    void Render(ShaderProgram* program, Entity* player){

        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-14.2f, 14.2f, -8.0f, 8.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        modelMatrix.Translate(position.x, position.y, position.z);
        modelMatrix.Scale(size.x, size.y, size.z);
        
        Matrix viewMatrix;

        viewMatrix.Translate(-1.0f*player->position.x, -1.0f*player->position.y, 0.0f);

        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        if (entityType==ENTITY_PLAYER){
            int direction =face;
            if (moving){
                
                if (moving_index>=4){moving_index=0;}
                if (direction == 1){
                    if (moving_index==0){
                        sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0,0,0.33f,0.25f, 1);}
                    if (moving_index==1 || moving_index==3){
                        sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0,0.33f,0.25f, 1);}
                    if (moving_index==2){
                        sprite =SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.66f,0,0.33f,0.25f, 1);
                    }
                    
                }if (direction == 3){
                    if (moving_index==0){
                        sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0,0.25f,0.33f,0.25f, 1);
                    }if (moving_index==1||moving_index==3){
                        sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0.25f,0.33f,0.25f, 1);}
                    if (moving_index==2){
                        sprite =SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.66f,0.25f,0.33f,0.25f, 1);
                    }
                }
                if (direction==4){
                    if (moving_index==0){
                        sprite=SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0,0.5f,0.33f,0.25f, 1);
                    }if (moving_index==1||moving_index==3){
                        sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0.5f,0.33f,0.25f, 1);}
                    if (moving_index==2){
                        sprite =SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.66f,0.5f,0.33f,0.25f, 1);
                    }
                }
                if (direction ==2){
                    if (moving_index==0){
                        sprite=SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0,0.75f,0.33f,0.25f, 1);
                    }
                    if (moving_index==1||moving_index==3){
                        sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0.75f,0.33f,0.25f, 1);}
                    if (moving_index==2){
                        sprite =SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.66f,0.75f,0.33f,0.25f, 1);
                    }
                }
            }else{
                if (direction == 1){
                    sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0,0.33f,0.25f, 1);}
                if (direction == 3){
                    sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0.25f,0.33f,0.25f, 1);}
                if (direction==4){
                    sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0.5f,0.33f,0.25f, 1);}
                if (direction ==2){
                    sprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0.33f,0.75f,0.33f,0.25f, 1);}
            }
        }
        sprite.Draw(program);
    }
    bool moving = false;int moving_index=0;
    void Render_without_moving(ShaderProgram* program, Entity* player){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-14.2f, 14.2f, -8.0f, 8.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        modelMatrix.Scale(size.x, size.y, size.z);
        
        Matrix viewMatrix;
        
      //  viewMatrix.Translate(-1.0f*player->position.x, -1.0f*player->position.y, 0.0f);
        
        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        sprite.Draw(program);
    }
    
  void RedShader(ShaderProgram* program){
        
    }
    bool CollidesWithX(Entity* entity,GameMode gamemode){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_ENEMY){
                bool s = false;
                if (gamemode==STATE_GAME_LEVEL && entity->enm_index==1){
                    s=true;
                }
                if (gamemode==STATE_GAME_LEVEL_2 && entity->enm_index==2){
                    s=true;
                }if (gamemode==STATE_GAME_LEVEL_3 && entity->enm_index==3){
                    s=true;
                }
                if (s){
                    hp-=10;
                    Mix_PlayChannel(-1, someSound, 0);
                    
                if (entity->position.x < position.x){
                    entity->position.x-=1;position.x+=1;
                }else{
                    entity->position.x+=1;position.x-=1;
                }
                //shader, become red
                hurt=true;
                }
                
            }
            if(entity->entityType == ENTITY_COIN){
                if (entity->coin_index == mission+1){
                    
                    entity->position.x = -2000.0f;
                    mission_complete();
                }
            }else if(entity->entityType == ENTITY_STATIC){
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
    
    bool CollidesWithY(Entity* entity,GameMode gamemode){
        
        if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
            return false;
        }else{
            if(entity->entityType == ENTITY_ENEMY){
                
                bool s = false;
                if (gamemode==STATE_GAME_LEVEL && entity->enm_index==1){
                    s=true;
                }
                if (gamemode==STATE_GAME_LEVEL_2 && entity->enm_index==2){
                    s=true;
                }if (gamemode==STATE_GAME_LEVEL_3 && entity->enm_index==3){
                    s=true;
                }
                if (s){
                    
                hp-=10;
                if (entity->position.y < position.y){
                    entity->position.y-=1;position.y+=1;
                }else{
                    entity->position.y+=1;position.y-=1;}
                //shader, become red
                }
            }
            if(entity->entityType == ENTITY_COIN){
                if (entity->coin_index == mission+1){
                    entity->position.x = -2000.0f;
                    mission_complete();
                }

            }else if(entity->entityType == ENTITY_STATIC){
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

    void move_towards(Vector3 destination){
        int speed =1;
        if (destination.y>position.y){
            velocity.y=speed;
        }
        else{velocity.y=-speed;}
        if (destination.x>position.x){
            velocity.x=speed;
        }
        else{velocity.x=-speed;}
    }
    void enm_logic(Entity* player){
        if (enm_index==1){
            
        }
        if (enm_index == 2){
            move_towards(trails[current_trail]);
            if (Distance(position, trails[current_trail])<1){
                current_trail++;
                if (current_trail==trails.size()){
                    current_trail=0;
                }
            }
        }
        if (enm_index==3){
        if (state==0){
            move_towards(trails[current_trail]);
            if (Distance(position, trails[current_trail])<1){
                current_trail++;
                if (current_trail==trails.size()){
                    current_trail=0;
                }
            }
        }
        if (state==1){
            move_towards(player->position);
        }
        }
        
    }
    std::vector<Vector3> trails;
    int current_trail=0;
    int state = 0;// 0 = 巡逻； 1= 跟踪player；
    
    SheetSprite sprite;
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    
    
    EntityType entityType;
    
    bool collideTop;
    bool collideBottom;
    bool collideLeft;
    bool collideRight;
    
    int face=1;//1,2,3,4 = 前后左右
    bool exist=true;
    
    int coin_index;
    int enm_index;
};

void shootBullet(float x,float y,int face_index,std::vector<Entity*> &bullets){
    Entity newBullet(SheetSprite(LoadTexture(RESOURCE_FOLDER"hurt.png"), 0,0,1,1, 1), x,y, 0.3,0.3,0,0,0,0,ENTITY_BULLET);
    if (face_index==1){newBullet.velocity.y=-0.1;}
    if (face_index==2){newBullet.velocity.y=0.1;}
    if (face_index==3){newBullet.velocity.x=-0.1;}
    if (face_index==4){newBullet.velocity.x=0.1;}
    bullets.push_back(&newBullet);}

void setup(ShaderProgram* program){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Violet College", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
//    glClearColor(254.0f/255.0f, 223.0f/255.0f, 225.0f/255.0f, 1.0f);
    
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void processGameInput(SDL_Event* event, bool& done, Entity* player,std::vector<Entity*>& bullets){

}
void info (ShaderProgram* program,Entity* player){
    Entity star_board (SheetSprite(LoadTexture(RESOURCE_FOLDER"board.jpg"), 0,0,1,1, 1), -5,0, 30,20,0,0,0,0, ENTITY_OBSTACLE);
    star_board.Render_without_moving(program, player);
    GLint tex =LoadTexture(RESOURCE_FOLDER"font1.png");
    player->DrawText(program, tex, "Score: "+std::to_string(score), 2,-1,-12,5);
    player->DrawText(program, tex, "Hp: "+std::to_string(hp), 2,-1,-12,3);
    player->DrawText(program, tex, "Mission "+std::to_string(mission+1)+":", 2, -1, -12, 1);
    player->DrawText(program, tex, missions[mission], 2, -1, -12, -1);
}
void updateGame(float elapsed, Entity* player, std::vector<Entity*> entities,std::vector<Entity*>& bullets, Entity* coin,ShaderProgram* program,GameMode gamemode ){
    player->collideBottom = false;
    player->collideTop = false;
    player->collideRight = false;
    player->collideLeft = false;
    
    player->UpdateY(elapsed);
    for (Entity* entityPtr : entities){
        if (entityPtr->exist){
        player->CollidesWithY(entityPtr,gamemode);
            
            if(entityPtr->entityType==ENTITY_ENEMY){
                entityPtr->enm_logic(player);
                entityPtr->UpdateX(elapsed);
                entityPtr->UpdateY(elapsed);
                
                if (Distance(entityPtr->position, player->position)<5){
                    entityPtr->state=1;
                }
                if (Distance(entityPtr->position, player->position)>8){
                    entityPtr->state=0;
                }
            
            }}
    }
    player->CollidesWithY(coin,gamemode);
    
    player->UpdateX(elapsed);
    for (Entity* entityPtr : entities){
        if (entityPtr->exist){
            player->CollidesWithX(entityPtr,gamemode);
            
        }
    }
    
    for (Entity* entityPtr : bullets){
            entityPtr->UpdateX(elapsed);
            entityPtr->UpdateY(elapsed);
        for (Entity* enmPtr : entities){
            if (enmPtr->entityType==ENTITY_ENEMY && enmPtr->exist){
                //击中怪物
                if (enmPtr->CollidesWithX(entityPtr,gamemode)||enmPtr->CollidesWithY(entityPtr,gamemode)){
                    Mix_PlayChannel(-1, someSound, 0);
                    score+=10;
                    enmPtr->exist=false;
                    entityPtr->exist=false;
                }
            }
        }
    }
    player->CollidesWithX(coin,gamemode);
    
}

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
    float retVal = dstMin + ((value - srcMin)/(srcMax-srcMin) * (dstMax-dstMin));
    if(retVal < dstMin) {
        retVal = dstMin;
    }
    if(retVal > dstMax) {
        retVal = dstMax;
    }
    return retVal;
}

void DrawPicture(ShaderProgram* program,Entity*player,Vector3 position,Vector3 size,SheetSprite sprite){
    
    glBindTexture(GL_TEXTURE_2D, sprite.textureID);
    float vertices[]={position.x,position.y,position.x+size.x,position.y,position.x+size.x,position.y+size.y,position.x,position.y,position.x+size.x,position.y+size.y,position.x,position.y+size.y};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    float texCoords[] = {0.0,1.0,1.0,1.0,1.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0};
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES,0,6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

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

void animate(float animationTime,ShaderProgram*program,Entity*player){
    
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-14.2f, 14.2f, -8.0f, 8.0f, -1.0f, 1.0f);
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    glUseProgram(program->programID);
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    
    float animationValue = mapValue(animationTime, 0,100,0.0, 2.0);
    
    DrawPicture(program, player, Vector3(-30,-20+lerp(0, 20, animationValue),0), Vector3(94,45,0), (SheetSprite(LoadTexture(RESOURCE_FOLDER"violet_map.png"), 0,0,25,12, 1)));
 //   std::cout<<animationTime<<std::endl;
  //  modelMatrix.Identity();
//    modelMatrix.Translate(100,100,0);
    //    Title.Translate(1.5,lerp(0, 10, animationValue), 0.0);
    
}
void renderGame(ShaderProgram* program, Entity* player, std::vector<Entity*> entities, Entity* coin,std::vector<Entity*>& bullets,GameMode gamemode,float animationtime){
    if (gamemode == STATE_GAME_OVER){
        GLint tex =LoadTexture(RESOURCE_FOLDER"font1.png");
        player->DrawText(program, tex, "You Have Been Slain", 2,-1,-10,-2);
    }
    if (gamemode == STATE_WIN){
        GLint tex =LoadTexture(RESOURCE_FOLDER"font1.png");
        player->DrawText(program, tex, "You Win", 2,-1,-10,0);
        player->DrawText(program, tex, "Score:"+std::to_string(score), 2,-1,-10,-2);
    }
    if (gamemode == STATE_MAIN_MENU){
        
        animate(animationtime, program, player);
        GLint tex =LoadTexture(RESOURCE_FOLDER"font1.png");
        player->DrawText(program, tex, "VIOLET EVERCOLLEGE", 2, -1,-10,0);
        player->DrawText(program, tex, "Press Space To Start", 1, -0.5,-10,-2);
    }
    if (gamemode == STATE_GAME_LEVEL||gamemode == STATE_GAME_LEVEL_2||gamemode == STATE_GAME_LEVEL_3){
        
    for (Entity* entityPtr : entities){
        if (entityPtr->exist&& entityPtr->entityType!=ENTITY_ENEMY){
             entityPtr->Render(program, player);
        }
        if (entityPtr->entityType==ENTITY_ENEMY && entityPtr->exist){
                    if (gamemode == STATE_GAME_LEVEL && entityPtr->enm_index==1){
                         entityPtr->Render(program, player);
                    }else  if (gamemode == STATE_GAME_LEVEL_2 && entityPtr->enm_index==2){
                        entityPtr->Render(program, player);
                        
                    }else  if (gamemode == STATE_GAME_LEVEL_3 && entityPtr->enm_index==3){
                        entityPtr->Render(program, player);
                        
                    }
            
        }
    }for (Entity* entityPtr : bullets){
        if (entityPtr->exist){
        entityPtr->Render(program, player);
        }}
        
        player->Render(program, player);
        coin->Render(program, player);
        
        
        
        if (hurt || hp<=50){
            
            Matrix projectionMatrix;
            projectionMatrix.SetOrthoProjection(-14.2f, 14.2f, -8.0f, 8.0f, -1.0f, 1.0f);
            Matrix modelMatrix;
            Matrix viewMatrix;
            
            glUseProgram(program->programID);
            program->SetProjectionMatrix(projectionMatrix);
            program->SetModelMatrix(modelMatrix);
            program->SetViewMatrix(viewMatrix);
            DrawPicture(program, player, Vector3(-14.2,-8,0), Vector3(28.4,16,0), SheetSprite(LoadTexture(RESOURCE_FOLDER"hurt.png"), 0,0,1,1, 1));
        }
        
        
        //show notification
        if (unread){
            Entity star_board (SheetSprite(LoadTexture(RESOURCE_FOLDER"board.jpg"), 0,0,1,1, 1), -5,0, 30,20,0,0,0,0, ENTITY_OBSTACLE);
            star_board.Render_without_moving(program, player);
            GLint tex =LoadTexture(RESOURCE_FOLDER"font1.png");
            player->DrawText(program, tex, "NEW mission!! ", 2,-1,-12,5);
            player->DrawText(program, tex,missions[mission], 1.5f,-1,-12,3);
        }
        
        
        }
    
}

void cleanup(){
    SDL_Quit();
}
int main(int argc, char *argv[])
{
    ShaderProgram program;
    float lastFrameTicks = 0.0f;
    float accumulator = 0.0f;
    
    Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,4096);
    
    someSound=Mix_LoadWAV("someSound.wav");
    Mix_Music *music;
    music=Mix_LoadMUS("music.mp3");
    setup(&program);
    
    GLuint itemSpriteSheet = LoadTexture(RESOURCE_FOLDER"items_spritesheet.png");
    
    SheetSprite itemSheet = SheetSprite(itemSpriteSheet, 288.0f/1024.0f, 432.0f/1024.0f, 70.0f/1024.0f, 70.0f/1024.0f, 0.2);

    SheetSprite player_front =SheetSprite(LoadTexture(RESOURCE_FOLDER"violet.png"), 0,0,0.33f,0.25f, 1);
    Entity player(player_front, 70, -2, 1.2f,1.8f, 0,0,0, 0, ENTITY_PLAYER);
    
    Entity coin(SheetSprite(LoadTexture(RESOURCE_FOLDER"violet_2.png"), 0.33,0,0.33f,0.25f, 1), 40, 26, 1.2f, 1.8f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);//this is the one in the library
    coin.coin_index=1;
    Entity coin_2(itemSheet, 45, 1, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);//five guys
    coin_2.coin_index=2;
    Entity coin_3(itemSheet,1, 20, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ENTITY_COIN);//bus stop
    coin_3.coin_index=3;
    Entity ground (SheetSprite(LoadTexture(RESOURCE_FOLDER"violet_map.png"), 227/2500,0,1,1, 1), 37,17, 93.75,45,0,0,0,0, ENTITY_OBSTACLE);
    
    Entity basic(SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"), 0,0,1,1, 1), 60,10,1,1,0,0,0,0, ENTITY_ENEMY);
    basic.enm_index=1;//not moving
    Entity basic_2(SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"), 0,0,1,1, 1), 50,20,1,1,0,0,0,0, ENTITY_ENEMY);
    basic_2.enm_index=1;//not moving
    Entity basic_3(SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"), 0,0,1,1, 1), 40,25,1,1,0,0,0,0, ENTITY_ENEMY);
    basic_3.enm_index=1;//not moving
    Entity basic_4(SheetSprite(LoadTexture(RESOURCE_FOLDER"slime_normal.png"), 0,0,1,1, 1), 66,5,1,1,0,0,0,0, ENTITY_ENEMY);
    basic.enm_index=1;//not moving

    Entity middle(SheetSprite(LoadTexture(RESOURCE_FOLDER"slimeGreen_walk.png"), 0,0,1,1, 1), 40,0,1,1,0,0,0,0, ENTITY_ENEMY);
    middle.enm_index=2;// moving, not attacking
    middle.trails={Vector3(30,10,0),Vector3(45,0,0),Vector3(50,20,0)};
    
    Entity middle_2(SheetSprite(LoadTexture(RESOURCE_FOLDER"slimeGreen_walk.png"), 0,0,1,1, 1), 30,20,1,1,0,0,0,0, ENTITY_ENEMY);
    middle_2.enm_index=2;// moving, not attacking
    middle_2.trails={Vector3(40,10,0),Vector3(25,25,0),Vector3(35,24,0)};
    
    Entity middle_3(SheetSprite(LoadTexture(RESOURCE_FOLDER"slimeGreen_walk.png"), 0,0,1,1, 1), 45,10,1,1,0,0,0,0, ENTITY_ENEMY);
    middle_3.enm_index=2;// moving, not attacking
    middle_3.trails={Vector3(50,10,0),Vector3(15,15,0),Vector3(30,15,0)};
    
    
    
    Entity slime(SheetSprite(LoadTexture(RESOURCE_FOLDER"slimeBlue_blue.png"), 0,0,1,1, 1), 0,0,1,1,0,0,0,0, ENTITY_ENEMY);
    slime.trails={Vector3(10,0,0),Vector3(5,5,0),Vector3(5,10,0)};
    slime.enm_index=3;
    
    
    Entity slime_2(SheetSprite(LoadTexture(RESOURCE_FOLDER"slimeBlue_blue.png"), 0,0,1,1, 1), 10,2,1,1,0,0,0,0, ENTITY_ENEMY);
    slime.trails={Vector3(0,10,0),Vector3(20,-2,0),Vector3(0,0,0)};
    slime.enm_index=3;
    
    Entity slime_3(SheetSprite(LoadTexture(RESOURCE_FOLDER"slimeBlue_blue.png"), 0,0,1,1, 1), 0,20,1,1,0,0,0,0, ENTITY_ENEMY);
    slime.trails={Vector3(20,0,0),Vector3(30,5,0),Vector3(40,-2,0)};
    slime.enm_index=3;
    Entity slime_4(SheetSprite(LoadTexture(RESOURCE_FOLDER"slimeBlue_blue.png"), 0,0,1,1, 1), 20,-2,1,1,0,0,0,0, ENTITY_ENEMY);
    slime.trails={Vector3(30,-2,0),Vector3(20,5,0),Vector3(10,5,0)};
    slime.enm_index=3;
    
    
    
    
    
    
    
    
    
    
    
    
    
    Entity dog(SheetSprite(LoadTexture(RESOURCE_FOLDER"dog.png"), 0,0,1,1, 1), 1857/26.67,260/26.67, 74/26.67,42/26.67,0,0,0,0,ENTITY_STATIC);

    Entity grass_2 (SheetSprite(LoadTexture(RESOURCE_FOLDER"grass_2.png"), 0,0,1,1, 1),
                    56,445/26.67, 693/26.67,286/26.67,0,0,0,0, ENTITY_OBSTACLE);
    Entity chipotle(SheetSprite(LoadTexture(RESOURCE_FOLDER"chipotle.png"), 0,0,1,1, 1),
                    18.634,-2.8, 255/26.67,41/26.67,0,0,0,0, ENTITY_STATIC);
    Entity crocodile(SheetSprite(LoadTexture(RESOURCE_FOLDER"crocodile.png"), 0,0,1,1, 1), 1422/26.67,170/26.67,86/26.67,41/26.67,0,0,0,0, ENTITY_STATIC);
    Entity grass(SheetSprite(LoadTexture(RESOURCE_FOLDER"grass.png"), 0,0,1,1, 1), 1204/26.67,170/26.67, 330/26.67,41/26.67,0,0,0,0,ENTITY_STATIC);
    Entity grass_3(SheetSprite(LoadTexture(RESOURCE_FOLDER"grass.png"), 0,0,1,1, 1), 1650/26.67,170/26.67, 330/26.67,41/26.67,0,0,0,0,ENTITY_STATIC);
    
    Entity library(SheetSprite(LoadTexture(RESOURCE_FOLDER"library.png"), 0,0,1,1, 1),
                   53,854/26.67, 760/26.67,286/26.67,0,0,0,0, ENTITY_STATIC);
    Entity main_building_2(SheetSprite(LoadTexture(RESOURCE_FOLDER"main_building_2.png"), 0,0,1,1, 1), 18.3738,23.5035, 336/26.67,525/26.67,0,0,0,0, ENTITY_STATIC);
    Entity main_building(SheetSprite(LoadTexture(RESOURCE_FOLDER"main_building.png"), 0,0,1,1, 1), 7.09052,27.5456, 281/26.67,304/26.67,0,0,0,0, ENTITY_STATIC);
    Entity road(SheetSprite(LoadTexture(RESOURCE_FOLDER"road.png"), 0,0,1,1, 1), 0,0,
                -226/26.67,0/26.67,0,0,0,0, ENTITY_STATIC);
    Entity sculpture(SheetSprite(LoadTexture(RESOURCE_FOLDER"sculpture.png"), 0,0,1,1, 1),10,20.3666, 108/26.67,144/26.67,0,0,0,0, ENTITY_STATIC);
    Entity six_guys(SheetSprite(LoadTexture(RESOURCE_FOLDER"six_guys.png"), 0,0,1,1, 1), 1340/26.67,-70/26.67, 212/26.67,67/26.67,0,0,0,0, ENTITY_STATIC);
    Entity chair(SheetSprite(LoadTexture(RESOURCE_FOLDER"chair.png"), 0,0,1,1, 1), 1635/26.67,500/26.67, 38/26.67,38/26.67,0,0,0,0, ENTITY_STATIC);

    Entity chair_5(SheetSprite(LoadTexture(RESOURCE_FOLDER"chair.png"), 0,0,1,1, 1), 1535/26.67,500/26.67, 38/26.67,38/26.67,0,0,0,0, ENTITY_STATIC);
    Entity chair_2(SheetSprite(LoadTexture(RESOURCE_FOLDER"chair.png"), 0,0,1,1, 1), 1335/26.67,350/26.67, 38/26.67,38/26.67,0,0,0,0, ENTITY_STATIC);
    Entity chair_3(SheetSprite(LoadTexture(RESOURCE_FOLDER"chair.png"), 0,0,1,1, 1), 1735/26.67,400/26.67, 38/26.67,38/26.67,0,0,0,0, ENTITY_STATIC);
    Entity chair_4(SheetSprite(LoadTexture(RESOURCE_FOLDER"chair.png"), 0,0,1,1, 1), 1535/26.67,3350/26.67, 38/26.67,38/26.67,0,0,0,0, ENTITY_STATIC);

    
    std::vector<Entity*> bullets={};
    std::vector<Entity*> entities;
    entities={&ground,&grass,&chipotle,&crocodile,&dog,&library,&main_building,&main_building_2,&road,&sculpture,&six_guys,&grass_2,&grass_3,&chair,&chair_2,&chair_3,&chair_4,&chair_5,
        &slime,&coin_2,&coin_3,&coin,&basic,&middle,
        &basic_3,&basic_2,&middle_2,&middle_3,&basic_4,&slime_2,&slime_3,&slime_4
    };
    bool tab_open=false;
    GameMode gamemode = STATE_MAIN_MENU;
    SDL_Event event;
    bool done = false;
    float animationTime=0;
    float every_second=0;
    
    Mix_PlayMusic(music, -1);
    Mix_VolumeMusic(30);
    while (!done) {
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if (hurt){
            hurt_time+=elapsed;
        }
        if (hurt_time>1){
            hurt=false;
        }

        animationTime = animationTime + elapsed;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        processGameInput(&event, done, &player,bullets);
        
        glClear(GL_COLOR_BUFFER_BIT);
        while(elapsed >= FIXED_TIMESTEP) {
            
            if (gamemode!=STATE_MAIN_MENU && gamemode!=STATE_GAME_OVER && gamemode !=STATE_WIN){
                updateGame(FIXED_TIMESTEP, &player, entities, bullets,&coin,&program,gamemode);}
            elapsed -= FIXED_TIMESTEP;
        }
        
        accumulator = elapsed;
        
            if (mission==1){
                gamemode=STATE_GAME_LEVEL_2;
            }
            if (mission==2){
                gamemode=STATE_GAME_LEVEL_3;
            }
            if (mission==3){
                gamemode=STATE_WIN;
            }
            
        
        
        renderGame(&program, &player, entities, &coin,bullets,gamemode,animationTime);
        if (hp<=0){
            gamemode=STATE_GAME_OVER;
        }
        
        every_second+=FIXED_TIMESTEP;
        if (every_second>=0.2f){
            every_second=0;
            player.moving_index+=1;
        }
        if (tab_open){
            info(&program,&player);
        }
        
        //switching mode
        
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                    SDL_Quit();
                }else if(event.key.keysym.scancode == SDL_SCANCODE_RETURN){
                        if(gamemode==STATE_MAIN_MENU){gamemode = STATE_GAME_LEVEL;}
                        if (gamemode==STATE_GAME_OVER){gamemode=STATE_MAIN_MENU;}
                        
                    }else if(event.key.keysym.scancode == SDL_SCANCODE_F){
                    if (gamemode==STATE_GAME_LEVEL||gamemode == STATE_GAME_LEVEL_2||gamemode == STATE_GAME_LEVEL_3){
                        Entity newBullet(SheetSprite(LoadTexture(RESOURCE_FOLDER"hurt.png"), 0,0,1,1, 1), player.position.x,player.position.y, 0.3,0.3,0,0,0,0,ENTITY_BULLET);
                    int face_index=player.face;
                    float speed = 5;
                    if (face_index==1){newBullet.velocity.y=-speed;}
                    if (face_index==2){newBullet.velocity.y=speed;}
                    if (face_index==3){newBullet.velocity.x=-speed;}
                    if (face_index==4){newBullet.velocity.x=speed;}
                        bullets.push_back(&newBullet);

                    }
                    }else if(event.key.keysym.scancode == SDL_SCANCODE_TAB){
                        tab_open=true;
                    }
                
                if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                    player.acceleration.x = 7.5f;
                    player.face=4; player.moving=true;
                }else if(event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                    player.face=3; player.moving=true;
                    player.acceleration.x = -7.5f;
                }else{
                    player.acceleration.x = 0.0f;
                }
                
                if(event.key.keysym.scancode == SDL_SCANCODE_UP){
                    player.acceleration.y = 7.5f; player.moving=true;
                    player.face = 2;
                }else if(event.key.keysym.scancode == SDL_SCANCODE_DOWN){
                    player.acceleration.y = -7.5f;
                    player.face=1; player.moving=true;
                }else{
                    player.acceleration.y = 0.0f;
                }
            }else if(event.type == SDL_KEYUP){
                if(event.key.keysym.scancode == SDL_SCANCODE_TAB){
                    tab_open=false;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    unread=false;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                    player.acceleration.x = 0;player.moving=false;
                }else if(event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                    player.acceleration.x = 0;player.moving=false;
                }
                
                if(event.key.keysym.scancode == SDL_SCANCODE_UP){
                    player.acceleration.y = 0;player.moving=false;
                }else if(event.key.keysym.scancode == SDL_SCANCODE_DOWN){
                    player.acceleration.y = 0;player.moving=false;
                }
                
            }
            
        }
        SDL_GL_SwapWindow(displayWindow);
    }
    
    cleanup();
    return 0;
}
