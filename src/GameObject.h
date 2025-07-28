#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glm/glm.hpp>
#include <string>

// Un enum para definir los tipos de formas b�sicas que podemos crear.
enum class ShapeType {
    Cube,
    Sphere // Podr�amos a�adir m�s en el futuro
};

// Contiene los datos de transformaci�n de un objeto.
struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

// Representa un objeto en nuestra escena.
struct GameObject {
    unsigned int id;
    std::string name;
    Transform transform;
    ShapeType shape;

    // Constructor
    GameObject(unsigned int p_id, std::string p_name, ShapeType p_shape)
        : id(p_id), name(p_name), shape(p_shape) {
    }
};

#endif // GAMEOBJECT_H
