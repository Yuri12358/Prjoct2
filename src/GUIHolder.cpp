#include<Prjoct2/TextureHolder.hpp>
#include<Prjoct2/JSONHolder.hpp>
#include<Prjoct2/GUIHolder.hpp>
#include<Prjoct2/App.hpp>
#include<functional>
#include<iostream>

GUIHolder * GUIHolder::s_instance = nullptr;

GUIHolder::GUIHolder()
	: m_gui(App::get().window()) {
	std::string defaultThemePath = JSONHolder::get()["settings"]
		["default theme"];
	m_theme.load(defaultThemePath);
	tgui::Theme::setDefault(&m_theme);
	m_createMenuBar();
	m_createComponentSelector();
}

GUIHolder & GUIHolder::get() {
	if (s_instance == nullptr) {
		s_instance = new GUIHolder();
	}
	return *s_instance;
}

tgui::Gui & GUIHolder::gui() {
	return m_gui;
}

void GUIHolder::m_createMenuBar() {
	auto bar = tgui::MenuBar::create();
	m_gui.add(bar, "menuBar");
	bar->setSize("100%", JSONHolder::get()["settings"]["menuBar"]
		["height"].get<int>());
	bar->addMenu("File");
	bar->addMenuItem("New");
	bar->addMenuItem("Open");
	bar->addMenuItem("Save");
	std::function<void(const std::vector<sf::String> &)> signal
		= std::bind(&GUIHolder::m_menuBarSignal, this, std::placeholders::_1);
	bar->connect("menuItemClicked", signal);
}

void GUIHolder::m_createComponentSelector() {
	tgui::ScrollablePanel::Ptr panel = tgui::ScrollablePanel::create();
	m_gui.add(panel, "componentPanel");
	panel->setSize(0, "100% - menuBar.size");
	panel->setPosition("0", "menuBar.bottom");
	tgui::Button::Ptr toggleButton = tgui::Button::create(">");
	m_gui.add(toggleButton, "componentPanelToggleButton");
	toggleButton->setSize(25, 25);
	toggleButton->setPosition(tgui::bindRight(panel), 100);
	toggleButton->connect("pressed", [&](std::string text) {
		if (text == ">") {
			m_gui.get<tgui::Button>("componentPanelToggleButton")
				->setText("<");
			int buttonSize = JSONHolder::get()["settings"]
				["componentPanel"]["button"]["size"];
			int spacing = JSONHolder::get()["settings"]
				["componentPanel"]["spacing"];
			m_gui.get<tgui::ScrollablePanel>("componentPanel")
				->setSize(2 * spacing + buttonSize + 20,
				"100% - menuBar.size");
		} else {
			m_gui.get<tgui::Button>("componentPanelToggleButton")
				->setText(">");
			m_gui.get<tgui::ScrollablePanel>("componentPanel")
				->setSize(0, "100% - menuBar.size");
		}
	});
	m_createComponentButtons();
	auto menu = m_gui.get<tgui::MenuBar>("menuBar");
	m_gui.remove(menu);
	m_gui.add(menu, "menuBar");
}

void GUIHolder::m_createComponentButtons() {
	for (nlohmann::json & entry : JSONHolder::get()["settings"]
		["components"]) {
		std::string name = entry["name"];
		if (entry.count("texture") == 1) {
			m_addComponentButton(name, entry["texture"]
				.get<std::string>());
		} else {
			m_addComponentButton(name);
		}
	}
}

void GUIHolder::m_addComponentButton(const std::string & name,
	const std::string & requestedTextureName) {
	if (JSONHolder::get()["components/" + name].is_null()) {
		JSONHolder::get().fromFile("components/" + name);
	}
	std::string textureName;
	if (requestedTextureName != "") {
		textureName = requestedTextureName;
	} else {
		textureName = JSONHolder::get()["components/" + name]["texture"];
	}
	int buttonSize = JSONHolder::get()["settings"]["componentPanel"]
		["button"]["size"];
	int spacing = JSONHolder::get()["settings"]["componentPanel"]
		["spacing"];
	auto button = tgui::Button::create();
	button->setSize(buttonSize, buttonSize);
	m_createComponentButtonTextures(textureName);
	button->getRenderer()->setTexture(TextureHolder::get()["shelf/"
		+ textureName]);
	button->getRenderer()->setTextureHover(TextureHolder::get()[
		"shelf/opened/" + textureName]);
	button->getRenderer()->setBorders(tgui::Borders());
	auto panel = m_gui.get<tgui::ScrollablePanel>("componentPanel");
	int index = panel->getWidgets().size();
	button->setPosition(spacing, buttonSize * index + spacing 
		* (index + 1));
	std::function<void(const std::string &)> signal = std::bind(&GUIHolder
		::m_componentButtonSignal, this, name);
	button->connect("pressed", signal);
	panel->add(button);
	panel->setContentSize(sf::Vector2f(spacing * 2 + buttonSize,
		buttonSize * (index + 1) + spacing * (index + 2)));
}

