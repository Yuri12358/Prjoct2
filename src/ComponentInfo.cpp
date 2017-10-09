#include<Prjoct2/ComponentInfo.hpp>
#include<Prjoct2/JSONHolder.hpp>

sf::IntRect ComponentInfo::getRect(const nlohmann::json & component) {
	int x = component["position"]["x"];
	int y = component["position"]["y"];
	int w = component["width"];
	int h = component["height"];
	return sf::IntRect(x, y, w, h);
}

sf::IntRect ComponentInfo::getRect(const std::string & id) {
	nlohmann::json & component(JSONHolder::get()["components"][id]);
	return getRect(component);
}

nlohmann::json * ComponentInfo::getComponentPin(const std::string & id,
	int pinX, int pinY) {
	nlohmann::json::iterator it = JSONHolder::get()["components"].find(id);
	if (it == JSONHolder::get()["components"].end()) {
		return nullptr;
	}
	std::string type = (*it)["type"];
	if (type == "dot") {
		nlohmann::json & pinPattern = (*it)["pin pattern"];
		if (pinX == static_cast<int>(pinPattern["x"])
			&& pinY == static_cast<int>(pinPattern["y"])) {
			int nextPinID = (*it)["next pin ID"];
			(*it)["next pin ID"] = nextPinID + 1;
			std::string strID(std::to_string(nextPinID));
			nlohmann::json & pin = (*it)["pins"][strID];
			pin = pinPattern;
			pin["parentID"] = id;
			pin["id"] = strID;
			return &pin;
		}
	} else {
		for (nlohmann::json & pin : (*it)["pins"]) {
			if (pinX == static_cast<int>(pin["x"])
				&& pinY == static_cast<int>(pin["y"])) {
				return &pin;
			}
		}
	}
	return nullptr;
}

nlohmann::json * ComponentInfo::getComponentPinAt(const std::string & id,
	int x, int y) {
	nlohmann::json::iterator it = JSONHolder::get()["components"]
		.find(id);
	if (it == JSONHolder::get()["components"].end()) {
		return nullptr;
	}
	int pinX = x - static_cast<int>((*it)["position"]["x"]);
	int pinY = y - static_cast<int>((*it)["position"]["y"]);
	return getComponentPin(id, pinX, pinY);
}

Direction ComponentInfo::getPinOrientation(const nlohmann::json & pin) {
	std::string parentID = pin["parentID"];
	const nlohmann::json & parent = JSONHolder::get()["components"]
		[parentID];
	int width = parent["width"];
	int height = parent["height"];
	Direction result = None;
	int x = static_cast<int>(pin["x"]);
	int y = static_cast<int>(pin["y"]);
	if (x == 0) {
		result = Direction(result | Left);
	}
	if (x == width) {
		result = Direction(result | Right);
	}
	if (y == 0) {
		result = Direction(result | Up);
	}
	if (y == height) {
		result = Direction(result | Down);
	}
	if (result == None) {
		result = All;
	}
	return result;
}

