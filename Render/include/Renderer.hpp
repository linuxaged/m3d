namespace m3d {

class Scene;

class Renderer {
public:
    Renderer() {}
    virtual ~Renderer() {}

    /* Non-copyable */
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /* Init */
    virtual void Init(Scene*) = 0;

    /* Render Loop */
    virtual void OnWindowSizeChanged() = 0;
    virtual void Draw() = 0;

protected:
    /* Create Device */
    virtual void CreateDevice() = 0;
};
}