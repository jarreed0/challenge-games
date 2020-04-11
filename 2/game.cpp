#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <vector>

#define WIDTH 400
#define HEIGHT 650
#define FONT_SIZE 24

#define GRAVITY 6
#define SPEED 4

bool running;
SDL_Renderer* renderer;
SDL_Window* window;
TTF_Font *font;
SDL_Color fcolor;
int frameCount, timerFPS, lastFrame, fps;
std::vector<SDL_Texture*> textures;
int step;

struct object {
 SDL_Rect dest, src;
 int img;
};

struct collectable {
 object obj;
 int type;
};

struct ship {
 object obj;
 int fired;
 int frame;
};

struct laser {
 object lo, ro, laser;
 int fired;
};

object pod;
object tp;
ship tship;
laser tlaser;
collectable tc;
std::vector<collectable> c;
std::vector<ship> ships;
std::vector<object> platforms;
std::vector<laser> lasers;
SDL_Rect life, bkg;
int lifeColor;

bool left, right;

void gen() {
    int r = rand() % 34;
    if(r==0) { //ship
        tship.obj.dest.x=rand() % WIDTH-tship.obj.dest.w;
        if(tship.obj.dest.x<0)tship.obj.dest.x=0;
        ships.push_back(tship);
    } else if(r==1) { //laser
        platforms.push_back(tp);
    } else if(r==2) { //platform left
        tp.src.x=10;
        tp.src.w=20;
        tp.dest.w=rand() % WIDTH/2 + 44;
        tp.dest.x=0;
        platforms.push_back(tp);
    } else if(r==3) { //platform right
        tp.src.x=0;
        tp.src.w=20;
        tp.dest.w=rand() % WIDTH/2 + 44;
        tp.dest.x=WIDTH-tp.dest.w;
        platforms.push_back(tp);
    } else if(r==4) { //platform middle
        tp.src.x=0;
        tp.src.w=30;
        tp.dest.w=rand() % WIDTH/3 + 88;
        tp.dest.x=(WIDTH-tp.dest.w)/2;
        platforms.push_back(tp);
    } else if(r==5) { //o
        tc.type=0;
        tc.obj.src.x=0;
        tc.obj.dest.x=rand() % WIDTH-tc.obj.dest.w;
        if(tc.obj.dest.x<0)tc.obj.dest.x=0;
        c.push_back(tc);
    } else if(r==6) { //b
        tc.type=1;
        tc.obj.src.x=22;
        tc.obj.dest.x=rand() % WIDTH-tc.obj.dest.w;
        if(tc.obj.dest.x<0)tc.obj.dest.x=0;
        c.push_back(tc);
    } else if(r==7) { //p
        tc.type=2;
        tc.obj.src.x=44;
        tc.obj.dest.x=rand() % WIDTH-tc.obj.dest.w;
        if(tc.obj.dest.x<0)tc.obj.dest.x=0;
        c.push_back(tc);
    }
}

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


