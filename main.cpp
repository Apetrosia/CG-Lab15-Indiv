#include <SFML/Graphics.hpp>
#include <GL/glew.h>

#include <stb_image.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

#include "shader.h"
#include "camera.h"
#include "model.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(-8.0f, 12.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, -45.0f);
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool camDown = false;
float camTimer = 0.5f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float timer = 0.5f;
float circleTimer = 0.0f;

glm::vec3 balloon_pos = glm::vec3(0.0f, 2.0f, 0.0f);

std::vector<glm::vec3> presents;
std::vector<glm::vec3> houses = {
    glm::vec3(-5.0f,  -3.85f, 0.0f),
    glm::vec3(-8.0f, -3.85f, 7.0f),
    glm::vec3( 8.0f,  -3.85f, 12.0f),
    glm::vec3(-7.0f,  -3.85f, -6.0f),
    glm::vec3(-9.0f,  -3.85f, 14.0f),
    glm::vec3( 9.0f,  -3.85f, -7.0f),
};

void Init()
{
    // Инициализация шейдеров и буферов
    // InitTexture();
    // InitShader();
    // InitBuffers();

    glEnable(GL_DEPTH_TEST);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(sf::Window& window)
{
    if (camTimer >= 0.5f && sf::Keyboard::isKeyPressed(sf::Keyboard::O))
    {
        camTimer = 0.0f;
        camDown = !camDown;
        if (camDown)
        {
            camera.Yaw = 180.0f;
            camera.Pitch = -90.0f;
            camera.Position = balloon_pos + glm::vec3(0.0f, -1.0f, 0.0f);
        }
        else
        {
            camera.Yaw = 0.0f;
            camera.Pitch = -45.0f;
            camera.Position = balloon_pos + glm::vec3(-8.0f, 12.0f, 0.0f);
        }

        camera.updateCameraVectors();
    }

    glm::vec3 old_camera_pos = camera.Position;
    // Проверка закрытия окна при нажатии ESC
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        window.close();

    // Управление камерой с помощью клавиш WASD
    float v = deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        v *= 2.5;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        camera.ProcessKeyboard(FORWARD, v);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        camera.ProcessKeyboard(BACKWARD, v);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        camera.ProcessKeyboard(LEFT, v);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        camera.ProcessKeyboard(RIGHT, v);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        camera.ProcessKeyboard(UP, deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        camera.ProcessKeyboard(ROTATE_LEFT, deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        camera.ProcessKeyboard(ROTATE_RIGHT, deltaTime);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
        if (timer >= 0.5f)
        {
            timer = 0.0f;
            presents.push_back(balloon_pos - glm::vec3(0.0f, 0.2f, 0.0f));
        }

    balloon_pos += camera.Position - old_camera_pos;

    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
    float xpos = static_cast<float>(mousePosition.x);
    float ypos = static_cast<float>(mousePosition.y);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Инвертируем для работы с координатами камеры
    lastX = xpos;
    lastY = ypos;

    /*
    // Обрабатываем движение мыши через камеру
    //camera.ProcessMouseMovement(xoffset, 0.0f);

    float theta = xoffset * camera.MouseSensitivity * 0.9f / 8.0f;

    // Начальный угол (в радианах)
    float alpha = std::atan2(camera.Position.z, camera.Position.x);

    // Новый угол
    float beta = alpha + theta;

    // Вычисляем новые координаты
    camera.Position.x = 8.0f * std::cos(beta);
    camera.Position.z = 8.0f * std::sin(beta);

    camera.updateCameraVectors();

    glm::vec3 radius = balloon_pos - camera.Position;

    float prod = camera.Front.x * radius.x + camera.Front.z * radius.z;
    float mods = std::sqrt(camera.Front.x * camera.Front.x + camera.Front.z * camera.Front.z)
        * std::sqrt(radius.x * radius.x + radius.z * radius.z);

    // Угол в радианах
    camera.Yaw += std::acos(prod / mods) * 180.0f / 3.1415f;

    camera.updateCameraVectors();
    */
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int main()
{
    setlocale(LC_ALL, "ru");

    sf::Window window(sf::VideoMode(1000, 800), "OpenG(ame)L", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    window.setMouseCursorVisible(false);

    glewInit();

    Init();

    // Точечный источник света
    Shader phongShader = Shader("5.1.directional.vs", "5.1.phong.fs");
    Shader lightingShader = Shader("5.1.directional.vs", "5.1.phong.fs");

    Model balloon("resources/objs/balloon.obj");
    Model field("resources/objs/field.obj");
    Model tree("resources/objs/tree.obj");
    Model present("resources/objs/present.obj");
    Model house("resources/objs/house.obj");
    Model circle("resources/objs/circle.obj");

    glm::vec3 objPosition[] = {
        glm::vec3(-5.0f, -3.85f, 0.0f),
        glm::vec3(-5.0f, -3.85f, 0.0f),
        glm::vec3(-5.0f, -3.85f, 0.0f),
    };

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    sf::Clock clock;

    // Основной цикл
    while (window.isOpen())
    {
        // per-frame time logic
        // --------------------
        float currentFrame = clock.getElapsedTime().asSeconds();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (timer < 0.5f)
            timer += deltaTime;
        if (camTimer < 0.5f)
            camTimer += deltaTime;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects

        // Направленный источник света
        lightingShader.use();
        lightingShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("viewPos", camera.Position);

        // light properties
        lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // Отрисовка моделей
        lightingShader.setBool("useLightmap", false);
        lightingShader.setBool("useCircles", false);

        // Поле
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        lightingShader.setMat4("model", model);
        field.Draw(lightingShader);

        // Елка
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.85f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        lightingShader.setMat4("model", model);
        tree.Draw(lightingShader);

        // Аэростат
        model = glm::mat4(1.0f);
        model = glm::translate(model, balloon_pos);
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        lightingShader.setMat4("model", model);
        balloon.Draw(lightingShader);

        // Аэростат

        // Подарки
        for (int i = 0; i < presents.size(); i++)
        {
            if (presents[i].y <= -3.6f)
            {
                presents.erase(presents.begin() + i--);
                continue;
            }

            bool skip = false;
            for (int j = 0; j < houses.size(); j++)
            {
                glm::vec3 vec = glm::vec3(presents[i].x - houses[j].x, presents[i].y - houses[j].y, presents[i].z - houses[j].z);
                float dist = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);

                if (dist <= 2.0f)
                {
                    presents.erase(presents.begin() + i--);
                    houses.erase(houses.begin() + j);
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;

            model = glm::mat4(1.0f);
            model = glm::translate(model, presents[i]);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            lightingShader.setMat4("model", model);
            present.Draw(lightingShader);
            presents[i] -= glm::vec3(0.0f, 1.0f * deltaTime, 0.0f);
        }

        // Домики
        for (int i = 0; i < houses.size(); i++)
        {
            lightingShader.setBool("useLightmap", true);
            lightingShader.setBool("useCircles", false);
            lightingShader.use();
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(houses[i]));
            model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
            lightingShader.setMat4("model", model);
            house.Draw(lightingShader);

            // Кружки
            lightingShader.setBool("useLightmap", false);
            lightingShader.setBool("useCircles", true);
            model = glm::translate(model, glm::vec3(0.0f, 6.0f, 0.0f));
            lightingShader.setVec3("center", houses[i] + glm::vec3(0.0f, 6.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
            circleTimer += deltaTime;
            if (circleTimer >= 360)
                circleTimer = 0.0f;
            lightingShader.setFloat("time", circleTimer);
            lightingShader.setMat4("model", model);
            circle.Draw(lightingShader);
        }

        // ОбменBuffers
        window.display();

        // Обработка событий
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }

    return 0;
}
