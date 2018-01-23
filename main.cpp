#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <string>
#include <sstream>
#include <math.h>
#include <fstream>
#include <iostream>
using namespace std;

const int shipSpeed = 4;
const int windowWidth = 1000;
const int windowHeght = 700;

SDL_Surface* window = NULL;
SDL_Surface* background = NULL;
SDL_Surface* ship = NULL;
SDL_Surface* rock = NULL;
SDL_Surface* metal = NULL;
SDL_Surface* shock = NULL;
SDL_Surface* target = NULL;
SDL_Surface* boom = NULL;
SDL_Surface* battery = NULL;
SDL_Surface* bar = NULL;
SDL_Surface* shields[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
SDL_Surface* timerSurface = NULL;
SDL_Rect shieldPos;
SDL_Rect shipPos;
SDL_Rect targetPos;
SDL_Event inputEvent;
TTF_Font* timerFont;
Mix_Chunk* zap = NULL;
Mix_Chunk* charge = NULL;
Mix_Chunk* discharge = NULL;
Mix_Chunk* explosion = NULL;
Mix_Chunk* rocket = NULL;
Mix_Music* spaceMusic = NULL;
int shieldTimer = 0;
int mousePosX;
int mousePosY;
int powerLevel = 0;
bool running = true;
bool moveLeft = false;
bool moveRight = false;
bool click = false;

class Asteroid {
public:
    SDL_Rect clip;
    SDL_Rect shockClip;
    SDL_Rect pos;
    int shockTimer;
    int xSpeed;
    int ySpeed;
    int size;
    bool isMetal; // true is metal
    bool isCharged;
    bool showShock;
    void show_asteroids()
    {
        if (isMetal) // if the aasteroid is metal
        {
            if (SDL_GetTicks() - shockTimer > 500) // if the sock should change
            {
                shockTimer = SDL_GetTicks(); // reset them clock
                showShock = !showShock; // change shock
            }
            if (isCharged) {
                if (showShock) // if the shock should show
                {
                    SDL_BlitSurface(shock, &shockClip, window,
                        new SDL_Rect(pos)); // show shock
                }
            }
            SDL_BlitSurface(metal, &clip, window,
                new SDL_Rect(pos)); // show metal
        }
        else {
            SDL_BlitSurface(rock, &clip, window,
                new SDL_Rect(pos)); // show rock if not metal
        }
    }
    void move() // chnge the location
    {
        pos.x += xSpeed;
        pos.y += ySpeed;
    }
    void reset()
    {
        isCharged = true;
        size = rand() % 4;
        pos.w = 200; // set colision dimensions
        pos.h = 200;
        clip.x = size * 200; // dicides size
        clip.y = (rand() % 4) * 200; // decides variety
        pos.x = rand() % windowWidth; // randomizes x location
        pos.y = (-200); // sets y to above window
        isMetal = rand() % 2; // decides metal or not(preformance warning)
        shockTimer = 0; // resets shock timer
        xSpeed = (rand() % 10) - 5; // the motion on the x(-5 for left or righT)
        ySpeed = rand() % 10; // the amount it moves on the x axis
        if ((xSpeed == 0) && (ySpeed == 0)) // if randomization went wrong
        {
            reset();
        }
    }
    Asteroid()
    {
        shockClip.w = 200;
        shockClip.h = 200;
        shockClip.y = 0;
        shockClip.x = 0;
        clip.w = 200; // set clip dimensions
        clip.h = 200;
        isCharged = true;
        reset(); // all others
    }
} asteroids[10];

SDL_Surface* load_pic(string file, bool key)
{
    SDL_Surface* optomized_image_load_fxn = NULL;
    SDL_Surface* loaded_image_load_fxn = IMG_Load(file.c_str()); // load the
    // image
    key = true;
    if (key) // if the image needs to be color keyed
    {
        optomized_image_load_fxn = SDL_DisplayFormat(
            loaded_image_load_fxn); // convert the image to 32 bitmap
        Uint32 color_key_load_fxn = SDL_MapRGB(optomized_image_load_fxn->format, 0, 0,
            0); // set color key to green in 32bitmap format
        SDL_SetColorKey(optomized_image_load_fxn, SDL_SRCCOLORKEY,
            color_key_load_fxn); // preform the color key action
    }
    else {
        optomized_image_load_fxn = SDL_DisplayFormatAlpha(
            loaded_image_load_fxn); // convert the image to 32 bitmap with alpha
    }
    SDL_FreeSurface(loaded_image_load_fxn); // free middle step load surface
    return optomized_image_load_fxn; // return the final image
} // used to load images in the format of the screen with co

void init_systems()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);
}

