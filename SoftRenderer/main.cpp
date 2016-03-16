#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <DirectXMath.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float float32;
typedef double float64;

typedef uint8 byte;

typedef unsigned int uint;

namespace video
{
    struct color
    {
        uint8 m_a = 255;
        uint8 m_r = 0;
        uint8 m_g = 0;
        uint8 m_b = 0;

        color(uint8 _r, uint8 _g, uint8 _b, uint8 _a = 255)
            : m_r(_r), m_g(_g), m_b(_b), m_a(_a)
        {
        }
    };

    struct camera
    {
        glm::vec3 m_position;
        glm::vec3 m_target;
    };

    class mesh
    {
    public:
        mesh(glm::vec3* _vertices, int _vertCount);
        ~mesh();

    public:
        std::vector<glm::vec3> m_vertices;
        glm::vec3 m_position;
        glm::vec3 m_rotation;
    };

    mesh::mesh(glm::vec3* _vertices, int _vertCount)
        : m_position(glm::vec3(0.f, 0.f, 0.f)),
        m_rotation(glm::vec3(0.f, 0.f, 0.f))
    {
        for (int i = 0; i < _vertCount; ++i) {
            m_vertices.push_back(_vertices[i]);
        }
    }

    mesh::~mesh()
    {
    }

    class device
    {
    public:
        device(int _width, int _height)
            : m_width(_width), m_height(_height)
        {
            m_buffer = new uint32[m_width * m_height];
        }

        ~device()
        {
            delete m_buffer;
        }

        void clear(uint32 _value = 0xFF000000);
        void poke(int _index, uint32 _value);
        void draw_point(const glm::vec2& _position);
        glm::vec2 project(const glm::vec3& _position, const glm::mat4& _translationMatrix);

        SDL_Surface* create_surface();

        int index_from_xy(int _x, int _y) const { return m_width * _y + _x; }

        void render(const camera& _camera, mesh* _meshes, int _meshCount);

    private:
        int m_width = 0;
        int m_height = 0;
        uint32* m_buffer = nullptr;
    };

    void device::clear(uint32 _value /* = 0xFF000000 */)
    {
        int size = m_width * m_height;
        for (int i = 0; i < size; ++i) {
            m_buffer[i] = _value;
        }
    }

    void device::poke(int _index, uint32 _value)
    {
        m_buffer[_index] = _value;
    }

    void device::draw_point(const glm::vec2& _position)
    {
        if (_position.x >= 0 && _position.y >= 0 && _position.x < m_width && _position.y < m_height) {
            poke(index_from_xy((int)_position.x, (int)_position.y), 0xFFFFFF00);
        }
    }

    glm::vec2 device::project(const glm::vec3& _position, const glm::mat4& _translationMatrix)
    {
        auto point = _translationMatrix * glm::vec4(_position, 1.f);
        //auto point = glm::vec4(_position, 1.f) * _translationMatrix;
        float32 x = point.x * m_width + m_width / 2.f;
        float32 y = -point.y * m_height + m_height / 2.f;
        return glm::vec2(x, y);
    }

    SDL_Surface* device::create_surface()
    {
        return SDL_CreateRGBSurfaceFrom(m_buffer, m_width, m_height, 32, m_width * 4, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    }

    void device::render(const camera& _camera, mesh* _meshes, int _meshCount)
    {
        auto viewMatrix = glm::lookAt(_camera.m_position, _camera.m_target, glm::vec3(0.f, 1.f, 0.f));
        auto projectionMatrix = glm::perspective(1.75f, (float32)m_width / (float32)m_height, 0.1f, 1000.f);

        for (int i = 0; i < _meshCount; ++i) {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.f), _meshes[i].m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
            rotation = glm::rotate(rotation, _meshes[i].m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
            rotation = glm::rotate(rotation, _meshes[i].m_rotation.z, glm::vec3(0.f, 0.f, 1.f));

            glm::mat4 translation = glm::translate(glm::mat4(1.f), _meshes[i].m_position);
            auto worldMatrix = translation * rotation;

            auto transformMatrix = projectionMatrix * viewMatrix;

            for (auto vertex : _meshes[i].m_vertices) {
                //auto point = glm::project(vertex, viewMatrix * worldMatrix, projectionMatrix, glm::vec4(0.f, (float32)m_width, 0.f, (float32)m_height));
                auto point = project(vertex, transformMatrix);
                draw_point(point);
            }
        }
    }
}

namespace constants
{
    const int width = 800;
    const int height = 600;
}

int main(int argc, char* argv[])
{
    SDL_Window* window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, constants::width, constants::height, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    const int pixelSize = 1;
    video::device device(constants::width / pixelSize, constants::height / pixelSize);

    const float halfSize = 0.2f;

    glm::vec3 vertices[8] = {
        { -halfSize, halfSize, halfSize },
        { halfSize, halfSize, halfSize },
        { -halfSize, -halfSize, halfSize },
        { -halfSize, -halfSize, -halfSize },
        { -halfSize, halfSize, -halfSize },
        { halfSize, halfSize, -halfSize },
        { halfSize, -halfSize, halfSize },
        { halfSize, -halfSize, -halfSize },
    };
    video::mesh cubeMesh(vertices, 8);

    for (auto v : vertices) {
        std::cout << "-----------------------\n";
        std::cout << v.x << ", " << v.y << ", " << v.z << std::endl;
    }

    bool isRunning = true;

    video::camera defaultCamera;
    defaultCamera.m_position = glm::vec3(0.f, 0.f, 10.f);
    defaultCamera.m_target = glm::vec3(0.f, 0.f, 0.f);

    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    isRunning = false;
                }
                break;

            case SDL_QUIT:
                isRunning = false;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        device.clear();

        device.render(defaultCamera, &cubeMesh, 1);

        defaultCamera.m_position.z += 0.01f;
        defaultCamera.m_target.z += 0.0001f;
        cubeMesh.m_position.z += 0.0001f;
        cubeMesh.m_rotation.x += 0.0005f;
        cubeMesh.m_rotation.y += 0.0005f;

        SDL_Surface* surface = device.create_surface();
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        SDL_RenderPresent(renderer);

        //SDL_Delay(10000);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}