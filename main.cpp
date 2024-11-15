#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "SpaceInvaders.h"


int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));
    const size_t buffer_width = 224;
    const size_t buffer_height = 256;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow *window = glfwCreateWindow(3 * buffer_width,3 * buffer_height, "Space Invaders", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "Error initializing GLEW.\n");
        glfwTerminate();
        return -1;
    }
    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
    printf("Renderer used: %s\n", glGetString(GL_RENDERER));
    printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glfwSwapInterval(1);

    glClearColor(1.0, 0.0, 0.0, 1.0);

    // Create graphics buffer
    Buffer buffer;
    buffer.width = buffer_width;
    buffer.height = buffer_height;
    buffer.data = new uint32_t[buffer.width * buffer.height];

    buffer_clear(&buffer, 0);

    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);

    //creating shader for displaying buffer
    static const char* fragment_shader =
            "\n"
            "#version 330\n"
            "\n"
            "uniform sampler2D buffer;\n"
            "noperspective in vec2 TexCoord;\n"
            "\n"
            "out vec3 outColor;\n"
            "\n"
            "void main(void){\n"
            "    outColor = texture(buffer, TexCoord).rgb;\n"
            "}\n";

    static const char* vertex_shader =
            "\n"
            "#version 330\n"
            "\n"
            "noperspective out vec2 TexCoord;\n"
            "\n"
            "void main(void){\n"
            "\n"
            "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
            "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
            "    \n"
            "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
            "}\n";


    GLuint shader_id = glCreateProgram();

    {
        //create vertex shader
        GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(shader_vp, 1, &vertex_shader, 0);
        glCompileShader(shader_vp);
        validate_shader(shader_vp, vertex_shader);
        glAttachShader(shader_id, shader_vp);

        glDeleteShader(shader_vp);
    }

    {
        //create fragment shader
        GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(shader_fp, 1, &fragment_shader, 0);
        glCompileShader(shader_fp);
        validate_shader(shader_fp, fragment_shader);
        glAttachShader(shader_id, shader_fp);

        glDeleteShader(shader_fp);
    }

    glLinkProgram(shader_id);

    if(!validate_program(shader_id))
    {
        fprintf(stderr, "Error while validating shader.\n");
        glfwTerminate();
        glDeleteVertexArrays(1, &fullscreen_triangle_vao);
        delete[] buffer.data;
        return -1;
    }

    glUseProgram(shader_id);

    GLint location = glGetUniformLocation(shader_id, "buffer");
    glUniform1i(location, 0);

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(fullscreen_triangle_vao);

    Sprite alien_sprites[6];
    fill_alien_sprites(alien_sprites);

    //filling sprites
    Sprite alien_death_sprite;
    alien_death_sprite.width = 13;
    alien_death_sprite.height = 7;
    alien_death_sprite.data = new uint8_t[91]
            {
        0,1,0,0,1,0,0,0,1,0,0,1,0, // .@..@...@..@.
        0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
        0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
        1,1,0,0,0,0,0,0,0,0,0,1,1, // @@.........@@
        0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
        0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
        0,1,0,0,1,0,0,0,1,0,0,1,0  // .@..@...@..@.
            };

    Sprite player_sprite;
    player_sprite.width = 11;
    player_sprite.height = 7;
    player_sprite.data = new uint8_t[77]
            {
        0,0,0,0,0,1,0,0,0,0,0, // .....@.....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
            };

    Sprite bullet_sprite;
    bullet_sprite.width = 1;
    bullet_sprite.height = 3;
    bullet_sprite.data = new uint8_t[3]
            {
        1,
        1,
        1
            };

    Sprite alien_bullet_sprites[2];
    alien_bullet_sprites[0].width = 3;
    alien_bullet_sprites[0].height = 4;
    alien_bullet_sprites[0].data = new uint8_t[12]
            {
        0,1,0,
        0,1,0,
        1,1,1,
        0,1,0
            };

    alien_bullet_sprites[1].width = 3;
    alien_bullet_sprites[1].height = 4;
    alien_bullet_sprites[1].data = new uint8_t[12]
            {
        0,1,0,
        1,1,1,
        0,1,0,
        0,1,0
            };

    Sprite text_spritesheet;
    text_spritesheet.width = 5;
    text_spritesheet.height = 7;

    text_spritesheet.data = new uint8_t[65 * 35]
            {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
        0,1,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,0,1,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,1,0,1,0,
        0,0,1,0,0,0,1,1,1,0,1,0,1,0,0,0,1,1,1,0,0,0,1,0,1,0,1,1,1,0,0,0,1,0,0,
        1,1,0,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,0,1,1,
        0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,1,0,1,1,1,1,
        0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
        1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
        0,0,1,0,0,1,0,1,0,1,0,1,1,1,0,0,0,1,0,0,0,1,1,1,0,1,0,1,0,1,0,0,1,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,

        0,1,1,1,0,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,1,1,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,
        1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,0,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,0,
        1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,

        0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
        0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
        0,1,1,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,0,1,1,1,0,

        0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,1,0,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
        1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
        1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,1,1,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
        0,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
        0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
        1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
        1,0,0,0,1,1,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
        1,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,0,1,0,1,1,0,0,1,1,1,0,0,0,1,1,0,0,0,1,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,0,1,1,0,1,1,1,1,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,1,1,1,0,
        1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,1,0,1,1,1,0,1,1,1,0,0,0,1,
        1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,
        1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
        1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,

        0,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,
        0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
        1,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0,
        0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
        0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            };

    Sprite number_spritesheet = text_spritesheet;
    number_spritesheet.data += 16 * 35;

    SpriteAnimation alien_animation[3];

    for(size_t i = 0; i < 3; i++) {
        alien_animation[i].loop = true;
        alien_animation[i].num_frames = 2;
        alien_animation[i].frame_duration = 30;
        alien_animation[i].time = 0;

        alien_animation[i].frames = new Sprite *[2];
        alien_animation[i].frames[0] = &alien_sprites[2 * i];
        alien_animation[i].frames[1] = &alien_sprites[2 * i + 1];
    }

    SpriteAnimation alien_bullet_animation;
    alien_bullet_animation.loop = true;
    alien_bullet_animation.num_frames = 2;
    alien_bullet_animation.frame_duration = 30;
    alien_bullet_animation.time = 0;
    alien_bullet_animation.frames = new Sprite* [2];
    alien_bullet_animation.frames[0] = &alien_bullet_sprites[0];
    alien_bullet_animation.frames[1] = &alien_bullet_sprites[1];

    //initializing Game
    Game game;
    game.width = buffer.width;
    game.num_player_bullets = 0;
    game.num_alien_bullets = 0;
    game.height = buffer.height;
    game.num_aliens = 55;
    game.aliens = new Alien[game.num_aliens];

    game.player.x = 112 - 5;
    game.player.y = 32;

    game.player.life = 3;

    for(size_t i = 0; i < game.num_aliens; i++)
    {
        game.alive_aliens.push_back(i);
    }

    for(size_t yi = 0; yi < 5; yi++)
    {
        for(size_t xi = 0; xi < 11; xi++)
        {
            Alien& alien = game.aliens[yi * 11 + xi];
            alien.type = (5 - yi) / 2 + 1;

            const Sprite& sprite = alien_sprites[2 * (alien.type - 1)];

            alien.x = 16 * xi + 20 + (alien_death_sprite.width - sprite.width) / 2;
            alien.y = 17 * yi + 128;
        }
    }

    uint8_t* death_counters = new uint8_t[game.num_aliens];
    for(size_t i = 0; i < game.num_aliens; i++)
    {
        death_counters[i] = 10;
    }

    uint32_t clear_color = rgb_to_uint32(139, 0, 255);

    size_t score = 0;
    size_t credits = 0;

    int player_move_dir = 0;

    game_running = true;

    while(!glfwWindowShouldClose(window) && game_running)
    {
        buffer_clear(&buffer, clear_color);

        //stats output
        buffer_draw_text(
                &buffer,
                text_spritesheet, "SCORE",
                4, game.height - text_spritesheet.height - 7,
                rgb_to_uint32(0, 0, 0));

        buffer_draw_number(
                &buffer,
                number_spritesheet, score,
                4 + 2 * number_spritesheet.width,
                game.height - 2 * number_spritesheet.height - 12,
                rgb_to_uint32(0, 0 ,0));

        buffer_draw_text(
                &buffer,
                text_spritesheet, "LIVES:",
                game.width - text_spritesheet.width * 6 - 20,
                game.height - text_spritesheet.height - 7,
                rgb_to_uint32(0, 0, 0));

        buffer_draw_number(
                &buffer,
                number_spritesheet, game.player.life,
                game.width - 13,
                game.height - text_spritesheet.height - 7,
                rgb_to_uint32(0, 0, 0));

        for(size_t i = 0; i < game.width; i++)
        {
            buffer.data[game.width * 16 + i] = rgb_to_uint32(0, 0, 0);
        }

        //drawing aliens
        for(size_t ai = 0; ai < game.num_aliens; ai++)
        {
            if(!death_counters[ai]) continue;

            const Alien& alien = game.aliens[ai];
            if(alien.type == ALIEN_DEAD)
            {
                buffer_draw_sprite(&buffer, alien_death_sprite, alien.x, alien.y, rgb_to_uint32(255, 255, 255));
            }
            else
            {
                const SpriteAnimation& animation = alien_animation[alien.type - 1];
                size_t current_frame = alien_animation->time / alien_animation->frame_duration;
                const Sprite &sprite = *animation.frames[current_frame];
                buffer_draw_sprite(&buffer, sprite,
                                   alien.x, alien.y, rgb_to_uint32(255, 255, 255));
            }
        }
        buffer_draw_sprite(&buffer, player_sprite,
                           game.player.x, game.player.y, rgb_to_uint32(128, 0, 0));

        //drawing player bullets
        for(size_t bi = 0; bi < game.num_player_bullets; bi++)
        {
            const Bullet& bullet = game.player_bullets[bi];
            const Sprite& sprite = bullet_sprite;
            buffer_draw_sprite(&buffer, sprite, bullet.x, bullet.y, rgb_to_uint32(128, 0, 0));
        }

        //drawing alien bullets
        for(size_t bi = 0; bi < game.num_alien_bullets; bi++)
        {
            const Bullet& bullet = game.alien_bullets[bi];
            const SpriteAnimation& animation = alien_bullet_animation;
            size_t current_frame = alien_bullet_animation.time / alien_bullet_animation.frame_duration;
            const Sprite& sprite = *animation.frames[current_frame];
            buffer_draw_sprite(&buffer, sprite, bullet.x, bullet.y, rgb_to_uint32(0, 0, 0));
        }

        alien_bullet_animation.time++;
        if(alien_bullet_animation.time == alien_bullet_animation.frame_duration * alien_bullet_animation.num_frames)
        {
            if(alien_bullet_animation.loop) alien_bullet_animation.time = 0;
        }

        for(size_t i = 0; i < 3; i++)
        {
            ++alien_animation[i].time;
            if (alien_animation[i].time == alien_animation[i].num_frames * alien_animation[i].frame_duration) {
                if (alien_animation[i].loop) alien_animation[i].time = 0;
            }
        }

        glTexSubImage2D(
                GL_TEXTURE_2D, 0, 0, 0,
                buffer.width, buffer.height,
                GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
                buffer.data
                );


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);

        for(size_t ai = 0; ai < game.num_aliens; ai++)
        {
            const Alien& alien = game.aliens[ai];
            if(alien.type == ALIEN_DEAD && death_counters[ai])
            {
                death_counters[ai]--;
            }
        }

        //player bullets simulation
        for(size_t bi = 0; bi < game.num_player_bullets;) {
            game.player_bullets[bi].y += game.player_bullets[bi].dir;
            if (game.player_bullets[bi].y >= game.height ||
                game.player_bullets[bi].y < bullet_sprite.height) {
                game.player_bullets[bi] = game.player_bullets[game.num_player_bullets-- - 1];
                continue;
            }

            for (size_t ai = 0; ai < game.num_aliens; ai++) {
                const Alien &alien = game.aliens[ai];
                if (alien.type == ALIEN_DEAD) continue;

                const SpriteAnimation animation = alien_animation[alien.type - 1];
                size_t current_frame = animation.time / animation.frame_duration;
                const Sprite &alien_sprite = *animation.frames[current_frame];
                bool overlap = sprite_overlap_check(
                        bullet_sprite, game.player_bullets[bi].x, game.player_bullets[bi].y,
                        alien_sprite, alien.x, alien.y);
                if(overlap)
                {
                    game.aliens[ai].type = ALIEN_DEAD;
                    game.aliens[ai].x -= (alien_death_sprite.width - alien_sprite.width) / 2;
                    game.player_bullets[bi] = game.player_bullets[game.num_player_bullets - 1];
                    game.num_player_bullets--;
                    score += 10 * (4 - game.aliens[ai].type);
                    for(auto it = game.alive_aliens.begin(); it != game.alive_aliens.end();)
                    {
                        if(*it == ai) it = game.alive_aliens.erase(it);
                        else it++;
                    }
                    continue;
                }

            }

            bi++;
        }

        //alien bullets simulation
        for(size_t bi = 0; bi < game.num_alien_bullets;)
        {
            size_t current_frame = alien_bullet_animation.time / alien_bullet_animation.frame_duration;
            Bullet& current_bullet = game.alien_bullets[bi];
            Sprite& current_sprite = *alien_bullet_animation.frames[current_frame];
            game.alien_bullets[bi].y += game.alien_bullets[bi].dir;
            if (game.alien_bullets[bi].y >= game.height ||
                game.alien_bullets[bi].y < current_sprite.height) {
                game.alien_bullets[bi] = game.alien_bullets[game.num_alien_bullets-- - 1];
                continue;
            }
            bool overlap = sprite_overlap_check(current_sprite, current_bullet.x, current_bullet.y,
                                                player_sprite, game.player.x, game.player.y);
            if(overlap)
            {
                game.alien_bullets[bi] = game.alien_bullets[game.num_alien_bullets - 1];
                game.num_alien_bullets--;
                game.player.life--;
            }
            bi++;
        }

        //creating new player bullet
        if(fire_pressed && game.num_player_bullets < GAME_MAX_BULLETS)
        {
            game.player_bullets[game.num_player_bullets].x = game.player.x + player_sprite.width / 2;
            game.player_bullets[game.num_player_bullets].y = game.player.y + player_sprite.height;
            game.player_bullets[game.num_player_bullets].dir = 2;
            game.num_player_bullets++;
        }
        fire_pressed = false;

        //creating new alien bullet
        if(alien_shot_timer >= alien_shot_interval)
        {
            alien_shot_timer = 0;
            if(game.num_alien_bullets < GAME_MAX_BULLETS)
            {
                size_t random_idx = rand() % game.alive_aliens.size();
                Alien& alien = game.aliens[game.alive_aliens[random_idx]];
                game.alien_bullets[game.num_alien_bullets].x = alien.x + (alien_sprites[2 * (alien.type - 1)].width) / 2 - 1;
                game.alien_bullets[game.num_alien_bullets].y = alien.y;
                game.alien_bullets[game.num_alien_bullets].dir = -2;
                game.num_alien_bullets++;
            }
        }

        //player moving simulation
        player_move_dir = 2 * move_dir;

        if(game.player.x + player_sprite.width + player_move_dir >= game.width - 1)
        {
            game.player.x = game.width - player_sprite.width - 1;
        }
        else if((int)game.player.x + player_move_dir <= 0)
        {
            game.player.x = 0;
        }
        else game.player.x += player_move_dir;

        alien_shot_timer++;
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    glDeleteVertexArrays(1, &fullscreen_triangle_vao);

    for(size_t i = 0; i < 6; i++)
    {
        delete[] alien_sprites[i].data;
    }

    delete[] text_spritesheet.data;
    delete[] alien_death_sprite.data;

    for(size_t i = 0; i < 3; i++)
    {
        delete[] alien_animation[i].frames;
    }

    delete[] buffer.data;
    delete[] game.aliens;
    delete[] death_counters;

    return 0;
}

