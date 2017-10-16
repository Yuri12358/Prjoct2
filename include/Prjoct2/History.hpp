#ifndef HISTORY_HPP
#define HISTORY_HPP

#include<nlohmann/json.hpp>
#include<memory>
#include<list>

class History {
	struct HistoryEntry {
		void undo();
		void redo();

		nlohmann::json patch;
		nlohmann::json reversePatch;
	};

	typedef std::unique_ptr<HistoryEntry> HistoryEntryPtr;

	History();

	static History * s_instance;
	std::list<HistoryEntryPtr> m_entryList;
	std::list<HistoryEntryPtr> m_redoList;
	nlohmann::json m_beforeModification;
	bool m_modificationStarted;

public:
	static History & get();
	void beginModification();
	void abortModification();
	void endModification();
	void undo();
	void redo();
};

#endif // HISTORY_HPP
