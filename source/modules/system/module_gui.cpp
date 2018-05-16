#include "mcv_platform.h"
#include "module_gui.h"
#include "render/render_objects.h"
#include "gui/gui_parser.h"
#include "gui/controllers/gui_menu_buttons_controller.h"
#include "gui/widgets/gui_bar.h"
#include "gui/gui_controller.h"

using namespace GUI;

CModuleGUI::CModuleGUI(const std::string& name)
	: IModule(name)
{}

bool CModuleGUI::start()
{
	const float width = 1080;
	const float height = 640;
	_orthoCamera.setOrthographicGUI(width, height);

	_technique = Resources.get("gui.tech")->as<CRenderTechnique>();
	_quadMesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
	_fontTexture = Resources.get("data/textures/gui/font.dds")->as<CTexture>();

	initializeWidgetStructure();
	
	return true;
}

void CModuleGUI::initializeWidgetStructure() {

	/*CParser parser;
	parser.parseFile("data/gui/test.json");
	parser.parseFile("data/gui/main_menu_background.json");
	parser.parseFile("data/gui/ingame.json");
	parser.parseFile("data/gui/main_menu_buttons.json");
	parser.parseFile("data/gui/main_menu_option_buttons.json");*/

	registerWigdetStruct(EGUIWidgets::MAIN_MENU_BACKGROUND, "data/gui/main_menu_background.json");
	registerWigdetStruct(EGUIWidgets::MAIN_MENU_BUTTONS, "data/gui/main_menu_buttons.json");
	/*parser.parseFile("data/gui/main_menu.json");
	parser.parseFile("data/gui/gameplay.json");
	parser.parseFile("data/gui/game_over.json");*/

	activateWidget("main_menu_background");
	activateWidget("main_menu_buttons");
	auto newGameCB = []() {
		CEngine::get().getGUI().outOfMainMenu();
	};
	auto continueCB = []() {
		CEngine::get().getGUI().outOfMainMenu();
	};
	auto optionsCB = []() {
		//activateWidget("main_menu_buttons");
		//activateWidget("main_menu_buttons");
	};
	auto exitCB = []() {
		exit(0);
	};
	//CEngine::get().getGUI().getWidget("stamina_bar", true)->getBarParams()->_processValue = 0.5f;
	/*CMenuButtonsController* mmc = new CMenuButtonsController();
	mmc->registerOption("new_game", newGameCB);
	mmc->registerOption("continue", continueCB);
	mmc->registerOption("options", optionsCB);
	mmc->registerOption("exit", exitCB);
	mmc->setCurrentOption(0);
	registerController(mmc);*/

}

void CModuleGUI::registerWigdetStruct(EGUIWidgets wdgt_type, std::string wdgt_path, GUI::CController wdgt_controller) {

	WidgetStructure wdgt_struct;
	CParser parser;
	wdgt_struct._widgetName = parser.parseFile(wdgt_path);
	wdgt_struct._type = wdgt_type;
	wdgt_struct._controller = wdgt_controller;
}

void CModuleGUI::outOfMainMenu() {
	CEngine::get().getModules().changeGameState("map_intro");
	activateWidget("blackground");
	_controllers.clear();
}

void CModuleGUI::enterMainMenu() {

}

bool CModuleGUI::stop()
{
	return true;
}

void CModuleGUI::update(float delta)
{
	if (EngineInput[VK_DOWN].getsPressed())
	{
		deactivateWidget("main_menu_buttons");
	}

	if (EngineInput[VK_UP].getsPressed())
	{
		deactivateWidget("main_menu_background");
	}

	for (auto& wdgt : _activeWidgets)
	{
		wdgt->updateAll(delta);
	}
	for (auto& controller : _controllers)
	{
		controller->update(delta);
	}

}

void CModuleGUI::renderGUI()
{
	for (auto& wdgt : _activeWidgets)
	{
		wdgt->renderAll();
	}
}

void CModuleGUI::registerWidget(CWidget* wdgt)
{
	_registeredWidgets.push_back(wdgt);
}

CWidget* CModuleGUI::getWidget(const std::string& name, bool recursive) const
{
	for (auto& rwdgt : _registeredWidgets)
	{
		if (rwdgt->getName() == name)
		{
			return rwdgt;
		}
	}

	if (recursive)
	{
		for (auto& rwdgt : _registeredWidgets)
		{
			CWidget* wdgt = rwdgt->getChild(name, true);
			if (wdgt)
			{
				return wdgt;
			}
		}
	}

	return nullptr;
}

void CModuleGUI::activateWidget(const std::string& name)
{
	CWidget* wdgt = getWidget(name);
	if (wdgt)
	{
		_activeWidgets.push_back(wdgt);
	}
}

void CModuleGUI::deactivateWidget(const std::string& name)
{
	CWidget* wdgt = getWidget(name);
	for (auto it = _activeWidgets.begin(); it != _activeWidgets.end();) {
		if (*it == wdgt) {
			_activeWidgets.erase(it);
			break;
		}
		it++;
	}
}

void CModuleGUI::registerController(GUI::CController* controller)
{
	auto it = std::find(_controllers.begin(), _controllers.end(), controller);
	if (it == _controllers.end())
	{
		_controllers.push_back(controller);
	}
}

void CModuleGUI::unregisterController(GUI::CController* controller)
{
	auto it = std::find(_controllers.begin(), _controllers.end(), controller);
	if (it != _controllers.end())
	{
		_controllers.erase(it);
	}
}

CCamera& CModuleGUI::getCamera()
{
	return _orthoCamera;
}

MVariants& CModuleGUI::getVariables()
{
	return _variables;
}

void CModuleGUI::renderTexture(const MAT44& world, const CTexture* texture, const VEC2& minUV, const VEC2& maxUV, const VEC4& color)
{
	assert(_technique && _quadMesh);

	cb_object.obj_world = world;
	cb_object.obj_color = VEC4(1, 1, 1, 1);
	cb_object.updateGPU();

	cb_gui.minUV = minUV;
	cb_gui.maxUV = maxUV;
	cb_gui.tint_color = color;
	cb_gui.updateGPU();

	_technique->activate();
	if (texture)
		texture->activate(TS_ALBEDO);

	_quadMesh->activateAndRender();
}

void CModuleGUI::renderText(const MAT44& world, const std::string& text, const VEC4& color)
{
	assert(_fontTexture);

	int cellsPerRow = 8;
	float cellSize = 1.f / 8.f;
	char firstCharacter = ' ';
	for (size_t i = 0; i < text.size(); ++i)
	{
		char c = text[i];

		int cell = c - firstCharacter;
		int row = cell / cellsPerRow;
		int col = cell % cellsPerRow;

		VEC2 minUV = VEC2(col * cellSize, row * cellSize);
		VEC2 maxUV = minUV + VEC2(1, 1) * cellSize;
		VEC2 gap = i * VEC2(1, 0);
		MAT44 w = MAT44::CreateTranslation(gap.x, gap.y, 0.f) * world;

		renderTexture(w, _fontTexture, minUV, maxUV, color);
	}
}
