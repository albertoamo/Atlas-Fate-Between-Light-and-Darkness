#pragma once

#include "modules/module.h"

// Container class for scenes.
class Scene {

public:
    bool isLoaded;
    CHandle root;

    std::string name;
    std::string navmesh;
    std::vector<std::string> groups_subscenes;
};

class CModuleSceneManager : public IModule
{
public:

    int sceneCount;

    CModuleSceneManager(const std::string& name);
	bool start() override;
	bool stop() override;
	void update(float delta) override;

    bool loadScene(const std::string & name);
    Scene* createScene(const std::string& name);
    Scene* getSceneByName(const std::string& name);
    Scene* getActiveScene();

private:
    Scene * _activeScene;
    Scene * _persistentScene;
    std::map<std::string, Scene*> _scenes;

    void loadJsonScenes(const std::string filepath);
    void setActiveScene(Scene* scene);
    bool unLoadActiveScene();
};