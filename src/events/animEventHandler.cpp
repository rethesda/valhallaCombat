#include "events.h"
#include "attackHandler.h"
#include "blockHandler.h"
#include "executionHandler.h"
//all credits to Bingle&Ersh
constexpr uint32_t hash(const char* data, size_t const size) noexcept
{
	uint32_t hash = 5381;

	for (const char* c = data; c < data + size; ++c) {
		hash = ((hash << 5) + hash) + (unsigned char)*c;
	}

	return hash;
}

constexpr uint32_t operator"" _h(const char* str, size_t size) noexcept
{
	return hash(str, size);
}

RE::BSEventNotifyControl animEventHandler::HookedProcessEvent(RE::BSAnimationGraphEvent& a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* src) {
    FnProcessEvent fn = fnHash.at(*(uint64_t*)this);
	std::string_view eventTag = a_event.tag.data();
	switch (hash(eventTag.data(), eventTag.size())) {
	case "preHitFrame"_h:
		//DEBUG("==========prehitFrame==========");
		attackHandler::GetSingleton()->registerAtk(a_event.holder->As<RE::Actor>());
		break;
	case "attackStop"_h:
		//DEBUG("==========attackstop==========");
		attackHandler::GetSingleton()->checkout(a_event.holder->As<RE::Actor>());
		break;
	case "blockStartOut"_h:
		//DEBUG("===========blockStartOut===========");
		if (settings::bPerfectBlockToggle) {
			blockHandler::GetSingleton()->registerPerfectBlock(a_event.holder->As<RE::Actor>());
		}
		break;
	case "tailcombatidle"_h:
		/*Unghost the executor on finish*/
		if (settings::bStunToggle) {
			executionHandler::GetSingleton()->activeExecutor.erase(a_event.holder->As<RE::Actor>());
			setIsGhost(a_event.holder->As<RE::Actor>(), false);
		}
		break;
	case "SoundPlay"_h:
		DEBUG("soundplay: {} from actor: {}", a_event.payload, a_event.holder->GetName());
	case "TKDR_DodgeStart"_h:
		//DEBUG("==========TK DODGE============");
		staminaHandler::checkStamina(a_event.holder->As<RE::Actor>());
		break;
	}
    return fn ? (this->*fn)(a_event, src) : RE::BSEventNotifyControl::kContinue;
}

std::unordered_map<uint64_t, animEventHandler::FnProcessEvent> animEventHandler::fnHash;