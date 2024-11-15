#include <cstdio>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "globals.h"


struct Buffer
{
    size_t width, height;
    uint32_t* data;
};

struct Sprite
{
    size_t width, height;
    uint8_t* data;
};

enum AlienType: uint8_t
{
    ALIEN_DEAD = 0,
    ALIEN_TYPE_A = 1,
    ALIEN_TYPE_B = 2,
    ALIEN_TYPE_C = 3
};

struct Alien
{
    size_t x,y;
    uint8_t type;
};

struct Player
{
    size_t x, y;
    size_t life;
};

struct Bullet
{
    size_t x, y;
    int dir;
};

struct Game
{
    size_t width, height;
    size_t num_player_bullets;
    size_t num_aliens;
    size_t num_alien_bullets;
    Alien* aliens;
    Player player;
    Bullet player_bullets[GAME_MAX_BULLETS];
    Bullet alien_bullets[GAME_MAX_BULLETS];
    std::vector<size_t> alive_aliens;
};

struct SpriteAnimation
{
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    Sprite** frames;
};


void error_callback(int error, const char* description);
void validate_shader(GLuint, const char *file = 0);
bool validate_program(GLuint);
void key_callback(GLFWwindow*, int key, int scancode, int action, int mods);
void buffer_clear(Buffer* buffer, uint32_t color);
bool sprite_overlap_check(
        const Sprite& sp_a, size_t x_a, size_t y_a,
        const Sprite& sp_b, size_t x_b, size_t y_b);
void buffer_draw_sprite(
        Buffer* buffer, const Sprite& sprite,
        size_t x, size_t y, uint32_t color);
void buffer_draw_text(
        Buffer* buffer,
        const Sprite& text_spritesheet,
        const char* text,
        size_t x, size_t y,
        uint32_t color);
void buffer_draw_number(
        Buffer* buffer,
        const Sprite& number_spritesheet, size_t number,
        size_t x, size_t y,
        uint32_t color);
uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b);
void fill_alien_sprites(Sprite* alien_sprites);
