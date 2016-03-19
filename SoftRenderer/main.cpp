#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <limits>
#include <SDL2/SDL.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

namespace math
{
    float32 clamp(float32 _value, float32 _min = 0.f, float32 _max = 1.f);
    float32 lerp(float32 _a, float32 _b, float32 _t);

    float32 clamp(float32 _value, float32 _min, float32 _max)
    {
        if (_value < _min) { return _min; }
        else if (_value > _max) { return _max; }
        else { return _value; }
    }

    float32 lerp(float32 _a, float32 _b, float32 _t)
    {
        return (_b - _a) * _t + _a;
    }
}

namespace video
{
    struct color
    {
        uint8 m_a = 255;
        uint8 m_r = 0;
        uint8 m_g = 0;
        uint8 m_b = 0;

        color()
            : m_a(255), m_r(255), m_g(255), m_b(255)
        { }

        color(uint8 _r, uint8 _g, uint8 _b, uint8 _a = 255)
            : m_a(_a), m_r(_r), m_g(_g), m_b(_b)
        { }

        const static color s_white;
        const static color s_black;
        const static color s_red;
        const static color s_green;
        const static color s_blue;
        const static color s_magenta;
        const static color s_yellow;
        const static color s_cyan;
    };

    uint32 color_pack(const color& _color);

    const color color::s_white = color{ 255, 255, 255 };
    const color color::s_black = color{ 0, 0, 0 };
    const color color::s_red = color{ 255, 0, 0 };
    const color color::s_green = color{ 0, 255, 0 };
    const color color::s_blue = color{ 0, 0, 255 };
    const color color::s_magenta = color{ 255, 0, 255 };
    const color color::s_yellow = color{ 255, 255, 0 };
    const color color::s_cyan = color{ 0, 255, 255 };

    uint32 color_pack(const color& _color)
    {
        return (_color.m_a << 24) | (_color.m_r << 16) | (_color.m_g << 8) | (_color.m_b << 0);
    }

    struct camera
    {
        glm::vec3 m_position;
        glm::vec3 m_target;
    };

    class mesh
    {
    public:
        mesh(glm::vec3* _vertices, int _vertCount, uint16* _indices, int _faceCount);
        ~mesh();

        struct face
        {
            uint16 m_a, m_b, m_c;

            face(uint16 _a, uint16 _b, uint16 _c) : m_a(_a), m_b(_b), m_c(_c) {}
        };

    public:
        std::vector<glm::vec3> m_vertices;
        std::vector<face> m_faces;
        glm::vec3 m_position;
        glm::vec3 m_rotation;
    };

    mesh::mesh(glm::vec3* _vertices, int _vertCount, uint16* _indices, int _faceCount)
        : m_position(glm::vec3(0.f, 0.f, 0.f)),
        m_rotation(glm::vec3(0.f, 0.f, 0.f))
    {
        for (int i = 0; i < _vertCount; ++i) {
            m_vertices.push_back(_vertices[i]);
        }

        for (int i = 0; i < _faceCount; ++i) {
            mesh::face face = {
                _indices[(i * 3) + 0],
                _indices[(i * 3) + 1],
                _indices[(i * 3) + 2],
            };
            m_faces.push_back(face);
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
            m_depthBuffer = new float32[m_width * m_height];
        }

        ~device()
        {
            delete m_buffer;
            delete m_depthBuffer;
        }

        void clear(uint32 _value = 0xFF000000);
        void poke(int _index, uint32 _value);
        void put_pixel(int _x, int _y, float32 _depth, const color& _color);
        void draw_point(const glm::vec3& _position, const color& _color);
        void draw_line(const glm::vec3& _start, const glm::vec3& _end, const color& _color);
        void draw_triangle(const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3, const color& _color);
        glm::vec3 project(const glm::vec3& _position, const glm::mat4& _translationMatrix);

        SDL_Surface* create_surface();

        int index_from_xy(int _x, int _y) const
        {
            int result = m_width * _y + _x;
            if (result < 0 || result >= m_width * m_height) {
                return -1;
            }
            return result;
        }

        void render(const camera& _camera, mesh* _meshes, int _meshCount);

    private:
        int m_width = 0;
        int m_height = 0;
        uint32* m_buffer = nullptr;
        float32* m_depthBuffer = nullptr;
    };

    void device::clear(uint32 _value /* = 0xFF000000 */)
    {
        int size = m_width * m_height;
        for (int i = 0; i < size; ++i) {
            m_buffer[i] = _value;
            m_depthBuffer[i] = std::numeric_limits<float32>::max();
        }
    }

    void device::poke(int _index, uint32 _value)
    {
        m_buffer[_index] = _value;
    }

    void device::put_pixel(int _x, int _y, float32 _depth, const color& _color)
    {
        int index = index_from_xy(_x, _y);
        if (index < 0) {
            return;
        }

        if (m_depthBuffer[index] < _depth) {
            return;
        }

        m_depthBuffer[index] = _depth;
        poke(index, color_pack(_color));
    }