void load_files_startup()
{
    background = load_pic("space.png", true);
    ship = load_pic("ship.png", true);
    rock = load_pic("rock.png", true);
    metal = load_pic("metal.png", true);
    shock = load_pic("shock.png", true);
    target = load_pic("target.png", true);
    boom = load_pic("boom.png", true);
    battery = load_pic("battery.png", true);
    bar = load_pic("bar.png", true);
    shields[0] = load_pic("red.png", true);
    shields[1] = load_pic("orange.png", true);
    shields[2] = load_pic("yellow.png", true);
    shields[3] = load_pic("green.png", true);
    shields[4] = load_pic("blue.png", true);
    shields[5] = load_pic("purple.png", true);
    timerFont = TTF_OpenFont("chintzy.ttf", 44);
    zap = Mix_LoadWAV("zap.wav");
    charge = Mix_LoadWAV("charge.wav");
    discharge = Mix_LoadWAV("discharge.wav");
    explosion = Mix_LoadWAV("explosion.wav");
    rocket = Mix_LoadWAV("rocket.wav");
    spaceMusic = Mix_LoadMUS("space_mus.wav");
}

void set_values_startup()
{
    shipPos.x = (windowWidth - ship->w) / 2;
    shipPos.y = windowHeght - ship->h;
    shipPos.w = 150;
    shipPos.h = 150;
    targetPos.w = 50;
    targetPos.h = 50;
}

void start_up()
{
    init_systems();
    window = SDL_SetVideoMode(windowWidth, windowHeght, 32, SDL_SWSURFACE);
    load_files_startup();
    set_values_startup();
}

void shut_down()
{
    SDL_FreeSurface(ship);
    SDL_FreeSurface(background);
    SDL_FreeSurface(rock);
    SDL_FreeSurface(metal);
    SDL_FreeSurface(shock);
    SDL_Quit();
}

double check_colide(SDL_Rect a, SDL_Rect b)
{
    int x, y;
    x = (a.x + (a.w / 2)) - (b.x + (b.w / 2)); // finds the center of both
    y = (a.y + (a.h / 2)) - (b.y + (b.h / 2));
    return sqrt((x *= x) + (y *= y)); // returns there distance apart
}

void events_fxn()
{
    while (SDL_PollEvent(&inputEvent)) {
        if (inputEvent.type == SDL_QUIT) {
            running = false;
        }
        else if (inputEvent.type == SDL_KEYDOWN) // if the user pushed a button
        {
            switch (inputEvent.key.keysym.sym) {
            case SDLK_a:
                moveLeft = true;
                break;
            case SDLK_d:
                moveRight = true;
                break;
            default:
                break;
            }
        }
        else if (inputEvent.type == SDL_KEYUP) {
            switch (inputEvent.key.keysym.sym) {
            case SDLK_a:
                moveLeft = false;
                break;
            case SDLK_d:
                moveRight = false;
                break;
            default:
                break;
            }
        }
        else if (inputEvent.type == SDL_MOUSEMOTION) {
            mousePosX = inputEvent.motion.x;
            mousePosY = inputEvent.motion.y;
        }
        else if (inputEvent.type == SDL_MOUSEBUTTONDOWN) {
            if (inputEvent.button.button == SDL_BUTTON_LEFT) {
                click = true;
            }
        }
    }
    return;
}

void destroy_ship()
{
    Mix_PlayChannel(-1, explosion, 0);
    SDL_Rect explosionPos;
    for (int i = 0; i < 100; i++) {
        explosionPos.x = shipPos.x + ((rand() % 150) - 30);
        explosionPos.y = shipPos.y + ((rand() % 150) - 30);
        SDL_BlitSurface(boom, NULL, window, &explosionPos);
        SDL_Flip(window);
        SDL_Delay(30);
    }
    running = false; // shut_down
}

void handle_target()
{
    targetPos.x = mousePosX - 25;
    targetPos.y = mousePosY - 25;
    if (click) // if there was a click
    {
        click = false;
        for (int i = 0; i < 10; i++) {
            if (asteroids[i].isCharged) {
                if (check_colide(targetPos, asteroids[i].pos) < 130) // 30 for target size,100 for shock size
                {
                    asteroids[i].isCharged = false;
                    Mix_PlayChannel(-1, zap, 0);
                }
            }
        }
    }
    SDL_BlitSurface(target, NULL, window, &targetPos);
}

