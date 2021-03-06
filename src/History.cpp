#include<Prjoct2/JSONHolder.hpp>
#include<Prjoct2/QuadTree.hpp>
#include<Prjoct2/History.hpp>
#include<stdexcept>
#include<iostream>
#include<iomanip>

History * History::s_instance = nullptr;

History::History()
	: m_modificationStarted() {
}

History & History::get() {
	if (s_instance == nullptr) {
		s_instance = new History();
	}
	return *(s_instance);
}

void History::beginModification() {
	if (m_modificationStarted) {
		throw std::logic_error("[HISTORY] Modification is already \
started!");
	}
	m_beforeModification = JSONHolder::get()["components"];
	m_modificationStarted = true;
}

void History::abortModification() {
	if (!m_modificationStarted) {
		throw std::logic_error("[HISTORY] Modification is not started, \
cannot abort!");
	}
	m_modificationStarted = false;
}

void History::endModification() {
	if (!m_modificationStarted) {
		throw std::logic_error("[HISTORY] Modification is not started, \
cannot end!");
	}
	nlohmann::json afterModification = JSONHolder::get()["components"];
	HistoryEntryPtr entry(new HistoryEntry);
	entry->patch = nlohmann::json::diff(m_beforeModification,
		afterModification);
	entry->reversePatch = nlohmann::json::diff(afterModification,
		m_beforeModification);
	m_entryList.push_front(std::move(entry));
	m_modificationStarted = false;
}

void History::undo() {
	if (m_entryList.empty()) {
		return;
	}
	m_entryList.front()->undo();
	m_redoList.push_front(std::move(m_entryList.front()));
	m_entryList.pop_front();
}

void History::HistoryEntry::undo() {
	QuadTree::get().removeAll();
	nlohmann::json components = JSONHolder::get()["components"];
	JSONHolder::get()["components"] = components.patch(reversePatch);
	QuadTree::get().addAll();
}

void History::redo() {
	if (m_redoList.empty()) {
		return;
	}
	m_redoList.front()->redo();
	m_entryList.push_front(std::move(m_redoList.front()));
	m_redoList.pop_front();
}

void History::HistoryEntry::redo() {
	QuadTree::get().removeAll();
	nlohmann::json components = JSONHolder::get()["components"];
	JSONHolder::get()["components"] = components.patch(patch);
	QuadTree::get().addAll();
}

void History::clear() {
	m_entryList.clear();
	m_redoList.clear();
	m_modificationStarted = false;
}