    void device::draw_point(const glm::vec3& _position, const color& _color)
    {
        put_pixel((int)_position.x, (int)_position.y, _position.z, _color);
    }

    void device::draw_line(const glm::vec3& _start, const glm::vec3& _end, const color& _color)
    {
        glm::vec3 start(_start);
        glm::vec3 end(_end);

        auto delta = glm::abs(end - start);
        auto sx = (start.x < end.x) ? 1 : -1;
        auto sy = (start.y < end.y) ? 1 : -1;
        auto err = delta.x - delta.y;

        glm::vec3 current = start;

        while (true) {
            draw_point(glm::vec3(current), _color);

            if (current == end) {
                break;
            }

            auto e2 = err * 2;

            if (e2 > -delta.y) {
                err -= delta.y;
                current.x += sx;
            }

            if (e2 < delta.x) {
                err += delta.x;
                current.y += sy;
            }
        }
    }

    void device::draw_triangle(const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3, const color& _color)
    {
        std::array<glm::vec3, 3> verts = {
            _v1, _v2, _v3
        };

        std::sort(verts.begin(), verts.end(), [](const glm::ivec3& _a, const glm::ivec3& _b) {
            return _a.y < _b.y;
        });

        auto top = verts[0];
        auto mid = verts[1];
        auto bot = verts[2];

        {
            float32 leftdx = (float32)(bot.x - top.x) / (float32)(bot.y - top.y);
            float32 rightdx = (float32)(mid.x - top.x) / (float32)(mid.y - top.y);

            if (mid.x < top.x) {
                std::swap(leftdx, rightdx);
            }

            float32 left, right;
            left = right = top.x;
            for (int y = top.y; y < mid.y; ++y) {
                for (int x = (int)left; x <= (int)right; ++x) {
                    float32 z = math::lerp(leftZ, rightZ, (float32)x / std::abs(right - left));
                    draw_point(glm::vec3((float32)x, (float32)y, z), _color);
                }
                left += leftdx;
                right += rightdx;
                leftZ += leftdz;
                rightZ += rightdz;
            }
        }

        {
            //float32 leftdx = (float32)(top.x - bot.x) / (float32)(top.y - bot.y);
            //float32 rightdx = (float32)(mid.x - bot.x) / (float32)(mid.y - bot.y);

            //if (mid.x < top.x) {
            //    std::swap(leftdx, rightdx);
            //}

            //float32 left, right;
            //left = right = (float32)bot.x;
            //for (int y = bot.y; y >= mid.y; --y) {
            //    draw_hline(y, (int)left, (int)right, _color);
            //    left -= leftdx;
            //    right -= rightdx;
            //}
        }
    }

    glm::vec3 device::project(const glm::vec3& _position, const glm::mat4& _translationMatrix)
    {
        auto point = _translationMatrix * glm::vec4(_position, 1.f);
        point /= point.w;
        float32 x = point.x * m_width + m_width / 2.f;
        float32 y = -point.y * m_height + m_height / 2.f;
        return glm::vec3(x, y, point.z);
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

            auto transformMatrix = projectionMatrix * viewMatrix * worldMatrix;

            int count = 0;
            for (auto face : _meshes[i].m_faces) {
                auto vertexA = _meshes[i].m_vertices[face.m_a];
                auto vertexB = _meshes[i].m_vertices[face.m_b];
                auto vertexC = _meshes[i].m_vertices[face.m_c];

                auto pointA = project(vertexA, transformMatrix);
                auto pointB = project(vertexB, transformMatrix);
                auto pointC = project(vertexC, transformMatrix);

                draw_triangle(pointA, pointB, pointC, (count++ % 2 == 0) ? color::s_yellow : color::s_cyan);
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

    const int pixelSize = 4;
    video::device device(constants::width / pixelSize, constants::height / pixelSize);

    const float halfSize = 2.f;

    glm::vec3 vertices[8] = {
        { -halfSize, halfSize, halfSize },
        { halfSize, halfSize, halfSize },
        { -halfSize, -halfSize, halfSize },
        { halfSize, -halfSize, halfSize },
        { -halfSize, halfSize, -halfSize },
        { halfSize, halfSize, -halfSize },
        { halfSize, -halfSize, -halfSize },
        { -halfSize, -halfSize, -halfSize },
    };

    uint16 indices[12 * 3] = {
        0, 1, 2,
        1, 2, 3,
        1, 3, 6,
        1, 5, 6,
        0, 1, 4,
        1, 4, 5,
        2, 3, 7,
        3, 6, 7,
        0, 2, 7,
        0, 4, 7,
        4, 5, 6,
        4, 6, 7
    };

    video::mesh cubeMesh(vertices, 8, indices, 12);

    bool isRunning = true;

    video::camera defaultCamera;
    defaultCamera.m_position = glm::vec3(0.f, 0.f, 10.f);
    defaultCamera.m_target = glm::vec3(0.f, 0.f, 0.f);

    float32 angle = 0.f;

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

        cubeMesh.m_rotation.x += 0.0023f;
        cubeMesh.m_rotation.y += 0.001f;

        SDL_Surface* surface = device.create_surface();
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