void handle_ship()
{
    if (moveLeft) {
        shipPos.x -= shipSpeed;
    }
    if (moveRight){
        shipPos.x += shipSpeed;
    }
    SDL_BlitSurface(ship, NULL, window, &shipPos);
}

void handle_shield()
{
    if ((SDL_GetTicks() > shieldTimer + 5000) && (powerLevel < 5)) {
        powerLevel += 1;
        shieldTimer = SDL_GetTicks();
        Mix_PlayChannel(-1, charge, 0);
    }
    shieldPos.x = (shipPos.x + (ship->w / 2)) - (shields[powerLevel]->w / 2);
    shieldPos.y = (shipPos.y + (ship->h / 2)) - (shields[powerLevel]->h / 2);
    shieldPos.w = 150 + (powerLevel * 50);
    shieldPos.h = 150 + (powerLevel * 50);
    SDL_BlitSurface(shields[powerLevel], NULL, window, &shieldPos);
}

void handle_asteroids()
{
    for (int i = 0; i < 10; i++) {
        if (check_colide(shipPos, asteroids[i].pos) < ((asteroids[i].size + 1) * 20) + 45) // 45 for the ship colide
        // radius, +1 for size sero,
        // x20 for increase factor
        {
            SDL_BlitSurface(background, NULL, window, NULL);
            SDL_BlitSurface(ship, NULL, window, &shipPos);
            asteroids[i].show_asteroids(); // render them
            destroy_ship();
        }
        if (asteroids[i].isMetal) {
            if (asteroids[i].isCharged) {
                if (check_colide(shieldPos, asteroids[i].pos) < 175 + (powerLevel * 25)) // 75 for starter shield width,100 for
                // shock width, 25 shields size increase
                {
                    powerLevel = 0;
                    Mix_PlayChannel(-1, discharge, 0);
                }
            }
        }
        if ((asteroids[i].pos.y > windowHeght) || (asteroids[i].pos.x < -200) || (asteroids[i].pos.x > windowWidth)) {
            asteroids[i].reset(); // check for resets
        }
        asteroids[i].move(); // move them
        asteroids[i].show_asteroids(); // render them
    }
}

void handle_battery()
{
    SDL_BlitSurface(battery, NULL, window, NULL);
    SDL_Rect barPos;
    barPos.x = 10;
    barPos.y = 10;
    for (int i = 0; i < powerLevel + 1; i++) {
        SDL_BlitSurface(bar, NULL, window, &barPos);
        barPos.x += 20;
    }
}

void handle_timer()
{
    int time = SDL_GetTicks();
    stringstream ss;
    ss << "Time: " << (time - (time % 1000)) / 1000 << ":"
       << time % 1000;
    SDL_Color green = { 0, 255, 0 };
    timerSurface = TTF_RenderText_Solid(timerFont, ss.str().c_str(), green);
    SDL_Rect timerPos;
    timerPos.x = 700;
    timerPos.y = 0;
    SDL_BlitSurface(timerSurface, NULL, window, &timerPos);
}

void game_won()
{
    Mix_PlayChannel(-1, rocket, 0);
    for (int i = 0; i < 90; i++) // 90 so the ship is off the screen
    {
        shipPos.y -= 10;
        SDL_BlitSurface(background, NULL, window, NULL);
        SDL_BlitSurface(ship, NULL, window, new SDL_Rect(shipPos));
        handle_shield();
        handle_battery();
        SDL_Rect timerPos;
        timerPos.x = 700;
        timerPos.y = 0;
        SDL_BlitSurface(timerSurface, NULL, window, &timerPos);
        SDL_Flip(window);
        SDL_Delay(25);
    }
}

int main(int argc, char* args[])
{
    start_up();
    Mix_PlayMusic(spaceMusic, 0);
    while (running) {
        events_fxn();
        SDL_FillRect(window, NULL, 0);
        SDL_BlitSurface(background, NULL, window, NULL);
        handle_ship();
        handle_shield();
        handle_asteroids();
        handle_battery();
        handle_target();
        handle_timer();
        if (powerLevel == 5) {
            game_won();
            running = false;
        }
        SDL_Flip(window);
        SDL_Delay(0);
    }
    return 0;
}
