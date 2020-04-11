#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <vector>

#define WIDTH 700
#define HEIGHT 380
#define FONT_SIZE 24
#define GRAVITY 8
#define UP 6
#define SPEED 4

bool running, flight;
SDL_Renderer* renderer;
SDL_Window* window;
TTF_Font *font;
SDL_Color fcolor;
int frameCount, timerFPS, lastFrame, fps;
std::vector<SDL_Texture*> textures;
int distance, coinCount;

struct object {
 SDL_Rect dest, src;
 int img;
};

std::vector<object> coins;

object player, ground, coin, boost;
bool boosted;
int boostCount;

int loadImage(std::string filename) {
  SDL_Texture* tex;

  SDL_Surface* surf;
  if(IMG_Load(filename.c_str()) < 0) {
    std::cout << "Failed at IMG_Load()" << IMG_GetError() << std::endl;
  } else {
    surf= IMG_Load(filename.c_str());
  }

  tex = IMG_LoadTexture(renderer, filename.c_str());
  textures.push_back(tex);
  return textures.size()-1;
}


void write(std::string text, int x, int y, int r, int g, int b) {
 SDL_Surface *surface;
 SDL_Texture *texture;
 if (font == NULL) {
  fprintf(stderr, "error: font not found\n");
  exit(EXIT_FAILURE);
 }
 fcolor.r = r;
 fcolor.g = g;
 fcolor.b = b;
 SDL_Rect rect;
 const char* t = text.c_str();
 surface = TTF_RenderText_Solid(font, t, fcolor);
 texture = SDL_CreateTextureFromSurface(renderer, surface);
 rect.x = x;
 rect.y = y;
 rect.w = surface->w;
 rect.h = surface->h;
 SDL_FreeSurface(surface);
 SDL_RenderCopy(renderer, texture, NULL, &rect);
 SDL_DestroyTexture(texture);
}

void draw(object o) {
    SDL_Rect dest = o.dest;
    SDL_Rect src = o.src;
    SDL_RenderCopyEx(renderer, textures[o.img], &src, &dest, 0, NULL, SDL_FLIP_NONE);
}

bool touching(object a, object b) {
  if ((a.dest.x < (b.dest.x + b.dest.w)) && ((a.dest.x + a.dest.w) > b.dest.x) &&
   (a.dest.y < (b.dest.y + b.dest.h)) && ((a.dest.y + a.dest.h) > b.dest.y)) {
    return true;
  }
  else {
    return false;
  }
}

void genCoins() {
    int s = rand() % 12 + 4;
    int l = rand() % 46 + 12;
    int r = rand() % 6;
    for(int i=0; i<s; i++) {
      coin.dest.x=(i*(32*2))+WIDTH;
      coin.dest.y=l*sin(coin.dest.x)+120;
      coins.push_back(coin);
    }
    if(r==0) {
      boost.dest.x=coins[coins.size()-1].dest.x+64;//distance+((s+1)*(32*2))+WIDTH;
      boost.dest.y=coins[coins.size()-1].dest.y;//l*sin(boost.dest.x)+WIDTH+120;
    }
}

void update() {
    if(flight && player.dest.y>0) { player.dest.y-=UP;
    } else if(!touching(player, ground)) {player.dest.y+=GRAVITY;}
    distance+=SPEED;
    if(coins.size()<=0)genCoins();
    for(int i=0;i<coins.size();i++){
      if(touching(player,coins[i])){
        coinCount++;coins.erase(coins.begin()+i);
      }
      coins[i].dest.x-=SPEED; 
      if(boosted) {coins[i].dest.x-=SPEED; }
      if(coins[i].dest.x+coins[i].dest.w<0) coins.erase(coins.begin()+i);
    }
    boost.dest.x-=SPEED;
    if(boosted)boost.dest.x-=SPEED;
    if(touching(player,boost)) {
      boosted=1;
      boostCount+=600;
      boost.dest.x=-100;
    }
    if(boosted) {
      boostCount--;
      std::cout << boostCount << std::endl;
      if(boostCount<=0) boosted=0;
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 27, 27, 30, 255);
    SDL_Rect rect;
    rect.x=rect.y=0;
    rect.w=WIDTH;
    rect.h=HEIGHT;
    SDL_RenderFillRect(renderer,&rect);

    frameCount++;
    int timerFPS = SDL_GetTicks()-lastFrame;
    if(timerFPS<(1000/60)) {
        SDL_Delay((1000/60)-timerFPS);
    }

    for(int i=0;i<coins.size();i++)draw(coins[i]);
    draw(boost);
    draw(player);
    draw(ground);
    std::string score = "Distance: " + std::to_string(distance) + " Coins: " + std::to_string(coinCount);
    if(boosted) score=score + " BOOST!";
    write(score, 3, 3, 30, 30, 200);

    SDL_RenderPresent(renderer);

}
void input() {
    SDL_Event e;
    const Uint8 *ks = SDL_GetKeyboardState(NULL);
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) running=false;
    }
    flight=0;
    if(ks[SDL_SCANCODE_SPACE]) flight=1;
    if(ks[SDL_SCANCODE_ESCAPE]) running=0;
}

void init() {
    font = TTF_OpenFont("res/Peepo.ttf", FONT_SIZE);
    player.src.x=player.src.y=0;
    player.src.w=player.src.h=16;
    player.dest.x=WIDTH*.3;player.dest.y=HEIGHT-(32*2);
    player.dest.w=32;player.dest.h=32;
    player.img=loadImage("res/player.png");
    ground.src.x=ground.src.y=0;
    ground.dest.x=0;ground.dest.y=HEIGHT-32;
    ground.src.w=ground.src.h=16;
    ground.dest.w=WIDTH;ground.dest.h=32;
    ground.img=loadImage("res/ground.png");
    coinCount=distance=0;
    coin.src.x=coin.src.y=0;
    coin.src.w=coin.src.h=16;
    coin.dest.w=coin.dest.h=32;
    coin.img=loadImage("res/coin.png");
    boost.src.x=boost.src.y=0;
    boost.src.w=boost.src.h=16;
    boost.dest.w=boost.dest.h=32;
    boost.img=loadImage("res/boost.png");
}

int main() {
    srand(time(NULL));
    running=1;
    boosted=1;
    boostCount=600;
    static int lastTime=0;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) std::cout << "Failed at SDL_Init()" << std::endl;
    if(SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) std::cout << "Failed at SDL_CreateWindowAndRenderer()" << std::endl;
    SDL_SetWindowTitle(window, "Challenge 1 - One Hour");
    TTF_Init();

    init();
    while(running) {
        lastFrame=SDL_GetTicks();
        if(lastFrame>=(lastFrame+1000)) {
            lastTime=lastFrame;
            fps=frameCount;
            frameCount=0;
        }

        update();
        input();
        render();
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}