void update() {
    life.x=pod.dest.x+10;
    life.y=pod.dest.y+15;
    //pod.dest.y+=GRAVITY;
    step+=GRAVITY;
    if(step>=66) {gen();step=0;}
    if(left) pod.dest.x-=SPEED;
    if(right) pod.dest.x+=SPEED;
    for(int i=0;i<ships.size();i++) {
        ships[i].obj.dest.y-=GRAVITY*1.5;
        if(ships[i].obj.dest.y+ships[i].obj.dest.h<0) ships.erase(ships.begin()+i);
    }
    for(int i=0;i<c.size();i++) {
        if(c[i].type==0)c[i].obj.dest.y-=GRAVITY;
        if(c[i].type==1)c[i].obj.dest.y-=GRAVITY/2;
        if(c[i].type==2)c[i].obj.dest.y-=GRAVITY*1.5;
        if(c[i].obj.dest.y+c[i].obj.dest.h<0) c.erase(c.begin()+i);
    }
    for(int i=0;i<platforms.size();i++) {
        platforms[i].dest.y-=GRAVITY/2;
        if(platforms[i].dest.y+platforms[i].dest.h<0) platforms.erase(platforms.begin()+i);
    }
    for(int i=0;i<lasers.size();i++) {
        lasers[i].lo.dest.y-=GRAVITY/2;
        lasers[i].ro.dest.y-=GRAVITY/2;
        lasers[i].laser.dest.y-=GRAVITY/2;
        if(lasers[i].ro.dest.y+lasers[i].ro.dest.h<0) lasers.erase(lasers.begin()+i);
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 15, 15, 18, 255);
    SDL_RenderFillRect(renderer,&bkg);

    frameCount++;
    int timerFPS = SDL_GetTicks()-lastFrame;
    if(timerFPS<(1000/60)) {
        SDL_Delay((1000/60)-timerFPS);
    }

    for(int i=0;i<platforms.size();i++) draw(platforms[i]);
    for(int i=0;i<lasers.size();i++) {draw(lasers[i].lo);draw(lasers[i].ro);draw(lasers[i].laser);}
    for(int i=0;i<ships.size();i++) draw(ships[i].obj);
    for(int i=0;i<c.size();i++) draw(c[i].obj);

    SDL_SetRenderDrawColor(renderer, 59+lifeColor, 13+lifeColor, 49+lifeColor, 255);
    SDL_RenderFillRect(renderer,&life);
    draw(pod);

    SDL_RenderPresent(renderer);

}
void input() {
    SDL_Event e;
    const Uint8 *ks = SDL_GetKeyboardState(NULL);
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) running=false;
    }
    if(ks[SDL_SCANCODE_ESCAPE]) running=0;
    left=right=0;
    if(ks[SDL_SCANCODE_RIGHT] || ks[SDL_SCANCODE_D]) right=1;
    if(ks[SDL_SCANCODE_LEFT] || ks[SDL_SCANCODE_A]) left=1;
}

void init() {
    font = TTF_OpenFont("res/Peepo.ttf", FONT_SIZE);
    pod.src.x=pod.src.y=0;
    pod.src.w=44;pod.src.h=50;
    pod.dest.w=44*1.5;pod.dest.h=50*1.5;
    pod.dest.x=WIDTH/2-(pod.dest.w/2);pod.dest.y=HEIGHT/3-(pod.dest.h/2);
    pod.img=loadImage("res/pod.png");
    bkg.x=bkg.y=0;
    bkg.w=WIDTH;
    bkg.h=HEIGHT;
    life.w=life.h=45;
    step=0;
    tship.obj.img=loadImage("res/ship.png");
    tship.obj.src.x=tship.obj.src.y=0;
    tship.obj.src.w=tship.obj.dest.w=55;
    tship.obj.src.h=tship.obj.dest.h=50;
    tship.obj.dest.y=HEIGHT;
    tc.obj.img=loadImage("res/c.png");
    tc.obj.src.x=tc.obj.src.y=0;
    tc.obj.src.w=tc.obj.src.h=22;
    tc.obj.dest.y=HEIGHT;
    tc.obj.dest.w=tc.obj.dest.h=22;
    tlaser.lo.img=loadImage("res/laser.png");
    tlaser.ro.img=tlaser.laser.img=tlaser.lo.img;
    tlaser.lo.dest.y=tlaser.ro.dest.y=tlaser.laser.dest.y=HEIGHT;
    tlaser.lo.src.x=tlaser.lo.src.y=0;
    tlaser.lo.src.w=tlaser.laser.src.w=tlaser.ro.src.w=44;
    tlaser.ro.src.x=220-44;tlaser.ro.src.y=0;
    tlaser.laser.src.x=88;tlaser.laser.src.y=0;
    tlaser.lo.dest.w=tlaser.lo.dest.h=tlaser.ro.dest.w=tlaser.ro.dest.h=tlaser.laser.dest.w=tlaser.laser.dest.h=22;
    tlaser.lo.dest.x=0;tlaser.ro.dest.x=WIDTH-22;tlaser.laser.dest.x=22;
    tlaser.laser.dest.w=WIDTH-44;
    tp.img=loadImage("res/platform.png");
    tp.dest.y=HEIGHT;
    tp.dest.h=22;
    tp.src.y=0;tp.src.h=12;
}

int main() {
    srand(time(NULL));
    running=1;
    static int lastTime=0;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) std::cout << "Failed at SDL_Init()" << std::endl;
    if(SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0) std::cout << "Failed at SDL_CreateWindowAndRenderer()" << std::endl;
    SDL_SetWindowTitle(window, "Challenge 2 - Two Hours");
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
