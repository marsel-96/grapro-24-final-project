module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

export module TestModule;

export class TestModule {
private:
    std::vector<int> m_data;
public:
    std::vector<int>& GetData() {
        return m_data;
    };
};
