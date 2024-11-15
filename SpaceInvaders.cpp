#include <cstdio>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "SpaceInvaders.h"




void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s", description);
}

void validate_shader(GLuint shader, const char *file){
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);

    if(length > 0) {
        printf("Shader %d(%s) compile error: %s\n", shader, (file ? file : ""), buffer);
    }
}

bool validate_program(GLuint program){
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

    if(length > 0){
        printf("Program %d link error: %s", program, buffer);
        return false;
    }

    return true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS) game_running = false;
            break;
        case GLFW_KEY_D:
            if(action == GLFW_PRESS) move_dir += 1;
            else if(action == GLFW_RELEASE) move_dir -= 1;
            break;
        case GLFW_KEY_A:
            if(action == GLFW_PRESS) move_dir -= 1;
            else if(action == GLFW_RELEASE) move_dir += 1;
            break;
        case GLFW_KEY_SPACE:
            if(action == GLFW_RELEASE) fire_pressed = true;
            break;
        default:
            break;
    }
}

void buffer_clear(Buffer* buffer, uint32_t color)
{
    for(size_t i = 0; i < buffer->width * buffer->height; ++i)
    {
        buffer->data[i] = color;
    }
}

bool sprite_overlap_check(
        const Sprite& sp_a, size_t x_a, size_t y_a,
        const Sprite& sp_b, size_t x_b, size_t y_b)
{
    if(x_a < x_b + sp_b.width && x_a + sp_a.width > x_b &&
       y_a < y_b + sp_b.height && y_a + sp_a.height > y_b)
    {
        return true;
    }
    return false;
}

void buffer_draw_sprite(
        Buffer* buffer, const Sprite& sprite,
        size_t x, size_t y, uint32_t color)
{
    for(size_t xi = 0; xi < sprite.width; xi++)
    {
        for(size_t yi = 0; yi < sprite.height; yi++)
        {
            if(sprite.data[yi * sprite.width + xi] &&
               (sprite.height - 1 + y - yi) < buffer->height &&
               (x + xi) < buffer->width)
            {
                buffer->data[(sprite.height - 1 + y - yi) * buffer->width + (x + xi)] = color;
            }
        }
    }
}

void buffer_draw_text(
        Buffer* buffer,
        const Sprite& text_spritesheet,
        const char* text,
        size_t x, size_t y,
        uint32_t color)
{
    size_t xp = x;
    size_t stride = text_spritesheet.width * text_spritesheet.height;
    Sprite sprite = text_spritesheet;
    for(const char* charp = text; *charp != '\0'; charp++)
    {
        char character = *charp - 32;
        if(character < 0 || character >= 65) continue;

        sprite.data = text_spritesheet.data + character * stride;
        buffer_draw_sprite(buffer, sprite, xp, y, color);
        xp += sprite.width + 1;
    }
}

void buffer_draw_number(
        Buffer* buffer,
        const Sprite& number_spritesheet, size_t number,
        size_t x, size_t y,
        uint32_t color)
{
    uint8_t digits[64];
    size_t num_digits = 0;

    size_t current_number = number;
    do
    {
        digits[num_digits++] = current_number % 10;
        current_number /= 10;
    }
    while(current_number > 0);
    size_t xp = x;
    size_t stride = number_spritesheet.width * number_spritesheet.height;
    Sprite sprite = number_spritesheet;
    for(size_t i = 0; i < num_digits; i++)
    {
        uint8_t digit = digits[num_digits - 1 - i];
        sprite.data = number_spritesheet.data + digit * stride;
        buffer_draw_sprite(buffer, sprite, xp, y, color);
        xp += sprite.width + 1;
    }
}

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b){return (r << 24) | (g << 16) | (b << 8) | 255;}

void fill_alien_sprites(Sprite* alien_sprites)
{
    alien_sprites[0].width = 8;
    alien_sprites[0].height = 8;
    alien_sprites[0].data = new uint8_t[64]
            {
                    0,0,0,1,1,0,0,0, // ...@@...
                    0,0,1,1,1,1,0,0, // ..@@@@..
                    0,1,1,1,1,1,1,0, // .@@@@@@.
                    1,1,0,1,1,0,1,1, // @@.@@.@@
                    1,1,1,1,1,1,1,1, // @@@@@@@@
                    0,1,0,1,1,0,1,0, // .@.@@.@.
                    1,0,0,0,0,0,0,1, // @......@
                    0,1,0,0,0,0,1,0  // .@....@.
            };

    alien_sprites[1].width = 8;
    alien_sprites[1].height = 8;
    alien_sprites[1].data = new uint8_t[64]
            {
                    0,0,0,1,1,0,0,0, // ...@@...
                    0,0,1,1,1,1,0,0, // ..@@@@..
                    0,1,1,1,1,1,1,0, // .@@@@@@.
                    1,1,0,1,1,0,1,1, // @@.@@.@@
                    1,1,1,1,1,1,1,1, // @@@@@@@@
                    0,0,1,0,0,1,0,0, // ..@..@..
                    0,1,0,1,1,0,1,0, // .@.@@.@.
                    1,0,1,0,0,1,0,1  // @.@..@.@
            };

    alien_sprites[2].width = 11;
    alien_sprites[2].height = 8;
    alien_sprites[2].data = new uint8_t[11 * 8]
            {
                    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
                    0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
                    0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
                    0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
                    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
                    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
                    1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
                    0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
            };

    alien_sprites[3].width = 11;
    alien_sprites[3].height = 8;
    alien_sprites[3].data = new uint8_t[88]
            {
                    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
                    1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
                    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
                    1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
                    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
                    0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
                    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
                    0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
            };

    alien_sprites[4].width = 12;
    alien_sprites[4].height = 8;
    alien_sprites[4].data = new uint8_t[96]
            {
                    0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
                    0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
                    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
                    1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
                    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
                    0,0,0,1,1,0,0,1,1,0,0,0, // ...@@..@@...
                    0,0,1,1,0,1,1,0,1,1,0,0, // ..@@.@@.@@..
                    1,1,0,0,0,0,0,0,0,0,1,1  // @@........@@
            };

    alien_sprites[5].width = 12;
    alien_sprites[5].height = 8;
    alien_sprites[5].data = new uint8_t[96]
            {
                    0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
                    0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
                    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
                    1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
                    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
                    0,0,1,1,1,0,0,1,1,1,0,0, // ..@@@..@@@..
                    0,1,1,0,0,1,1,0,0,1,1,0, // .@@..@@..@@.
                    0,0,1,1,0,0,0,0,1,1,0,0  // ..@@....@@..
            };
}
