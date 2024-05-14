#pragma once

#include <itugl/core/DeviceGL.h>
#include <itugl/application/Window.h>
#include <string>

class Application
{
public:
    // Construct the application specifying the dimensions of the window and its title
    Application(int width, int height, const char* title);

    // Destroy de application
    virtual ~Application();

    // Start the application
    int Run();

    // Get time in seconds from the start of the application
    [[nodiscard]] float GetCurrentTime() const { return m_currentTime; }

    // Get time in seconds of the current frame
    [[nodiscard]] float GetDeltaTime() const { return m_deltaTime; }

    // (C++) 1
    // Get the OpenGL device
    [[nodiscard]] const DeviceGL& GetDevice() const { return m_device; }
    DeviceGL& GetDevice() { return m_device; }

    // (C++) 1
    // Get the main window of the application
    Window& GetMainWindow() { return m_mainWindow; }
    const Window& GetMainWindow() const { return m_mainWindow; }

protected:
    // Test if the application is currently running
    bool IsRunning() const;

    // Request the application to stop running
    void Close() { Terminate(0); }

    // Load initial resources and initialize data before the main loop
    virtual void Initialize();

    // Update the application logic for the current frame
    virtual void Update();

    // Render the current frame
    virtual void Render();

    // Release all resources after the main loop
    virtual void Cleanup();

protected:
    // End the execution of the application and return the exit code (0 for OK)
    // Optionally provide an error message, if the exit code is not 0
    void Terminate(int exitCode, const char* errorMessage = nullptr);

private:
    // Set the new current time and compute the delta since the last time
    void UpdateTime(float newCurrentTime);

private:
    // OpenGL device
    DeviceGL m_device;

    // Main window
    Window m_mainWindow;

    // Time in seconds from the start of the application
    float m_currentTime;
    // Time in seconds of the current frame
    float m_deltaTime;

    // Exit code
    int m_exitCode;
    // Error message to display on exit
    std::string m_errorMessage;
};
