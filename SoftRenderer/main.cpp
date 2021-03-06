#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <limits>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

        color(uint32 _int)
            : m_a((_int & 0x000000FF) >> 24), m_r((_int & 0x00FF) >> 16), m_g((_int & 0x0000FF) >> 8), m_b(_int & 0xFF)
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

        enum class buffer_type
        {
            cColor,
            cDepth,
        };

        void resize(int _width, int _height);
        void clear(uint32 _value = 0xFF000000);
        void poke(int _index, uint32 _value);
        void put_pixel(int _x, int _y, float32 _depth, const color& _color);
        void draw_point(const glm::vec3& _position, const color& _color);
        void draw_line(const glm::vec3& _start, const glm::vec3& _end, const color& _color);
        void draw_triangle(const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3, const color& _color);
        void draw_hline(int _y, int _left, int _right, float32 _leftz, float32 _rightz, const color& _color);
        glm::vec3 project(const glm::vec3& _position, const glm::mat4& _translationMatrix);

        uint32* get_colors() const { return m_buffer; }
        int get_width() const { return m_width; }
        int get_height() const { return m_height; }
        int get_size() const { return m_width * m_height; }

        SDL_Surface* create_surface(buffer_type _bufferType = buffer_type::cColor);

        int index_from_xy(int _x, int _y) const
        {
            int result = m_width * _y + _x;
            if (result < 0 || result >= m_width * m_height) {
                return -1;
            }
            return result;
        }

        void xy_from_index(int _index, int& _x, int& _y) const
        {
            _x = _index % m_width;
            _y = _index / m_width;
        }

        void render(const camera& _camera, mesh* _meshes, int _meshCount);

    private:
        int m_width = 0;
        int m_height = 0;
        uint32* m_buffer = nullptr;
        float32* m_depthBuffer = nullptr;
    };

    void device::resize(int _width, int _height)
    {
        m_width = _width;
        m_height = _height;

        delete m_buffer;
        delete m_depthBuffer;

        m_buffer = new uint32[m_width * m_height];
        m_depthBuffer = new float32[m_width * m_height];

        clear();
    }

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
        glm::ivec3 start(_start);
        glm::ivec3 end(_end);

        auto delta = glm::abs(end - start);
        auto sx = (start.x < end.x) ? 1 : -1;
        auto sy = (start.y < end.y) ? 1 : -1;
        auto err = delta.x - delta.y;

        glm::ivec3 current = start;

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

    void device::draw_hline(int _y, int _left, int _right, float32 _leftz, float32 _rightz, const color& _color)
    {
        if (_y < 0 || _y >= m_height) {
            return;
        }

        int left = std::max(0, _left);
        int right = std::min(_right, m_width);

        if (left > m_width && right > m_width) {
            std::cout << "what\n";
        }

        if (right < left) {
            std::swap(left, right);
        }
        else if (left == right) {
            draw_point(glm::vec3(left, _y, _leftz), _color);
        }

        int distance = right - left;
        for (int x = left; x < right; ++x) {
            if (x < 0 || x >= m_width) {
                continue;
            }
            glm::vec3 point((float32)x, (float32)_y, math::lerp(_leftz, _rightz, (float32)x / (float32)distance));
            draw_point(point, _color);
        }
    }

    void device::draw_triangle(const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3, const color& _color)
    {
        std::array<glm::vec3, 3> verts = {
            { _v1, _v2, _v3 }
        };

        std::sort(verts.begin(), verts.end(), [](const glm::ivec3& _a, const glm::ivec3& _b) {
            return _a.y < _b.y;
        });

        auto top = verts[0];
        auto mid = verts[1];
        auto bot = verts[2];

        {
            float32 leftdx = (bot.x - top.x) / (bot.y - top.y);
            float32 rightdx = (mid.x - top.x) / (mid.y - top.y);
            float32 leftdz = (bot.z - top.z) / (bot.y - top.y);
            float32 rightdz = (mid.z - top.z) / (mid.y / top.y);

            if (mid.x < bot.x) {
                std::swap(leftdx, rightdx);
                std::swap(leftdz, rightdz);
            }

            glm::vec3 left, right;
            left = right = top;
            for (int y = top.y; y < mid.y; ++y) {
                draw_hline(y, (int)left.x, (int)right.x, left.z, right.z, _color);
                left.x += leftdx;
                right.x += rightdx;
                left.z += leftdz;
                right.z += rightdz;
            }
        }

        {
            float32 leftdx = (top.x - bot.x) / (top.y - bot.y);
            float32 rightdx = (mid.x - bot.x) / (mid.y - bot.y);
            float32 leftdz = (top.z - bot.z) / (top.y - bot.y);
            float32 rightdz = (mid.z - bot.z) / (top.y - bot.y);

            if (mid.x < top.x) {
               std::swap(leftdx, rightdx);
               std::swap(leftdz, rightdz);
            }

            glm::vec3 left, right;
            left = right = bot;
            for (int y = bot.y; y >= mid.y; --y) {
                draw_hline(y, (int)left.x, (int)right.x, left.z, right.z, _color);
                left.x -= leftdx;
                right.x -= rightdx;
                left.z -= leftdz;
                right.z -= rightdz;
            }
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

    SDL_Surface* device::create_surface(buffer_type _bufferType)
    {
        switch (_bufferType) {
            default:
            case buffer_type::cColor:
                return SDL_CreateRGBSurfaceFrom(m_buffer, m_width, m_height, 32, m_width * 4, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

            case buffer_type::cDepth:
                {
                    float32 maxDepth = 0.f;
                    for (int i = 0; i < m_width * m_height; ++i) {
                        if (m_depthBuffer[i] < std::numeric_limits<float32>::max() && m_depthBuffer[i] > maxDepth) {
                            maxDepth = m_depthBuffer[i];
                        }
                    }
                    uint8* depth = new uint8[m_width * m_height];
                    for (int i = 0; i < m_width * m_height; ++i) {
                        if (m_depthBuffer[i] < std::numeric_limits<float32>::max()) {
                            float32 d = m_depthBuffer[i] / maxDepth;
                            depth[i] = (uint8)(d * 255);
                        }
                        else {
                            depth[i] = 0x00;
                        }
                    }

                    auto result = SDL_CreateRGBSurfaceFrom(depth, m_width, m_height, 8, m_width, 0x00, 0xFF, 0xFF, 0xFF);
                    delete depth;
                    return result;
                }
        }
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
                //draw_line(pointA, pointB, color::s_yellow);
                //draw_line(pointA, pointC, color::s_yellow);
                //draw_line(pointB, pointC, color::s_yellow);
            }
        }
    }
}

