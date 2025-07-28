#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp> 

class Shader
{
public:
    // El ID del programa de shader
    unsigned int ID;

    // Constructor que lee y construye el shader
    Shader(const char* vertexPath, const char* fragmentPath);

    // Activa el shader
    void use();

    // Elimina el programa de shader
    void Delete();

    // Funciones para establecer uniformes
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;

private:
    // Función de utilidad para comprobar errores de compilación/enlace de shaders.
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