void GUIHolder::m_createComponentButtonTextures(const std::string & name) {
	int buttonSize = JSONHolder::get()["settings"]["componentPanel"]
		["button"]["size"];
	sf::RenderTexture renderTexture;
	renderTexture.create(buttonSize, buttonSize);
	sf::RectangleShape shelfRect(sf::Vector2f(buttonSize, buttonSize));
	shelfRect.setTexture(&TextureHolder::get()["shelf"], true);
	renderTexture.draw(shelfRect);
	int picSize = JSONHolder::get()["settings"]["componentPanel"]["button"]
		["picture size"];
	sf::RectangleShape picRect(sf::Vector2f(picSize, picSize));
	picRect.setFillColor(sf::Color(255, 255, 255, 128));
	picRect.setPosition((buttonSize - picSize) / 2, buttonSize - picSize
		- 10);
	renderTexture.draw(picRect);
	picRect.setFillColor(sf::Color::White);
	picRect.setTexture(&TextureHolder::get()[name], true);
	renderTexture.draw(picRect);
	renderTexture.display();
	TextureHolder::get().set("shelf/" + name, renderTexture.getTexture());
	shelfRect.setTexture(&TextureHolder::get()["shelf/" + name], true);
	shelfRect.move(0, 10);
	renderTexture.clear(sf::Color(128, 128, 128));
	renderTexture.draw(shelfRect);
	renderTexture.display();
	TextureHolder::get().set("shelf/opened/" + name, renderTexture
		.getTexture());
}

void GUIHolder::m_componentButtonSignal(const std::string & name) {
	JSONHolder::get()["current"] = JSONHolder::get()["components/" + name];
	JSONHolder::get()["next component type"] = name;
	App::get().cancelMovingComponent();
}

void GUIHolder::createContextMenu(int x, int y) {
	auto list = tgui::ListBox::create();
	list->setPosition(x, y);
	list->addItem("Move");
	list->addItem("Rotate");
	list->addItem("Delete");
	auto borders = list->getRenderer()->getBorders();
	auto padding = list->getRenderer()->getPadding();
	list->setSize(100, list->getItemHeight() * list->getItemCount()
		+ borders.getBottom() + borders.getTop()
		+ padding.getBottom() + padding.getTop());
	std::function<void(const std::string &)> signal = std::bind(&GUIHolder
		::m_contextMenuSignal, this, std::placeholders::_1);
	list->connect("itemSelected", signal);
	m_gui.add(list, "contextMenu");
}

void GUIHolder::m_contextMenuSignal(const std::string & item) {
	if (item == "") {
		return;
	}
	removeContextMenu();
	if (item == "Move") {
		App::get().moveSelectedComponent();
	} else if (item == "Rotate") {
		App::get().rotateSelectedComponent();
	} else if (item == "Delete") {
		App::get().deleteSelectedComponent();
	}
}

void GUIHolder::removeContextMenu() {
	m_gui.remove(m_gui.get("contextMenu"));
}

void GUIHolder::m_menuBarSignal(const std::vector<sf::String> & data) {
	if (data[0] == "File") {
		if (data[1] == "New") {
			App::get().createNewCircuit();
		} else if (data[1] == "Open") {
			App::get().openCircuit();
		} else if (data[1] == "Save") {
			App::get().saveCircuit();
		}
	}
}

void GUIHolder::m_createDialogWindow() {
	auto dialogWindow = tgui::MessageBox::create();
	m_gui.add(dialogWindow, "dialogWindow");
	dialogWindow->setSize(400, 100);
	dialogWindow->setTitleButtons(tgui::MessageBox::Close);
	auto container = tgui::HorizontalLayout::create();
	dialogWindow->add(container, "container");
	int spacing = 8;
	container->setSize(tgui::bindWidth(dialogWindow) - 2 * spacing, 25);
	container->setPosition(spacing, tgui::bindHeight(dialogWindow)
		- tgui::bindHeight(container) - spacing);
	auto editBox = tgui::EditBox::create();
	container->add(editBox, "editBox");
	container->setRatio(editBox, 2);
	editBox->setInputValidator("[a-zA-Z0-9_-]*");
	auto button = tgui::Button::create("OK");
	container->add(button, "button");
	dialogWindow->setPosition(100, 200);
	dialogWindow->connect("closed", [this] {
		closeDialogWindow();
		App::get().unlockUI();
	});
}

void GUIHolder::createOpenFileDialogWindow() {
	m_createDialogWindow();
	auto dialogWindow = m_gui.get<tgui::MessageBox>("dialogWindow");
	dialogWindow->setTitle("Open file");
	dialogWindow->setText("Enter circuit name:");
	auto container = dialogWindow->get<tgui::HorizontalLayout>("container");
	auto button = container->get<tgui::Button>("button");
	auto editBox = container->get<tgui::EditBox>("editBox");
	button->connect("pressed", [editBox] {
		App::get().openCircuit(editBox->getText());
	});
}

void GUIHolder::createSaveFileDialogWindow() {
	m_createDialogWindow();
	auto dialogWindow = m_gui.get<tgui::MessageBox>("dialogWindow");
	dialogWindow->setTitle("Save file");
	dialogWindow->setText("Enter circuit name:");
	auto container = dialogWindow->get<tgui::HorizontalLayout>("container");
	auto button = container->get<tgui::Button>("button");
	auto editBox = container->get<tgui::EditBox>("editBox");
	button->connect("pressed", [editBox] {
		App::get().saveCircuit(editBox->getText());
	});
}

void GUIHolder::closeDialogWindow() {
	m_gui.remove(m_gui.get("dialogWindow"));
}