namespace constants
{
    const int width = 1920;
    const int height = 1080;
}

int main(int argc, char* argv[])
{
    SDL_Window* window = SDL_CreateWindow("Soft Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, constants::width, constants::height, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);

    int pixelSize = 30;
    video::device device(constants::width / pixelSize, constants::height / pixelSize);

    const float halfSize = 3.f;

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

    glm::vec3 pa1(constants::width / pixelSize / 2, 20, 3);
    glm::vec3 pa2(pa1.x - 30, pa1.y + 40, 3);
    glm::vec3 pa3(pa1.x + 30, pa1.y + 40, 3);

    glm::vec3 pb1(pa1.x + 20, pa1.y, 5);
    glm::vec3 pb2(pb1.x - 30, pb1.y + 45, 1);
    glm::vec3 pb3(pb1.x + 30, pb1.y + 40, 5);

    SDL_Surface* shawnSurface = IMG_Load("shawn2.jpg");
    SDL_Texture* shawnTexture = SDL_CreateTextureFromSurface(renderer, shawnSurface);
    SDL_FreeSurface(shawnSurface);

    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        isRunning = false;
                        break;

                    case SDL_SCANCODE_MINUS:
                        pixelSize--;
                        pixelSize = std::max(pixelSize, 1);
                        device.resize(constants::width / pixelSize, constants::height / pixelSize);
                        break;

                    case SDL_SCANCODE_EQUALS:
                        pixelSize++;
                        pixelSize = std::min(pixelSize, 128);
                        device.resize(constants::width / pixelSize, constants::height / pixelSize);
                        break;
                    default:
                        break;
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
        //device.draw_triangle(pa1, pa2, pa3, video::color::s_blue);
        //device.draw_triangle(pb1, pb2, pb3, video::color::s_green);

        cubeMesh.m_rotation.x += 0.0023f;
        cubeMesh.m_rotation.y += 0.001f;

        //SDL_Surface* surface = device.create_surface(video::device::buffer_type::cColor);
        //SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        //SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        for (int i = 0; i < device.get_size(); ++i) {
            uint32 c = device.get_colors()[i];
            if ((c & 0x00FFFFFF) > 0) {
                int x, y;
                device.xy_from_index(i, x, y);

                SDL_Rect r {
                    x * pixelSize, y * pixelSize,
                    pixelSize, pixelSize,
                };

                video::color col(c);

                SDL_RenderCopy(renderer, shawnTexture, nullptr, &r);
            }
        }

        //SDL_DestroyTexture(texture);
        //SDL_FreeSurface(surface);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
