#include "Mesh.hpp"

class Mesh {
public:
    void Draw(GLuint shaderProgram) {
        GLuint diffuseNum = 1;
        GLuint specularNum = 1;

        for(GLuint i = 0; i < this->textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            if(this->textures[i]->type == DIFFUSE) {
                glUniform1i(glGetUniformLocation(shaderProgram, ("texture_Diffuse" + std::to_string(diffuseNum++)).c_str()), i);
                glBindTexture(GL_TEXTURE_2D, this->textures[i]->id);
            }
            else if(this->textures[i]->type == SPECULAR) {
                glUniform1i(glGetUniformLocation(shaderProgram, ("texture_Specular" + std::to_string(specularNum++)).c_str()), i);
                glBindTexture(GL_TEXTURE_2D, this->textures[i]->id);
            }
        }

        glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 40.0f);

        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, this->indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        for (GLuint i = 0; i < this->textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void UploadMeshData() {
        glGenVertexArrays(1, &this->VAO);
        glBindVertexArray(this->VAO);
        glGenBuffers(1, &this->EBO);
        glGenBuffers(3, this->VBO);

        GLuint* indices = &this->indices[0];
        GLfloat* vertices = &this->vertices[0];
        GLfloat* normals = &this->normals[0];
        GLfloat* texCoords = &this->texCoords[0];

        /* Element array buffer */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);

        /* Vertex coordinates */
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * vertexCount * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(glGetAttribLocation(program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(glGetAttribLocation(program, "in_Position"));

        /* Normal vectors */
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, 3 * vertexCount * sizeof(GLfloat), normals, GL_STATIC_DRAW);
        glVertexAttribPointer(glGetAttribLocation(program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(glGetAttribLocation(program, "in_Normal"));

        /* Texture coordinates */
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, 2 * vertexCount * sizeof(GLfloat), texCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(glGetAttribLocation(program, "in_TexCoord"), 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(glGetAttribLocation(program, "in_TexCoord"));

        /* Unbind VAO */
        glBindVertexArray(0);
    }
};
